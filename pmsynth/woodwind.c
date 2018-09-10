//-----------------------------------------------------------------------------
/*

Woodwind modelling (with the use of waveguides)

*/
//-----------------------------------------------------------------------------

#include "pmsynth.h"
#include "utils.h"

#define DEBUG
#include "logging.h"
#include "display.h"
#include "lcd.h"


//-----------------------------------------------------------------------------

// frequency to x scaling (xrange/fs)
#define WW_FSCALE ((float)(1ULL << 32) / AUDIO_FS)
#define WW_FSCALE_2 ((float)(1ULL << 32) / (AUDIO_FS / 2.0f))

#define WW_DELAY_MASK (WW_DELAY_SIZE - 1)
#define WW_FRAC_BITS (32U - WW_DELAY_BITS)
#define WW_FRAC_MASK ((1U << WW_FRAC_BITS) - 1)
#define WW_FRAC_SCALE (float)(1.f / (float)(1ULL << WW_FRAC_BITS))

#define R_1 0.42f
#define R_2 0.53f
#define FILTER_COEF 0.6f
#define DC_FILT_GAIN 0.99f

//-----------------------------------------------------------------------------

void ww_gen(struct ww *osc, float *out, size_t n) {

	// input generation
	//-----------------------------------------------------------------------------

	float am[n];
	float breath[n];
	float vibrato[n];

	sin_gen(&osc->vibrato, vibrato, NULL, n);
	adsr_gen(&osc->adsr, am, n);
	noise_gen_white(&osc->ns, breath, n);

	block_mul(breath, am, n); // white noise following adsr
	block_mul_k(breath, 0.0085f, n); // scaling white noise

	block_mul_k(vibrato, 0.008f, n); // scaling vibrato
	block_add(breath, vibrato, n); // adding vibrato to white noise

	block_add(breath, am, n); // adding to adsr to make pressure input

/*	 pressure input looks a bit like:
		  
		 _-_-_-_-_-_-_
		/             \
	   /               \
	  /                 \ (with dc offset)*/

	
	// delay line calcs
	for (size_t i = 0; i < n; i++) {

			if (i % osc->downsample_amt == 0){
			// delay line 2 (for jet reed)
			osc->dl_2[osc->dl_2_ptr_in] = breath[i] + osc->r_1 * osc->dl_1_out;

			// stepping and wrapping pointers for delay line 2
			osc->dl_2_ptr_in += 1;
			if (osc->dl_2_ptr_in > osc->dl_2_len){
				osc->dl_2_ptr_in = 0;
			}

			osc->dl_2_ptr_out += 1;
			if (osc->dl_2_ptr_out > osc->dl_2_len){
				osc->dl_2_ptr_out = 0;
			}

			float reed_out = osc->dl_2[osc->dl_2_ptr_out];

			// summing and adding
			reed_out = reed_out - (pow2(reed_out) * reed_out);
			reed_out = reed_out + osc->r_2 * osc->dl_1_out;

			// low pass filter
			float flute_out = osc->flute_out_old + osc->lp_filter_coef * (reed_out - osc->flute_out_old);
			osc->flute_out_old = flute_out;

			// for delay line 1
			osc->dl_1[osc->dl_1_ptr_in] = flute_out;

			// stepping and wrapping pointers for delay line 1
			osc->dl_1_ptr_in += 1;
			if (osc->dl_1_ptr_in > osc->dl_1_len){
				osc->dl_1_ptr_in = 0;
			}

			osc->dl_1_ptr_out += 1;
			if (osc->dl_1_ptr_out > osc->dl_1_len){
				osc->dl_1_ptr_out = 0;
			}

			// setting output
			osc->dl_1_out = osc->dl_1[osc->dl_1_ptr_out];

			// outputting audio
			out[i] = flute_out;

			// dc block

			osc->dc_filt_out = flute_out - osc->dc_filt_in + (DC_FILT_GAIN * osc->dc_filt_out);
			osc->dc_filt_in = flute_out;
			out[i] = osc->dc_filt_out;
			//out[i] = breath[i];
		} else
		out[i] = out [i-1]; // sample and hold (least cpu)

	}
}

//-----------------------------------------------------------------------------

void ww_blow(struct ww *osc) {
	osc->estate = 1;

	// delay line 2 (for jet reed)
	osc->dl_2_ptr_in = osc->dl_2_len;
	osc->dl_2_ptr_out = 0;

	// delay line 1 (reflection from the output hole)

	osc->dl_1_ptr_in = osc->dl_1_len;
	osc->dl_1_ptr_out = 0;


}

//-----------------------------------------------------------------------------

void ww_ctrl_attenuate(struct ww *osc, float attenuate) {
	//osc->k = 0.5f * attenuate;
}

void ww_set_samplerate(struct ww *osc, float downsample_amt) {
	osc->downsample_amt = downsample_amt;
}

void ww_update_coefficients(struct ww *osc, float lp_filter_coef, float r_1, float r_2) {
	osc->lp_filter_coef = lp_filter_coef;
	osc->r_1 = r_1;
	osc->r_2 = r_2;
}

void ww_ctrl_frequency(struct ww *osc, float freq) {
	osc->freq = freq;

	osc->dl_1_len_total = (AUDIO_FS/freq)/osc->downsample_amt;
	osc->dl_1_len = (uint32_t) osc->dl_1_len_total/2.0f; // delay line 1 length
	osc->dl_1_len_frac = osc->dl_1_len_total - (float) osc->dl_1_len;

	osc->dl_2_len_total = osc->dl_1_len_total/4.0f;
	osc->dl_2_len = (uint32_t) osc->dl_2_len_total; // delay line 2 length
	osc->dl_2_len_frac = osc->dl_2_len_total - (float) osc->dl_2_len;

	DBG("freq: %d, delay 1 length: %d, delay 2 length: %d\r\n", (int)freq, osc->dl_1_len, osc->dl_2_len);

}

void ww_init(struct ww *osc) {
	osc->downsample_amt = 1;
}

//-----------------------------------------------------------------------------
