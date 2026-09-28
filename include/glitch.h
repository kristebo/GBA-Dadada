#ifndef TG2027_GLITCH_H
#define TG2027_GLITCH_H

#include <gba.h>

enum {
    /* Each bar is a 32x32 4bpp sprite (4x4 tiles = 16 tiles), and there
       are 4 bars, so the caller must pre-fill this many solid-color
       tiles starting at tile_base before calling glitch_update(). */
    GLITCH_BAR_COUNT = 4,
    GLITCH_BAR_TILES = 16,
    GLITCH_TILES_REQUIRED = GLITCH_BAR_COUNT * GLITCH_BAR_TILES
};

/* Sprite-band glitch effect: periodically shakes a target OBJ sprite
   sideways and flashes four colored "static" bars behind it. Reserves
   OAM slots 0-3 for the bars; the caller owns and DMAs the OAM buffer.

   tile_base is the first tile index of GLITCH_TILES_REQUIRED
   pre-filled solid 32x32 tiles used for the bars (one 32x32 block per
   OAM_PALETTE bank). Call every frame with an incrementing frame
   counter; when the glitch is inactive the target sprite is restored
   to (target_x, target_y) and the bars are disabled. */
void glitch_update(OBJATTR *oam, u16 frame,
                    int target_index, int target_x, int target_y,
                    int tile_base);

#endif // TG2027_GLITCH_H
