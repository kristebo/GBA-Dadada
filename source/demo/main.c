#include <gba.h>
#include "tg_logo.h"
#include "glitch.h"
#include "shadebob.h"

static OBJATTR oam_buffer[128];

enum {
    COLOR_BG = RGB5(1, 2, 9),
    COLOR_PANEL = RGB5(3, 5, 16),
    COLOR_GRID = RGB5(5, 9, 22),
    COLOR_CYAN = RGB5(4, 25, 31),
    COLOR_GOLD = RGB5(31, 22, 5),
    COLOR_MAGENTA = RGB5(31, 5, 22),
    COLOR_WHITE = RGB5(25, 28, 31),
    COLOR_MUTED = RGB5(11, 15, 22),
    LOGO_FRAME_X = 88,
    LOGO_FRAME_Y = 46,
    LOGO_X = 88,
    OBJ_BITMAP_BASE_TILE = 512,
    LOGO_TILES = 64,
    GLITCH_FIRST_TILE = OBJ_BITMAP_BASE_TILE + LOGO_TILES,
    LOGO_OAM_INDEX = 4,
    BOOT_SCREEN_FRAMES = 90,
    LOGO_SCENE_FRAMES = 400
};

static const u8 font[43][5] = {
    { 7, 5, 7, 5, 5 }, { 6, 5, 6, 5, 6 }, { 7, 4, 4, 4, 7 },
    { 6, 5, 5, 5, 6 }, { 7, 4, 6, 4, 7 }, { 7, 4, 6, 4, 4 },
    { 7, 4, 5, 5, 7 }, { 5, 5, 7, 5, 5 }, { 7, 2, 2, 2, 7 },
    { 3, 1, 1, 5, 7 }, { 5, 5, 6, 5, 5 }, { 4, 4, 4, 4, 7 },
    { 5, 7, 7, 5, 5 }, { 5, 7, 7, 7, 5 }, { 7, 5, 5, 5, 7 },
    { 7, 5, 7, 4, 4 }, { 7, 5, 5, 7, 1 }, { 7, 5, 6, 5, 5 },
    { 7, 4, 7, 1, 7 }, { 7, 2, 2, 2, 2 }, { 5, 5, 5, 5, 7 },
    { 5, 5, 5, 5, 2 }, { 5, 5, 7, 7, 5 }, { 5, 5, 2, 5, 5 },
    { 5, 5, 2, 2, 2 }, { 7, 1, 2, 4, 7 },
    { 7, 5, 5, 5, 7 }, { 2, 6, 2, 2, 7 }, { 7, 1, 7, 4, 7 },
    { 7, 1, 7, 1, 7 }, { 5, 5, 7, 1, 1 }, { 7, 4, 7, 1, 7 },
    { 7, 4, 7, 5, 7 }, { 7, 1, 2, 2, 2 }, { 7, 5, 7, 5, 7 },
    { 7, 5, 7, 1, 7 }, { 0, 0, 0, 0, 0 },
    { 0, 6, 5, 5, 5 }, { 5, 5, 7, 1, 6 }, { 2, 7, 2, 2, 3 },
    { 2, 0, 6, 2, 7 }, { 6, 2, 2, 2, 7 }, { 0, 7, 6, 4, 7 }
};

enum {
    GLYPH_LOWER_N = 37,
    GLYPH_LOWER_Y,
    GLYPH_LOWER_T,
    GLYPH_LOWER_I,
    GLYPH_LOWER_L,
    GLYPH_LOWER_E
};

static void fill_rect(int x, int y, int width, int height, u16 color) {
    for (int row = y; row < y + height; ++row) {
        for (int column = x; column < x + width; ++column) {
            MODE3_FB[row][column] = color;
        }
    }
}

static int glyph_index(char character) {
    if (character >= 'A' && character <= 'Z') {
        return character - 'A';
    }
    if (character >= '0' && character <= '9') {
        return character - '0' + 26;
    }
    switch (character) {
        case 'n': return GLYPH_LOWER_N;
        case 'y': return GLYPH_LOWER_Y;
        case 't': return GLYPH_LOWER_T;
        case 'i': return GLYPH_LOWER_I;
        case 'l': return GLYPH_LOWER_L;
        case 'e': return GLYPH_LOWER_E;
        default: return 36;
    }
}

static void draw_text(int x, int y, const char *text, int scale, u16 color) {
    while (*text != '\0') {
        int glyph = glyph_index(*text++);

        for (int row = 0; row < 5; ++row) {
            for (int column = 0; column < 3; ++column) {
                if ((font[glyph][row] & (1 << (2 - column))) != 0) {
                    fill_rect(x + column * scale, y + row * scale,
                              scale, scale, color);
                }
            }
        }
        x += 4 * scale;
    }
}

static void draw_centered(int y, const char *text, int scale, u16 color) {
    int length = 0;

    while (text[length] != '\0') {
        ++length;
    }
    draw_text((SCREEN_WIDTH - length * 4 * scale) / 2, y, text, scale, color);
}

