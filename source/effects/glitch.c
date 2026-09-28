#include "glitch.h"

enum {
    GLITCH_START_FRAME = 48,
    GLITCH_PERIOD = 84,
    GLITCH_LENGTH = 5
};

static void set_glitch_bar(OBJATTR *oam, int index, int x, int y,
                            int color_bank, int tile_base) {
    oam[index].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_WIDE;
    oam[index].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
    oam[index].attr2 = OBJ_CHAR(tile_base + index * 4)
                     | OBJ_PALETTE(color_bank);
    oam[index].dummy = 0;
}

void glitch_update(OBJATTR *oam, u16 frame,
                    int target_index, int target_x, int target_y,
                    int tile_base) {
    u16 phase = frame % GLITCH_PERIOD;
    int active = phase >= GLITCH_START_FRAME
              && phase < GLITCH_START_FRAME + GLITCH_LENGTH;

    if (!active) {
        for (int i = 0; i < 4; ++i) {
            oam[i].attr0 = ATTR0_DISABLED;
        }
        oam[target_index].attr1 = OBJ_X(target_x) | ATTR1_SIZE_64;
        return;
    }

    int shift = (phase & 1) ? 5 : -4;
    int band_y = target_y + 12 + ((phase * 7) % 40);
    oam[target_index].attr1 = OBJ_X(target_x + shift) | ATTR1_SIZE_64;

    set_glitch_bar(oam, 0, target_x + shift, band_y, 1, tile_base);
    set_glitch_bar(oam, 1, target_x + shift + 32, band_y, 1, tile_base);
    set_glitch_bar(oam, 2, target_x + shift, band_y + 8, 2, tile_base);
    set_glitch_bar(oam, 3, target_x + shift + 32, band_y + 8, 3, tile_base);
}
