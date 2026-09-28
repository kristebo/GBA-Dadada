#ifndef TG2027_SHADEBOB_H
#define TG2027_SHADEBOB_H

/* Scene 3: shadebob. Ported from the SDL "Retro Shadebob Effect" by
   B. Ellacott / W.P. van Paassen. Runs a heat-blob along a looping
   path with a fading trail in GBA Mode 4.

   Assumes irqInit()/irqEnable(IRQ_VBLANK) have already been called.
   Never returns. */
void shadebob_run(void);

#endif // TG2027_SHADEBOB_H