static void draw_title_background(void) {
    REG_BG2CNT = BG_WRAP;
    REG_BG2PA = 256;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 256;
    REG_BG2X = 0;
    REG_BG2Y = 0;
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BG);

    for (int y = 22; y < 140; y += 8) {
        fill_rect(12, y, SCREEN_WIDTH - 24, 1, COLOR_GRID);
    }
    fill_rect(12, 20, SCREEN_WIDTH - 24, 1, COLOR_CYAN);
    fill_rect(12, 139, SCREEN_WIDTH - 24, 1, COLOR_CYAN);
    fill_rect(12, 20, 1, 120, COLOR_GRID);
    fill_rect(SCREEN_WIDTH - 13, 20, 1, 120, COLOR_GRID);

    draw_text(6, 7, "En Byte til", 1, COLOR_CYAN);
    draw_text(207, 7, "GBA", 1, COLOR_MUTED);

    fill_rect(72, 31, 96, 34, COLOR_PANEL);
    draw_centered(35, "TG", 6, COLOR_GOLD);
    draw_centered(72, "THE GATHERING", 2, COLOR_WHITE);
    draw_centered(91, "2027", 4, COLOR_CYAN);

    draw_centered(119, "A MACHINE FROM 2001", 1, COLOR_MUTED);
    draw_centered(128, "A DEMO FROM 2027", 1, COLOR_MUTED);
    draw_centered(146, "BOOT SEQUENCE READY", 1, COLOR_GOLD);
}

static void draw_logo_frame(void) {
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BG);
    draw_text(6, 7, "En Byte til", 1, COLOR_CYAN);
    draw_text(207, 7, "GBA", 1, COLOR_MUTED);

    fill_rect(68, 27, 104, 1, COLOR_CYAN);
    fill_rect(68, 28, 1, 101, COLOR_GRID);
    fill_rect(171, 28, 1, 101, COLOR_GRID);
    fill_rect(68, 128, 104, 1, COLOR_CYAN);
    fill_rect(76, 35, 88, 1, COLOR_GRID);
    fill_rect(76, 36, 1, 84, COLOR_GRID);
    fill_rect(155, 36, 1, 84, COLOR_GRID);
    fill_rect(76, 119, 80, 1, COLOR_GRID);

    draw_centered(18, "SIGNAL LOCK", 1, COLOR_GOLD);
    draw_centered(137, "TG2027", 1, COLOR_WHITE);
    draw_centered(148, "A DEMO FROM 2027", 1, COLOR_MUTED);

    oam_buffer[LOGO_OAM_INDEX].attr0 = OBJ_Y(LOGO_FRAME_Y) | ATTR0_COLOR_16 | ATTR0_SQUARE;
    oam_buffer[LOGO_OAM_INDEX].attr1 = OBJ_X(LOGO_FRAME_X) | ATTR1_SIZE_64;
}

static void init_logo_sprite(void) {
    dmaCopy(logo_gfxTiles, BITMAP_OBJ_BASE_ADR, logo_gfxTilesLen);
    dmaCopy(logo_gfxPal, SPRITE_PALETTE, logo_gfxPalLen);

    SPRITE_PALETTE[16 + 15] = COLOR_CYAN;
    SPRITE_PALETTE[32 + 15] = COLOR_MAGENTA;
    SPRITE_PALETTE[48 + 15] = COLOR_GOLD;

    u32 *solid_tiles = (u32 *)BITMAP_OBJ_BASE_ADR + LOGO_TILES * 8;
    for (int i = 0; i < 64; ++i) {
        solid_tiles[i] = 0xFFFFFFFF;
    }

    for (int i = 0; i < 128; ++i) {
        oam_buffer[i].attr0 = ATTR0_DISABLED;
    }

    oam_buffer[LOGO_OAM_INDEX].attr0 = ATTR0_DISABLED;
    oam_buffer[LOGO_OAM_INDEX].attr1 = OBJ_X(LOGO_X) | ATTR1_SIZE_64;
    oam_buffer[LOGO_OAM_INDEX].attr2 = OBJ_CHAR(OBJ_BITMAP_BASE_TILE) | OBJ_PRIORITY(0);
    oam_buffer[LOGO_OAM_INDEX].dummy = 0;
}

int main(void) {
    REG_DISPCNT = LCDC_OFF;
    irqInit();
    irqEnable(IRQ_VBLANK);
    draw_title_background();
    init_logo_sprite();
    dmaCopy(oam_buffer, OAM, sizeof(oam_buffer));
    REG_DISPCNT = MODE_3 | BG2_ENABLE | OBJ_ON | OBJ_1D_MAP;

    for (u16 frame = 0; frame < BOOT_SCREEN_FRAMES; ++frame) {
        VBlankIntrWait();
    }

    draw_logo_frame();
    for (u16 frame = 0; frame < LOGO_SCENE_FRAMES; ++frame) {
        VBlankIntrWait();
        // glitch_update(oam_buffer, frame, LOGO_OAM_INDEX,
        //               LOGO_FRAME_X, LOGO_FRAME_Y, GLITCH_FIRST_TILE);
        dmaCopy(oam_buffer, OAM, sizeof(oam_buffer));
    }

    shadebob_run();
    return 0; /* never reached */
}
