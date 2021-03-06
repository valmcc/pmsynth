//-----------------------------------------------------------------------------
/*

Patch 9

Waveguide woodwind instrument

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "pmsynth.h"

#define DEBUG
#include "logging.h"
#include "display.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct ww ww;
	struct pan pan;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
	float attenuate;
	float a;
	float d;
	float s;
	float r;
	float lp_filter_coef;
	float r_1;
	float r_2;
	float vibrato_amt;
	float noise_amt;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	if (v->note < 60){
		ww_set_samplerate(&vs->ww,2); // halve sample rate
	}
	if (v->note < 41){
		ww_set_samplerate(&vs->ww,4); // quarter sample rate
	}
	ww_ctrl_frequency(&vs->ww, midi_to_frequency((float)v->note - ps->bend));
}

static void ctrl_pan(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	pan_ctrl(&vs->pan, ps->vol, ps->pan);
}

static void ctrl_adsr(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_update(&vs->ww.adsr, ps->a, ps->d, ps->s, ps->r);
}

static void ctrl_coefs(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	ww_update_coefficients(&vs->ww, ps->lp_filter_coef, ps->r_1, ps->r_2);
}

static void ctrl_vib_noise(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	ww_update_vib_noise(&vs->ww, ps->vibrato_amt, ps->noise_amt);
}



//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p9 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	memset(vs, 0, sizeof(struct v_state));

	adsr_init(&vs->ww.adsr, ps->a, ps->d, ps->s, ps->r);
	//adsr_init(&vs->ww.adsr, 0.2f, 2.0f, 1.0f, 1.0f);
	noise_init(&vs->ww.ns);
	sin_init(&vs->ww.vibrato);
	sin_ctrl_frequency(&vs->ww.vibrato, 50);
	ww_init(&vs->ww);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_pan(v);
	ctrl_coefs(v);
	ctrl_vib_noise(v);

}

// stop the patch
static void stop(struct voice *v) {
	DBG("p9 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->ww.adsr);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	//DBG("p9 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	ww_set_velocity(&vs->ww, (float)vel / 127.f);
	gpio_set(IO_LED_AMBER);
	adsr_attack(&vs->ww.adsr);
	ww_blow(&vs->ww);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->ww.adsr);
	gpio_clr(IO_LED_AMBER);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	return 1;
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	struct v_state *vs = (struct v_state *)v->state;
	float out[n];
	ww_gen(&vs->ww, out, n);
	block_copy(out_l, out, n);
	block_mul_k(out_l, vs->pan.vol_l, n);
	//block_copy(out_r, out, n);
	//pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 0.1f;
	ps->pan = 0.5f;
	ps->bend = 0.f;
	ps->attenuate = 0.99f;
	ps->a = 0.1f;
	ps->d = 2.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
	ps->lp_filter_coef = 0.6f;
	ps->r_1 = 0.42f;
	ps->r_2 = 0.53f;
	ps->vibrato_amt = 0.008f;
	ps->noise_amt = 0.0085f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p9 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case VOLUME_SLIDER:		// volume
		ps->vol = midi_map(val, 0.f, 0.2f);
		update = 1;
		break;
	case MODWHEEL:		// filter cutoff
		svf2_ctrl_cutoff(&p->pmsynth->opf, logmap(midi_map(val, 1.0f, 0.0f)));
		break;
	case KNOB_1: 		// filter resonance
		svf2_ctrl_resonance(&p->pmsynth->opf, midi_map(val, 0.f, 0.98f));
		break;
	case KNOB_2:
		ps->r_1 = midi_map(val, 0.0f, 0.8f);
		ps->r_2 = midi_map(val, 0.2f, 0.46f);
		update = 4;
		break;
	case KNOB_3:
		ps->lp_filter_coef = midi_map(val, 0.0f, 1.0f);
		update = 4;
		break;
	case KNOB_4:
		ps->vibrato_amt = midi_map(val, 0.008f, 0.2f);
		ps->noise_amt = midi_map(val, 0.0085f, 0.09f);
		update = 5;
		break;
	case KNOB_5:
		ps->a = midi_map(val, 0.0f, 3.f);
		update = 3;
		break;
	case KNOB_6:
		ps->d = midi_map(val, 0.02f, 1.f);
		update = 3;
		break;
	case KNOB_7:
		ps->s = midi_map(val, 0.0f, 1.f);
		update = 3;
		break;
	case KNOB_8:
		ps->r = midi_map(val, 0.0f, 50.f);
		update = 3;
		break;
	case BUTTON_1:
		goto_next_patch(p);
		break;
	case BUTTON_6: //play demo song
		switch(p->pmsynth->seq0.m0.s_state){
			case 0:
				p->pmsynth->seq0.m0.s_state = 1;
				break;
			case 1:
				p->pmsynth->seq0.m0.s_state = 0;
				break;
			default:
				p->pmsynth->seq0.m0.s_state = 0;
				break;
			}
		break;
	case BUTTON_7: //panic button!
		stop_voices(p);
		break;
	default:
		break;
	}

	if (update == 1) {
		update_voices(p, ctrl_pan);
	}
	if (update == 3) {
		update_voices(p, ctrl_adsr);
	}
	if (update == 4) {
		update_voices(p, ctrl_coefs);
	}
	if (update == 5) {
		update_voices(p, ctrl_vib_noise);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p9 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	update_voices(p, ctrl_frequency);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch9 = {
	.start = start,
	.stop = stop,
	.note_on = note_on,
	.note_off = note_off,
	.active = active,
	.generate = generate,
	.init = init,
	.control_change = control_change,
	.pitch_wheel = pitch_wheel,
};

//-----------------------------------------------------------------------------
