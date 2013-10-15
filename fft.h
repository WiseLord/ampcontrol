#ifndef FFT_H
#define FFT_H

#include <inttypes.h>

#define FFT_SIZE	64
#define FFT_LOG2	6
#define N_WAVE		64
#define N_DB		64

void hammWindow(int16_t *fx);
void rev_bin(int16_t *fr);
void fft_radix4(int16_t *fr, int16_t *fi);
void cplx2dB(int16_t *fr, int16_t *fi);

#endif // FFT_H
