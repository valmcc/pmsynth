//-----------------------------------------------------------------------------
/*

Fast (but slightly inaccurate) Power Functions

Notes:

math library powf(2.f, x) = 4.58uS
math library truncf() = 300nS

pow2_frac() = 186nS
pow2() = 500nS

*/
//-----------------------------------------------------------------------------

#include <math.h>

#include "pmsynth.h"

//-----------------------------------------------------------------------------

// See ./scripts/exp.py

static const uint16_t exp0_table[64] = {
	0x8000, 0x8165, 0x82ce, 0x843a, 0x85ab, 0x871f, 0x8898, 0x8a15, 0x8b96, 0x8d1b, 0x8ea4, 0x9032, 0x91c4, 0x935a, 0x94f5, 0x9694,
	0x9838, 0x99e0, 0x9b8d, 0x9d3f, 0x9ef5, 0xa0b0, 0xa270, 0xa435, 0xa5ff, 0xa7ce, 0xa9a1, 0xab7a, 0xad58, 0xaf3b, 0xb124, 0xb312,
	0xb505, 0xb6fe, 0xb8fc, 0xbaff, 0xbd09, 0xbf18, 0xc12c, 0xc347, 0xc567, 0xc78d, 0xc9ba, 0xcbec, 0xce25, 0xd063, 0xd2a8, 0xd4f3,
	0xd745, 0xd99d, 0xdbfc, 0xde61, 0xe0cd, 0xe340, 0xe5b9, 0xe839, 0xeac1, 0xed4f, 0xefe5, 0xf281, 0xf525, 0xf7d1, 0xfa84, 0xfd3e,
};

static const uint16_t exp1_table[64] = {
	0x8000, 0x8006, 0x800b, 0x8011, 0x8016, 0x801c, 0x8021, 0x8027, 0x802c, 0x8032, 0x8037, 0x803d, 0x8043, 0x8048, 0x804e, 0x8053,
	0x8059, 0x805e, 0x8064, 0x806a, 0x806f, 0x8075, 0x807a, 0x8080, 0x8085, 0x808b, 0x8090, 0x8096, 0x809c, 0x80a1, 0x80a7, 0x80ac,
	0x80b2, 0x80b8, 0x80bd, 0x80c3, 0x80c8, 0x80ce, 0x80d3, 0x80d9, 0x80df, 0x80e4, 0x80ea, 0x80ef, 0x80f5, 0x80fa, 0x8100, 0x8106,
	0x810b, 0x8111, 0x8116, 0x811c, 0x8122, 0x8127, 0x812d, 0x8132, 0x8138, 0x813e, 0x8143, 0x8149, 0x814e, 0x8154, 0x815a, 0x815f,
};

//-----------------------------------------------------------------------------

// return powf(2.f, x) where x is an integer [-126,127]
float pow2_int(int x) {
	float f;
	// make a float32 per IEEE754
	*(uint32_t *) & f = (127 + x) << 23;
	return f;
}

// return powf(2.f, x) where x = [0,1)
float pow2_frac(float x) {
	int n = (int)(x * (float)(1U << 12));
	uint16_t x0 = exp0_table[(n >> 6) & 0x3f];
	uint16_t x1 = exp1_table[n & 0x3f];
	return (float)(x0 * x1) * (1.f / (float)(1U << 30));
}

// return powf(2.f, x)
float pow2(float x) {
	float nf = truncf(x);
	float ff = x - nf;
	if (ff < 0) {
		nf -= 1.f;
		ff += 1.f;
	}
	return pow2_frac(ff) * pow2_int((int)nf);
}

#define LOG_E2 (1.4426950408889634f)	// 1.0 / math.log(2.0)

// return powf(e, x)
float powe(float x) {
	return pow2(LOG_E2 * x);
}

#define LOG_200 (5.29831736655f)
#define LOG_15000 (9.61580548008f)
// return  exp(ln(40) + knob * (ln(15000) - ln(40))) // for freq range of 40Hz to 15000Hz

float logmap(float x) {
	return powe(LOG_200 + x * (LOG_15000 - LOG_200));
}


//-----------------------------------------------------------------------------
