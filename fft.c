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

static const int16_t hammTable[] PROGMEM =
{
	 1310,  1348,  1460,  1645,  1902,  2228,  2620,  3073,
	 3585,  4148,  4758,  5409,  6093,  6806,  7538,  8284,
	 9035,  9784, 10524, 11247, 11947, 12615, 13246, 13834,
	14372, 14855, 15278, 15637, 15929, 16150, 16299, 16374,

};

static const int16_t dbTable[] PROGMEM =
{
	   1,    1,    1,    1,    2,    2,    2,    3,
	   3,    4,    4,    5,    6,    7,    8,    9,
	  11,   12,   14,   17,   19,   22,   25,   29,
	  34,   39,   45,   52,   60,   69,   79,   91,
	 105,  121,  140,  161,  185,  213,  245,  282,
	 324,  373,  430,  494,  569,  655,  754,  867,
	 998, 1148, 1321, 1520, 1749, 2013, 2316, 2665,
	3067, 3529, 4061, 4672, 5376, 6186, 7118, 8191,
};

void hammWindow(int16_t *fr)
{
	uint8_t i;
	for (i = 0; i < FFT_SIZE; i++)
		fr[i] = ((int32_t)fr[i] * pgm_read_word(
			&hammTable[i < FFT_SIZE / 2 ? i : FFT_SIZE - 1 - i])) >> 14;
	return;
}

void rev_bin(int16_t *fr)
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

static inline void sum_dif(int16_t a, int16_t b, int16_t *s, int16_t *d)
{
	*s = a + b;
	*d = a - b;
}

static inline void mult_shf(int16_t cos, int16_t sin,
	int16_t x, int16_t y, int16_t *u, int16_t *v)
{
	*u = ((long)x * cos - (long)y * sin) >> 14;
	*v = ((long)y * cos + (long)x * sin) >> 14;
}

void fft_radix4(int16_t *fr, int16_t *fi)
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

		sum_dif(fr[i0], fr[i1], &xr, &ur);
		sum_dif(fr[i2], fr[i3], &yr, &vi);
		sum_dif(fi[i0], fi[i1], &xi, &ui);
		sum_dif(fi[i3], fi[i2], &yi, &vr);

		sum_dif(ui, vi, &fi[i1], &fi[i3]);
		sum_dif(xi, yi, &fi[i0], &fi[i2]);
		sum_dif(ur, vr, &fr[i1], &fr[i3]);
		sum_dif(xr, yr, &fr[i0], &fr[i2]);
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

				mult_shf(cos2, sin2, fr[i1], fi[i1], &xr, &xi);
				mult_shf(cos1, sin1, fr[i2], fi[i2], &yr, &vr);
				mult_shf(cos3, sin3, fr[i3], fi[i3], &vi, &yi);

				t = yi - vr;
				yi += vr;
				vr = t;

				ur = fr[i0] - xr;
				xr += fr[i0];

				sum_dif(ur, vr, &fr[i1], &fr[i3]);

				t = yr - vi;
				yr += vi;
				vi = t;

				ui = fi[i0] - xi;
				xi += fi[i0];

				sum_dif(ui, vi, &fi[i1], &fi[i3]);
				sum_dif(xr, yr, &fr[i0], &fr[i2]);
				sum_dif(xi, yi, &fi[i0], &fi[i2]);
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
	return;
}
