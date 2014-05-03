#include "fft.h"
#include <avr/pgmspace.h>

static const int16_t dbTable[N_DB] PROGMEM = {
	   1,    1,    2,    2,    3,    4,    6,    8,
	  10,   14,   18,   24,   33,   44,   59,   78,
	 105,  140,  187,  250,  335,  448,  599,  801,
	1071, 1432, 1915, 2561, 3425, 4580, 6125, 8191,
};

static const uint8_t sinTable[N_WAVE / 4 + 1] PROGMEM = {
	   0,   25,   50,   74,   98,  120,  142,  162,
	 180,  197,  212,  225,  236,  244,  250,  254,
	 255
};

#define mshf_16( a, b)    \
({                        \
int prod, val1=a, val2=b; \
__asm__ __volatile__ (    \
"muls %B1, %B2	\n\t"     \
"mov %B0, r0    \n\t"	  \
"mul %A1, %A2   \n\t"	  \
"mov %A0, r1    \n\t"     \
"mulsu %B1, %A2	\n\t"     \
"add %A0, r0    \n\t"     \
"adc %B0, r1    \n\t"     \
"mulsu %B2, %A1	\n\t"     \
"add %A0, r0    \n\t"     \
"adc %B0, r1    \n\t"     \
"clr r1         \n\t"     \
: "=&d" (prod)            \
: "a" (val1), "a" (val2)  \
);                        \
prod;                     \
})

static int16_t sinTbl(uint8_t phi)
{
	int16_t ret;
	uint8_t neg = 0;

	if (phi >= N_WAVE / 2) {
		phi -= N_WAVE / 2;
		neg = 1;
	}
	if (phi >= N_WAVE / 4)
		phi = N_WAVE / 2 - phi;

	ret = pgm_read_byte(&sinTable[phi]);

	return neg ? -ret : ret;
}

static inline void sumDif(int16_t a, int16_t b, int16_t *s, int16_t *d)
{
	*s = a + b;
	*d = a - b;
}

static inline void multShf(int16_t cos, int16_t sin,
	int16_t x, int16_t y, int16_t *u, int16_t *v)
{
	*u = (mshf_16(x, cos) - mshf_16(y, sin));
	*v = (mshf_16(y, cos) + mshf_16(x, sin));
}

void fftRad4(int16_t *fr, int16_t *fi)
{
	uint8_t ldm, rdx = 2;
	uint8_t i0, i1, i2, i3;
	int16_t xr, yr, ur, vr, xi, yi, ui, vi, t;
	int16_t cos1, sin1, cos2, sin2, cos3, sin3;
	uint8_t m, m4, phi0, phi;
	uint8_t r, i;

	for (i0 = 0; i0 < FFT_SIZE; i0 += 4) {
		i1 = i0 + 1;
		i2 = i1 + 1;
		i3 = i2 + 1;

		sumDif(fr[i0], fr[i1], &xr, &ur);
		sumDif(fr[i2], fr[i3], &yr, &vi);
		sumDif(fi[i0], fi[i1], &xi, &ui);
		sumDif(fi[i3], fi[i2], &yi, &vr);

		sumDif(ui, vi, &fi[i1], &fi[i3]);
		sumDif(xi, yi, &fi[i0], &fi[i2]);
		sumDif(ur, vr, &fr[i1], &fr[i3]);
		sumDif(xr, yr, &fr[i0], &fr[i2]);
	}

	for (ldm = 2 * rdx; ldm <= FFT_LOG2; ldm += rdx) {
		m = (1 << ldm);
		m4 = (m >> rdx);

		phi0 = N_WAVE / m;
		phi  = 0;

		for (i = 0; i < m4; i++) {
			sin1 = sinTbl(phi);
			sin2 = sinTbl(2 * phi);
			sin3 = sinTbl(3 * phi);

			cos1 = sinTbl(phi + N_WAVE / 4);
			cos2 = sinTbl(2 * phi + N_WAVE / 4);
			cos3 = sinTbl(3 * phi + N_WAVE / 4);

			for (r = 0; r < FFT_SIZE; r += m) {
				i0 = i + r;
				i1 = i0 + m4;
				i2 = i1 + m4;
				i3 = i2 + m4;

				multShf(cos2, sin2, fr[i1], fi[i1], &xr, &xi);
				multShf(cos1, sin1, fr[i2], fi[i2], &yr, &vr);
				multShf(cos3, sin3, fr[i3], fi[i3], &vi, &yi);

				t = yi - vr;
				yi += vr;
				vr = t;

				ur = fr[i0] - xr;
				xr += fr[i0];

				sumDif(ur, vr, &fr[i1], &fr[i3]);

				t = yr - vi;
				yr += vi;
				vi = t;

				ui = fi[i0] - xi;
				xi += fi[i0];

				sumDif(ui, vi, &fi[i1], &fi[i3]);
				sumDif(xr, yr, &fr[i0], &fr[i2]);
				sumDif(xi, yi, &fi[i0], &fi[i2]);
			}
			phi += phi0;
		}
	}
	return;
}

void cplx2dB(int16_t *fr, int16_t *fi)
{
	uint8_t i, j;
	int16_t calc;
	for (i = 0; i < FFT_SIZE / 2; i++) {
		calc = ((int32_t)fr[i] * fr[i] + (int32_t)fi[i] * fi[i]) >> 13;

		for (j = 0; j < N_DB; j++)
			if (calc <= pgm_read_word(&dbTable[j]))
				break;
		fr[i] = j;
	}
	for (i = 0; i < FFT_SIZE; i++)
		fi[i] = 0;
	return;
}
