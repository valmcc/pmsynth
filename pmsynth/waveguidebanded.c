//-----------------------------------------------------------------------------
/*

Banded waveguide model

*/
//-----------------------------------------------------------------------------

#include "pmsynth.h"
#include "utils.h"

#define DEBUG
#include "logging.h"
#include <math.h>
//-----------------------------------------------------------------------------
// frequency to x scaling (xrange/fs)
#define KS_FSCALE ((float)(1ULL << 32) / AUDIO_FS)

#define KS_DELAY_MASK (KS_DELAY_SIZE - 1)
#define KS_FRAC_BITS (32U - KS_DELAY_BITS)
#define KS_FRAC_MASK ((1U << KS_FRAC_BITS) - 1)
#define KS_FRAC_SCALE (float)(1.f / (float)(1ULL << KS_FRAC_BITS))

void wgb_gen(struct wgb *osc, float *out, size_t n) {
	float am[n];
	adsr_gen(&osc->adsr, am, n);
	for (size_t i = 0; i < n; i++) {
		out[i] = 0.0f;

		for (size_t j = 0; j < NUM_MODES; j++) {
			if (i % osc->mode[j].downsample_amt == 0){
				// mallet hit
				float mallet_out = 0;
				if (osc->estate == 1){
					mallet_out = impulse_gen_wgb(osc);		
					osc->mode[j].delay[osc->mode[j].dl_ptr_in] += mallet_out;
					// uncomment for direct mallet output
					//out[i] = mallet_out;
				}

				out[i] += osc->mode[j].delay[osc->mode[j].dl_ptr_out]*osc->mode[j].mix_factor;
				svf2_gen(&osc->mode[j].bpf, &osc->mode[j].delay[osc->mode[j].dl_ptr_out], &osc->mode[j].delay[osc->mode[j].dl_ptr_in], 1, FILT_BAND_PASS);

				//---------------------------------------------------------
				// linear interp for tuning, currently only applies to lowest harmonic to save cpu
				if (j == 0){
				float frac = (float) osc->mode[j].delay_len_frac;

				osc->mode[j].delay[osc->mode[j].dl_ptr_lin_tuner_2] = (frac) * osc->mode[j].delay[osc->mode[j].dl_ptr_lin_tuner_1] + (1.0f - frac) * osc->mode[j].delay[osc->mode[j].dl_ptr_lin_tuner_2];
				
				// wrapping linear interp pointers 
				osc->mode[j].dl_ptr_lin_tuner_1 += 1;
					if (osc->mode[j].dl_ptr_lin_tuner_1 > osc->mode[j].delay_len){
						osc->mode[j].dl_ptr_lin_tuner_1 = 0;
					}
				osc->mode[j].dl_ptr_lin_tuner_2 += 1;
					if (osc->mode[j].dl_ptr_lin_tuner_2 > osc->mode[j].delay_len){
						osc->mode[j].dl_ptr_lin_tuner_2 = 0;
					}
				}
				//----------------------------------------------------------

				osc->mode[j].dl_ptr_in += 1;
					if (osc->mode[j].dl_ptr_in > osc->mode[j].delay_len){
						osc->mode[j].dl_ptr_in = 0;
					}
				osc->mode[j].dl_ptr_out += 1;
					if (osc->mode[j].dl_ptr_out > osc->mode[j].delay_len){
						osc->mode[j].dl_ptr_out = 0;
						// mixing between modes
						osc->mode[j].delay[osc->mode[j].dl_ptr_out] = (osc->mode_mix_amt) * out[i] + (1.0f - osc->mode_mix_amt) * osc->mode[j].delay[osc->mode[j].dl_ptr_out];
					}
				} else {
				out[i] = out[i] * 0.5f + out[i-1] *0.5f; // linear interp on output when downsampling
				}
			}
		//all pass filter for tuning!
		// float temp = out[i];
		// out[i] = - osc->a * out[i]+ osc->ap_old_in + osc->a * osc->ap_old_out;
		// osc->ap_old_out = out[i];
		// osc->ap_old_in = temp;
	}
	block_mul(out, am, n);

	// low pass linked to envelope ended up being too cpu intensive so was removed
	//svf2_gen_lpf(&osc->opf, out, out, n, FILT_LOW_PASS);

	block_mul_k(out, (osc->velocity / 0.8f + 0.2f), n);
}

