#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Be aware that if you use a CRT (Monitor/Television) this may sort of burn in when used very long time.
// On todays (as of 2026) monitors there is almost no burning in, and if some is burn in it reverses after some time again.

unsigned long ziffern[] = {
#include "ZIFFERN.BIN"
};

void setBiosGraphicsMode(unsigned short mode) {
  union REGS regs;
  regs.w.ax = mode;
  int86(0x10, &regs, &regs);
}
 
unsigned short getBiosGraphicsMode() {
  union REGS regs;
  regs.w.ax = 0x0f00;
  int86(0x10, &regs, &regs);
  return regs.h.al;
}

// cli and sti are strongly needed at cmos readouts
#ifdef __WATCOMC__
#pragma aux dosCLI="cli" modify[]
#pragma aux dosSTI="sti" modify[]
#endif // __WATCOMC__

#ifdef __DJGPP__
void dosCLI() {__asm__ __volatile__("cli");}
void dosSTI() {__asm__ __volatile__("sti");}
#endif // __DJGPP__

unsigned char dosCmosRead(unsigned char reg, bool maskNMI) {
  dosCLI();
  outp(0x70, reg | (maskNMI?0x80:00));
  unsigned char r = (unsigned char)inp(0x71);
  dosSTI();
  return r;
}

unsigned char dosCmosUpdateInProgress() {
  return (unsigned char)(dosCmosRead(0x0a, true) & 0x80);
}

int rtcRead(unsigned char reg) {
  while(dosCmosUpdateInProgress()) {;}
  return (int)dosCmosRead(reg, true); // the NMI doesn't get reenabled after this, it would be more problematic if it is enabled during this
}

void clearScreenRam() {
  int y,j,x;
  for (y = 0; y < 480; y++) {
    for (j = 0; j < 4; j++) { // j = bit plane
      outp(0x3c4, 2);
      outp(0x3c5, 1<<j);
      for (x = 0; x < 640 / 8 / 4; x++) {
        *(unsigned long __far *)MK_FP(0xa000,y*(640/8)+x*4) = 0;
      }
    }
  }
}

void blitZiffer(long xp, long yp, long letter) {
  long y,j,x;
  for (y = 0; y < 128; y++) {
    for (j = 0; j < 4; j++) { // j = bit plane
      outp(0x3c4, 2);
      outp(0x3c5, 1<<j);
      unsigned long *read = &ziffern[letter*((96*128)/32)+y*(96/32)];
      for (x = 0; x < 96/32; x++) {
        *(unsigned long __far *)MK_FP(0xa000,(y+yp)*(640/8)+x*4+xp/32*4) = *read++;
      }
    }
  }
}

void update() {
  int rtc_seconds = rtcRead(0);
  int rtc_minutes = rtcRead(2);
  int rtc_hours   = rtcRead(4);
  int rtc_day     = rtcRead(7);
  int rtc_month   = rtcRead(8);
  int rtc_year    = rtcRead(9);
  long yp = (480-128*2) / 2;
  long xend = 640;
  xend -= 96; blitZiffer(xend,yp,0);
  xend -= 96; blitZiffer(xend,yp,(rtc_minutes & 15) + 1);
  xend -= 96; blitZiffer(xend,yp,(rtc_minutes>>4) + 1);
  xend -= 96; blitZiffer(xend,yp,0);
  xend -= 96; blitZiffer(xend,yp,(rtc_hours & 15) + 1);
  xend -= 96; blitZiffer(xend,yp,(rtc_hours>>4) + 1);
  yp += 128; 
  xend = 640;
  xend -= 96; blitZiffer(xend,yp,(rtc_seconds & 15) + 1);
  xend -= 96; blitZiffer(xend,yp,(rtc_seconds>>4) + 1);
/*  yp = 0;
  long xp = 0;
  blitZiffer(xp,yp,(rtc_day>>4) + 1); xp += 96;
  blitZiffer(xp,yp,(rtc_day&15) + 1); xp += 96;
  xp += 32;
  blitZiffer(xp,yp,(rtc_month>>4) + 1); xp += 96;
  blitZiffer(xp,yp,(rtc_month&15) + 1); xp += 96;
*/
}

int main(int argc, const char *argv[]) {
  unsigned short mode = getBiosGraphicsMode();

  setBiosGraphicsMode(0x12); // 640x480 with 16 colors
  clearScreenRam();

  while(!kbhit()) {
    update();
  }

  setBiosGraphicsMode(mode);

  return 0;
}
