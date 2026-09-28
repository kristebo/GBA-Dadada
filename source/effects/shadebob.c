#include <gba.h>
#include <math.h>
#include "shadebob.h"

#define SHADEBOB_TWO_PI 6.283185307179586f

enum {
    /* A heat-blob is dragged along a Lissajous-like path, leaving a
       fading trail. The path/margins are chosen so the 16x16 blob
       (plus its +-15px wobble) always stays inside the 240x160
       screen without needing per-pixel clamping. */
    SHADEBOB_BOB_SIZE = 16,
    SHADEBOB_PATH_LEN = 512,
    SHADEBOB_WOBBLE_LEN = 1024,
    SHADEBOB_RANGE_X = 160,
    SHADEBOB_RANGE_Y = 80,
    SHADEBOB_MARGIN_X = 20,
    SHADEBOB_MARGIN_Y = 20,
    SHADEBOB_WOBBLE_AMPLITUDE = 15,
    SHADEBOB_TRAIL_DELAY = 480,
    SHADEBOB_HEAT_SCALE = 8
};

/* Instead of a paletted SDL surface we use GBA Mode 4 (240x160, 8bpp,
   single buffer, BG palette as color ramp). The canonical pixel
   intensities live in a byte buffer in EWRAM (VRAM only accepts
   16-bit writes), and only the two 16x16 regions that change each
   frame (trail removed / head added) are re-packed into halfwords
   and pushed to VRAM. */
static EWRAM_BSS u8 shadebob_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static int shadebob_xpath[SHADEBOB_PATH_LEN];
static int shadebob_ypath[SHADEBOB_PATH_LEN];
static int shadebob_wobble[SHADEBOB_WOBBLE_LEN];

static const u8 shadebob_heat[16][16] = {
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 2, 2, 2, 3, 3, 2, 2, 2, 1, 1, 0, 0 },
    { 0, 0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 1, 0, 0 },
    { 0, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0 },
    { 0, 1, 2, 2, 3, 3, 3, 4, 4, 3, 3, 3, 2, 2, 1, 0 },
    { 1, 1, 2, 3, 3, 3, 4, 4, 4, 4, 3, 3, 3, 2, 1, 1 },
    { 1, 1, 2, 3, 3, 3, 4, 4, 4, 4, 3, 3, 3, 2, 1, 1 },
    { 0, 1, 2, 2, 3, 3, 3, 4, 4, 3, 3, 3, 2, 2, 1, 0 },
    { 0, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0 },
    { 0, 0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 1, 0, 0 },
    { 0, 0, 1, 1, 2, 2, 2, 3, 3, 2, 2, 2, 1, 1, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
};

/* Black -> blue -> red -> white color ramp, the classic shadebob
   palette. Values are 0-255 intensities packed to GBA RGB555 with
   RGB8(). */
static void init_shadebob_palette(void) {
    for (int i = 0; i < 64; ++i) {
        BG_PALETTE[i]       = RGB8(0, 0, i << 1);
        BG_PALETTE[i + 64]  = RGB8(i << 1, 0, 128 - (i << 1));
        BG_PALETTE[i + 128] = RGB8(128 + (i << 1), 0, 128 - (i << 1));
        BG_PALETTE[i + 192] = RGB8(255, i << 2, i << 2);
    }
}

/* Precompute the bob's movement path once at scene start (one-time
   floating point cost, not per-frame). */
static void init_shadebob_paths(void) {
    for (int i = 0; i < SHADEBOB_PATH_LEN; ++i) {
        float rad = (float)i * (SHADEBOB_TWO_PI / SHADEBOB_PATH_LEN);
        shadebob_xpath[i] = (int)(sinf(rad * 2.0f) * (SHADEBOB_RANGE_X / 2)
                                   + (SHADEBOB_RANGE_X / 2) + SHADEBOB_MARGIN_X);
        shadebob_ypath[i] = (int)(sinf(rad) * (SHADEBOB_RANGE_Y / 2)
                                   + (SHADEBOB_RANGE_Y / 2) + SHADEBOB_MARGIN_Y);
    }

    for (int i = 0; i < SHADEBOB_WOBBLE_LEN; ++i) {
        float rad = (float)i * (SHADEBOB_TWO_PI / SHADEBOB_WOBBLE_LEN);
        shadebob_wobble[i] = (int)(sinf(rad) * SHADEBOB_WOBBLE_AMPLITUDE);
    }
}

static int get_shadebob_x(int index) {
    return shadebob_xpath[index & (SHADEBOB_PATH_LEN - 1)]
         + shadebob_wobble[index & (SHADEBOB_WOBBLE_LEN - 1)];
}

static int get_shadebob_y(int index) {
    return shadebob_ypath[index & (SHADEBOB_PATH_LEN - 1)]
         + shadebob_wobble[index & (SHADEBOB_WOBBLE_LEN - 1)];
}

/* Add (sign=+1) or remove (sign=-1) the heat bob at (x0, y0). Updates
   the canonical byte buffer, then re-packs only the touched halfwords
   into VRAM so every VRAM write is a full 16-bit unit. */
static void shadebob_plot(int x0, int y0, int sign) {
    u16 *vram = (u16 *)VRAM;
    int start_col = x0 >> 1;
    int end_col = (x0 + SHADEBOB_BOB_SIZE - 1) >> 1;

    for (int row = 0; row < SHADEBOB_BOB_SIZE; ++row) {
        int row_base = (y0 + row) * SCREEN_WIDTH;

        for (int col = 0; col < SHADEBOB_BOB_SIZE; ++col) {
            int idx = row_base + x0 + col;
            int value = shadebob_buffer[idx]
                      + sign * (shadebob_heat[row][col] * SHADEBOB_HEAT_SCALE);
            if (value < 0) value = 0;
            if (value > 255) value = 255;
            shadebob_buffer[idx] = (u8)value;
        }

        u16 *row_vram = vram + row_base / 2;
        for (int col = start_col; col <= end_col; ++col) {
            int px_lo = row_base + col * 2;
            row_vram[col] = shadebob_buffer[px_lo]
                          | (shadebob_buffer[px_lo + 1] << 8);
        }
    }
}

void shadebob_run(void) {
    OBJATTR oam_buffer[128];

    for (int i = 0; i < 128; ++i) {
        oam_buffer[i].attr0 = ATTR0_DISABLED;
    }
    dmaCopy(oam_buffer, OAM, sizeof(oam_buffer));

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        shadebob_buffer[i] = 0;
    }

    init_shadebob_palette();
    init_shadebob_paths();

    REG_DISPCNT = MODE_4 | BG2_ENABLE;

    u32 trail = 0;
    while (1) {
        VBlankIntrWait();

        if (trail >= SHADEBOB_TRAIL_DELAY) {
            int tail_index = trail - SHADEBOB_TRAIL_DELAY;
            shadebob_plot(get_shadebob_x(tail_index), get_shadebob_y(tail_index), -1);
        }

        shadebob_plot(get_shadebob_x(trail), get_shadebob_y(trail), 1);

        ++trail;
    }
}
