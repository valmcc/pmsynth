//-----------------------------------------------------------------------------
/*

Patch 10

Banded Waveguide model

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
	struct wgb wgb;
	struct pan pan;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
	float attenuate;
	float brightness;
	float bp_res;
	float mode_mix_amt;
	float h_coef;
	float a;
	float d;
	float s;
	float r;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_frequency(&vs->wgb, midi_to_frequency((float)v->note + ps->bend));
}

static void ctrl_attenuate(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_attenuate(&vs->wgb, ps->attenuate);
}

static void ctrl_pan(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	pan_ctrl(&vs->pan, ps->vol, ps->pan);
}

static void ctrl_adsr(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_update(&vs->wgb.adsr, ps->a, ps->d, ps->s, ps->r);
}

static void ctrl_brightness(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_brightness(&vs->wgb, ps->brightness);
}

static void ctrl_mode_mix_amt(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_mode_mix_amt(&vs->wgb, ps->mode_mix_amt);
}

static void ctrl_harm_coef(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_harmonic_mod(&vs->wgb, ps->h_coef);
}


//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p10 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	memset(vs, 0, sizeof(struct v_state));

	adsr_init(&vs->wgb.adsr, ps->a, ps->d, ps->s, ps->r);
	wgb_init(&vs->wgb);
	pan_init(&vs->pan);

	ctrl_brightness(v);
	ctrl_harm_coef(v);
	ctrl_frequency(v);
	ctrl_attenuate(v);
	ctrl_mode_mix_amt(v);
	ctrl_pan(v);
	

}

// stop the patch
static void stop(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	DBG("p10 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	adsr_idle(&vs->wgb.adsr);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	//DBG("p10 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	//struct p_state *ps = (struct p_state *)v->patch->state;
	gpio_set(IO_LED_AMBER);
	adsr_attack(&vs->wgb.adsr);
	wgb_set_velocity(&vs->wgb, (float)vel / 127.f);
	ctrl_brightness(v);
	ctrl_harm_coef(v);
	//wgb_ctrl_brightness(&vs->wgb, ps->brightness);
	ctrl_frequency(v);
	wgb_pluck(&vs->wgb);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->wgb.adsr);
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
	wgb_gen(&vs->wgb, out, n);
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 1.f;
	ps->pan = 0.5f;
	ps->bend = 0.f;
	ps->attenuate = 0.99f;
	ps->bp_res = 0.499f;
	ps->brightness = 1.f;
	ps->a = 0.0f;
	ps->d = 2.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
	ps->mode_mix_amt = 0.1f;
	ps->h_coef = 0.f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p10 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:		// volume
		ps->vol = midi_map(val, 0.f, 1.5f);
		update = 1;
		break;
	case 2:		// left/right pan
		ps->pan = midi_map(val, 0.f, 1.f);
		update = 1;
		break;
	case 5:
		ps->attenuate = midi_map(val, 0.87f, 1.f);
		update = 2;
		break;
	case 71: // exciter velocity
		ps->brightness = midi_map(val, 0.05f, 1.0f);
		update = 5;
		break;
	case 74: // harmonic mod
		ps->h_coef = midi_map(val, -1.0f, 1.0f);
		update = 4;
		break;
	case 93: // mode mixing
		ps->mode_mix_amt = midi_map(val, 0.0f, 1.0f);
		update = 3;
		break;
	case 97:
		goto_next_patch(p);
		break;
		case 73:
		ps->a = midi_map(val, 0.0f, 5.f);
		update = 7;
		break;
	case 75:
		ps->d = midi_map(val, 0.02f, 4.f);
		update = 7;
		break;
	case 72:
		ps->s = midi_map(val, 0.0f, 1.f);
		update = 7;
		break;
	case 10:
		ps->r = midi_map(val, 0.0f, 4.f);
		update = 7;
		break;
	default:
		break;
	}
	if (update == 1) {
		update_voices(p, ctrl_pan);
	}
	if (update == 2) {
		update_voices(p, ctrl_attenuate);
	}
	if (update == 3) {
		update_voices(p, ctrl_mode_mix_amt);
	}
	if (update == 4) {
		update_voices(p, ctrl_harm_coef);
	}
	if (update == 5) {
		update_voices(p, ctrl_brightness);
	}
	if (update == 7) {
		update_voices(p, ctrl_adsr);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p2 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	update_voices(p, ctrl_frequency);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch10 = {
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
