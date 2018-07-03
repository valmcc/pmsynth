//-----------------------------------------------------------------------------
/*

Waveguide synth

*/
//-----------------------------------------------------------------------------

#include "ggm.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// frequency to x scaling (xrange/fs)
#define WG_FSCALE ((float)(1ULL << 32) / AUDIO_FS)

#define WG_DELAY_MASK (WG_DELAY_SIZE - 1)
#define WG_FRAC_BITS (32U - WG_DELAY_BITS)
#define WG_FRAC_MASK ((1U << WG_FRAC_BITS) - 1)
#define WG_FRAC_SCALE (float)(1.f / (float)(1ULL << WG_FRAC_BITS))

//-----------------------------------------------------------------------------


void wg_gen(struct wg *osc, float *out, size_t n) {
	for (size_t i = 0; i < n; i++) {
		// pointer edition
		if (osc->estate == 1){
			float mallet_out = mallet_gen(osc);			
			osc->delay_l[osc->x_pos_l] += mallet_out;
			osc->delay_r[osc->x_pos_r] += mallet_out;

		}

		// nut reflection 
		osc->delay_r[osc->bridge_pos] = -1.0f * osc->delay_l[osc->nut_pos];
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

		// added scaling factor for frac due to linear interp varying amplitudes
		out[i] = 1.5f * (frac) * 0.5f * (osc->delay_l[osc->x_pos_l] + 
			osc->delay_r[osc->x_pos_r]);


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

	}
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
	osc->excite_pos = 3;
	osc->x_pos_l = osc->excite_pos;
	osc->x_pos_r = osc->delay_len - osc->excite_pos;
	osc->x_pos_l_2 = osc->x_pos_l + 1;
	osc->x_pos_r_2 = osc->x_pos_r + 1;
	osc->bridge_pos = osc->delay_len;
	osc->nut_pos = 0;
	osc->pickup_pos = 4;



}

//-----------------------------------------------------------------------------

void wg_ctrl_reflection(struct wg *osc, float reflection) {
	osc->r = reflection;
}

void wg_ctrl_frequency(struct wg *osc, float freq) {
	osc->freq = freq;
	float delay_len_total = (AUDIO_FS/freq/2)+1;
	osc->delay_len = (uint32_t) delay_len_total; // delay line length
	osc->delay_len_frac = delay_len_total - (float) osc->delay_len;

	//osc->xstep = (uint32_t) (osc->freq * WG_FSCALE);
	DBG("delay length: %d\r\n", osc->delay_len);
}

void wg_ctrl_stiffness(struct wg *osc, float stiffness) {
	osc->a = stiffness;
}

/*void wg_ctrl_pickup_pos(struct wg *osc, float pickup_pos) {
	osc->pp = pickup_pos;
}


void wg_ctrl_excite_pos(struct wg *osc, float excite_pos) {
	osc->ep = excite_pos;
}*/

void wg_init(struct wg *osc) {
	// setting all pass values
	osc->ap_state_1 = 0.0f;
	osc->ap_state_2 = 0.0f;
}

//-----------------------------------------------------------------------------