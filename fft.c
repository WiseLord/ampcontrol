#include "fft.h"
#include <avr/pgmspace.h>

static const int16_t sinTable[] PROGMEM =
{
		 0,   1605,   3196,   4755,   6269,   7722,   9101,  10393,
	 11584,  12664,  13621,  14448,  15135,  15677,  16068,  16304,
	 16383,  16304,  16068,  15677,  15135,  14448,  13621,  12664,
	 11584,  10393,   9101,   7722,   6269,   4755,   3196,   1605,
		 0,  -1605,  -3196,  -4755,  -6269,  -7722,  -9101, -10393,
	-11584, -12664, -13621, -14448, -15135, -15677, -16068, -16304,
	-16383, -16304, -16068, -15677, -15135, -14448, -13621, -12664,
	-11584, -10393,  -9101,  -7722,  -6269,  -4755,  -3196,  -1605,
};

static const int16_t dbTable[] PROGMEM =
{
		1,   1,    2,    2,    3,    4,    6,    8,
	   10,  14,   18,   24,   33,   44,   59,   78,
	  105, 140,  187,  250,  335,  448,  599,  801,
	1071, 1432, 1915, 2561, 3425, 4580, 6125, 8191,
};

void revBin(int16_t *fr)
{
	int8_t m, l, mr = 0;
	int16_t tr;

	for (m = 1; m < FFT_SIZE; m++)
	{
		l = FFT_SIZE;
		do
			l >>= 1;
		while (mr + l >= FFT_SIZE);

		mr = (mr & (l - 1)) + l;

		if (mr <= m)
			continue;
		tr = fr[m];
		fr[m] = fr[mr];
		fr[mr] = tr;
	}
	return;
}

static inline void sumDif(int16_t a, int16_t b, int16_t *s, int16_t *d)
{
	*s = a + b;
	*d = a - b;
}

static inline void multShf(int16_t cos, int16_t sin,
	int16_t x, int16_t y, int16_t *u, int16_t *v)
{
	*u = ((long)x * cos - (long)y * sin) >> 14;
	*v = ((long)y * cos + (long)x * sin) >> 14;
}

void fftRad4(int16_t *fr, int16_t *fi)
{
	uint8_t ldm = 0, rdx = 2;
	uint8_t i0, i1, i2, i3;
	int16_t xr, yr, ur, vr, xi, yi, ui, vi, t;
	int16_t cos1, sin1, cos2, sin2, cos3, sin3;
	uint8_t m, m4, phI0, phI;
	uint8_t r, i;

	for (i0 = 0; i0 < FFT_SIZE; i0 += 4)
	{
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

	for (ldm = 2 * rdx; ldm <= FFT_LOG2; ldm += rdx)
	{
		m = (1 << ldm);
		m4 = (m >> rdx);

		phI0 =  N_WAVE / m;
		phI  = 0;

		for (i = 0; i < m4; i++)
		{
			sin1 = pgm_read_word(&sinTable[phI]);
			sin2 = pgm_read_word(&sinTable[2 * phI]);
			sin3 = pgm_read_word(&sinTable[3 * phI]);

			cos1 = pgm_read_word(&sinTable[phI + N_WAVE / 4]);
			cos2 = pgm_read_word(&sinTable[2 * phI + N_WAVE / 4]);
			cos3 = pgm_read_word(&sinTable[3 * phI + N_WAVE / 4]);

			for (r = 0; r < FFT_SIZE; r += m)
			{
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
			phI += phI0;
		}
	}
	return;
}

void cplx2dB(int16_t *fr, int16_t *fi)
{
	uint8_t i, j;
	int16_t calc;
	for (i = 0; i < FFT_SIZE / 2; i++)
	{
		calc = ((long)fr[i] * fr[i] + (long)fi[i] * fi[i]) >> 13;

		for (j = 0; j < N_DB; j++)
			if (calc <= pgm_read_word(&dbTable[j]))
				break;
		fr[i] = j;
	}
	for (i = 0; i < FFT_SIZE; i++)
		fi[i] = 0;

	return;
}
