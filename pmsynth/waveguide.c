//-----------------------------------------------------------------------------
/*

Waveguide synth

*/
//-----------------------------------------------------------------------------

#include "pmsynth.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// location of pickup (where the output is extracted from the delay line)
#define WG_PICKUP_POS 4

//-----------------------------------------------------------------------------


void wg_gen(struct wg *osc, float *out, size_t n) {
	float am[n];
	adsr_gen(&osc->adsr, am, n);
	for (size_t i = 0; i < n; i++) {
		// pointer edition



		if (i % osc->downsample_amt == 0){
			float mallet_out = 0;
			if (osc->estate == 1){
				mallet_out = impulse_gen(osc);		
				osc->delay_l[osc->x_pos_l] += mallet_out;
				osc->delay_r[osc->x_pos_r] += mallet_out;
				//if (osc->impulse_solo){
				//	out[i] = mallet_out;
				//}
			}

			//DBG("epos=%d, etate=%d, mallet_out=%d\r\n",osc->epos,osc->estate, (int) (mallet_out*1000));
			// nut reflection 
			osc->delay_r[osc->bridge_pos] = osc->tube * osc->delay_l[osc->nut_pos];
			// bridge reflection
			osc->delay_l[osc->bridge_pos] =  osc->r * osc->delay_r[osc->nut_pos];

			// all pass filter for stiffness (when used for pitch correction it changes the "stiffness")
			osc->delay_l[osc->x_pos_l_2] = osc->a * osc->delay_l[osc->x_pos_l_2] +
									osc->delay_l[osc->x_pos_l] -
									osc->a * osc->delay_l[osc->x_pos_l];


			// with linear interp
			float frac = (float) 1.0f - osc->delay_len_frac;
			osc->delay_l[osc->x_pos_l] = (1.0f - frac) * osc->delay_l[osc->x_pos_l]+
								(frac) * osc->delay_l[osc->x_pos_l_2];
			osc->delay_r[osc->x_pos_r] = (1.0f - frac) * osc->delay_r[osc->x_pos_r]+
								(frac) * osc->delay_r[osc->x_pos_r_2];


			// added scaling factor for frac due to linear interp varying amplitudes due to low pass effect
			
			out[i] = mallet_out * osc->impulse_solo + (
				((osc->velocity) / 0.8f + 0.2f) * 0.75f * (frac) * (osc->delay_l[osc->x_pos_l] + 
				osc->delay_r[osc->x_pos_r]))*(1.0f - osc->impulse_solo);



			//stepping and wrapping pointers
			osc->x_pos_l += 1;
			if (osc->x_pos_l > osc->delay_len){
				osc->x_pos_l = 0;
			}

			osc->x_pos_l_2 += 1;
			if (osc->x_pos_l_2 > osc->delay_len){
				osc->x_pos_l_2 = 0;
			}

			osc->x_pos_r += 1;
			if (osc->x_pos_r > osc->delay_len){
				osc->x_pos_r = 0;
			}

			osc->x_pos_r_2 += 1;
			if (osc->x_pos_r_2 > osc->delay_len){
				osc->x_pos_r_2 = 0;
			}

			osc->bridge_pos += 1;
			if (osc->bridge_pos > osc->delay_len){
				osc->bridge_pos = 0;
			}

			osc->nut_pos += 1;
			if (osc->nut_pos > osc->delay_len){
				osc->nut_pos = 0;
			}

			osc->pickup_pos += 1;
			if (osc->pickup_pos > osc->delay_len){
				osc->pickup_pos = 0;
			}
		} else
		out[i] = out[i] * 0.5f + out[i-1] *0.5f; // linear interp
		osc->epos += 1; // incrementing impulse sample
	} 
	block_mul(out, am, n);
	//svf2_gen_lpf(&osc->opf, out, out, n, FILT_LOW_PASS); //TODO remove
}

//-----------------------------------------------------------------------------

void wg_excite(struct wg *osc) {
	// On each time interval, insert the next sample point of the exciter
	// sample into a point of the delay line.

	// for now it sets a flag for wg_gen to begin adding this to the delay
	// line
	osc->estate = 1;
	osc->epos = 0;

	// setting up the positions of the various parts of the delay line
	//
	// bridge                  pickup         exciter            nut
	// |                         x               x                |
	// ------------------------------------------------------------
	//
	//
	//osc->excite_pos = 3;
	osc->x_pos_l = osc->excite_pos;
	osc->x_pos_r = (osc->delay_len) - osc->excite_pos;
	osc->x_pos_l_2 = osc->x_pos_l + 1;
	osc->x_pos_r_2 = osc->x_pos_r + 1;
	osc->bridge_pos = osc->delay_len;
	osc->nut_pos = 0;
	osc->pickup_pos = WG_PICKUP_POS;



}


//-----------------------------------------------------------------------------

void wg_ctrl_impulse_type(struct wg *osc, int impulse) {
	osc->epos = 0;
	osc->estate = 0;
	osc->impulse = impulse;
}

void wg_ctrl_reflection(struct wg *osc, float reflection) {
	osc->r = reflection;
}

void wg_ctrl_frequency(struct wg *osc, float freq) {
	osc->freq = freq;
	osc->delay_len_total = (AUDIO_FS/freq/2.0f/osc->downsample_amt)+1;
	osc->delay_len = (uint32_t) osc->delay_len_total; // delay line length
	osc->delay_len_frac = osc->delay_len_total - (float) osc->delay_len;
	//DBG("delay length: %d\r\n", osc->delay_len);
}

void wg_ctrl_stiffness(struct wg *osc, float stiffness) {
	osc->a = stiffness;
}

void wg_ctrl_pos(struct wg *osc, float excite_loc) {
	osc->excite_loc = excite_loc;
	osc->excite_pos = excite_loc * osc->delay_len_total;
	// needs to update position for current voices
}

void wg_ctrl_impulse_solo(struct wg *osc, int impulse_solo) {
	osc->impulse_solo = impulse_solo;
	//DBG("einc: %d\r\n", osc->einc);
	// brightness (speed of exciter pluck)
}

void wg_set_velocity(struct wg *osc, float velocity) {
	osc->velocity = velocity;
	// velocity adjusts volume and brightness
}

void wg_set_samplerate(struct wg *osc, float downsample_amt) {
	osc->downsample_amt = downsample_amt;
}

void wg_exciter_type(struct wg *osc, int exciter_type) {
	switch(exciter_type){
		case 0: //struck string
			osc->tube = -1;
			break;
		case 1: // struck tube
			osc->tube = 1;
			break;
		default:
			break;
	}

}

void wg_init(struct wg *osc) {
	// setting all pass values
	osc->ap_state_1 = 0.0f;
	osc->ap_state_2 = 0.0f;
	osc->downsample_amt = 1;
}

//-----------------------------------------------------------------------------
