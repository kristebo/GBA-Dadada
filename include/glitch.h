#ifndef TG2027_GLITCH_H
#define TG2027_GLITCH_H

#include <gba.h>

/* Sprite-band glitch effect: periodically shakes a target OBJ sprite
   sideways and flashes four colored "static" bars behind it. Reserves
   OAM slots 0-3 for the bars; the caller owns and DMAs the OAM buffer.

   tile_base is the first tile index of the 4 pre-filled 32x8 solid
   tiles used for the bars (4 tiles per bar, one per OAM_PALETTE bank).
   Call every frame with an incrementing frame counter; when the
   glitch is inactive the target sprite is restored to
   (target_x, target_y) and the bars are disabled. */
void glitch_update(OBJATTR *oam, u16 frame,
                    int target_index, int target_x, int target_y,
                    int tile_base);

#endif // TG2027_GLITCH_H
