#ifndef __PAD_H__
#define __PAD_H__

int waitPadReady(int port, int slot);
int initializePad(int port, int slot);
void waitVSync();

#endif  // __PAD_H__