//-----------------------------------------------------------------------------


void wgb_ctrl_opf_freq(struct wgb *osc, float freq) {
	// tuning the frequency to a reasonable range
	freq *= 124.5;
	freq = 440.0 * pow2((freq - 57.0)/12.0);
	svf2_ctrl_cutoff(&osc->opf, freq);

}
void wgb_ctrl_opf_res(struct wgb *osc, float res) {
	svf2_ctrl_resonance(&osc->opf, res);

}


//-----------------------------------------------------------------------------

void wgb_pluck(struct wgb *osc) {
	osc->estate = 1;
	osc->epos = 0;
	for (size_t i = 0; i < NUM_MODES; i++) {
		osc->mode[i].dl_ptr_out = 1;
		osc->mode[i].dl_ptr_in = 0;

		osc->mode[i].dl_ptr_lin_tuner_1 = 2;
		osc->mode[i].dl_ptr_lin_tuner_2 = 3;
	}

}

//-----------------------------------------------------------------------------

void wgb_ctrl_attenuate(struct wgb *osc, float attenuate) {
	// do nothing
}

void wgb_ctrl_frequency(struct wgb *osc, float freq) {

	osc->freq = freq;
	osc->mode[0].freq_coef = 1.0f;
	//osc->mode[1].freq_coef = 2.756f;
	//osc->mode[2].freq_coef = 5.404f;
	float mode_1_freq = 4.0198391420f;
	float mode_2_freq = 10.718498659f;
	float h_mod = osc->h_coef * 0.5f;
	osc->mode[1].freq_coef = h_mod * mode_1_freq + mode_1_freq;
	osc->mode[2].freq_coef = h_mod * mode_2_freq + mode_2_freq;
	osc->mode[0].mix_factor = 1.0f - 0.2f * clampf(osc->brightness,0.8f, 1.0f); 
	osc->mode[1].mix_factor = 1.2f * clampf(osc->brightness,0.f, 0.5f); 
	osc->mode[2].mix_factor = 0.3f * osc->brightness; 

	for (size_t i = 0; i < NUM_MODES; i++) {		

		uint32_t delay_len = AUDIO_FS/(osc->mode[i].freq_coef * freq);

		if (delay_len > WGB_DELAY_SIZE) {
			uint32_t rough_ds = (delay_len / WGB_DELAY_SIZE) + 1;
			osc->mode[i].downsample_amt = rough_ds + rough_ds % 2;

			osc->mode[i].delay_len_total = delay_len / osc->mode[i].downsample_amt;

			osc->mode[i].delay_len  = (uint32_t) osc->mode[i].delay_len_total;
			osc->mode[i].delay_len_frac = osc->mode[i].delay_len_total - (float) osc->mode[i].delay_len;

		} else {
			osc->mode[i].downsample_amt = 1;
			osc->mode[i].delay_len_total = delay_len;
			osc->mode[i].delay_len  = (uint32_t) osc->mode[i].delay_len_total;
			osc->mode[i].delay_len_frac = osc->mode[i].delay_len_total - (float) osc->mode[i].delay_len;
		}
		DBG("delay length for mode %d: %d.%d \r\n", i, osc->mode[i].delay_len, (uint32_t) osc->mode[i].delay_len_frac * 100.0f);
		svf2_ctrl_cutoff(&osc->mode[i].bpf, osc->mode[i].freq_coef * freq * osc->mode[i].downsample_amt);
		svf2_ctrl_resonance(&osc->mode[i].bpf, 0.4999999f);
		
		//DBG("downsample amount: %d delay length: %d\r\n", osc->mode[i].downsample_amt, osc->mode[i].delay_len);

	}

}

void wgb_set_velocity(struct wgb *osc, float velocity) {
	osc->velocity = velocity;
	// velocity adjusts volume and brightness
}

void wgb_ctrl_brightness(struct wgb *osc, float brightness) {
	osc->brightness = brightness;
}

void wgb_ctrl_harmonic_mod(struct wgb *osc, float h_coef) {
	osc->h_coef = h_coef;
}

void wgb_ctrl_mode_mix_amt(struct wgb *osc, float mode_mix_amt) {
	osc->mode_mix_amt = mode_mix_amt;
}

void wgb_init(struct wgb *osc) {
	// do nothing
}

//-----------------------------------------------------------------------------
