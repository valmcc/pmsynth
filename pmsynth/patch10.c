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
	float reflection_adjust;
	int impulse_type;
	int impulse_solo;
	int resonator_type;
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

static void ctrl_reflection(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_reflection_adjust(&vs->wgb, ps->reflection_adjust);
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

static void ctrl_impulse_solo(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_impulse_solo(&vs->wgb, ps->impulse_solo);
}

static void ctrl_impulse_type(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_impulse_type(&vs->wgb, ps->impulse_type);
}

static void ctrl_brightness(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_brightness(&vs->wgb, ps->brightness);
}

static void ctrl_resonator_type(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wgb_ctrl_resonator_type(&vs->wgb, ps->resonator_type);
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
	ctrl_reflection(v);
	ctrl_impulse_solo(v);
	ctrl_impulse_type(v);
	

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
	ctrl_impulse_type(v);
	ctrl_impulse_solo(v);
	ctrl_harm_coef(v);
	ctrl_frequency(v);
	ctrl_reflection(v);
	ctrl_resonator_type(v);
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
	block_copy(out_l, out, n);
	block_mul_k(out_l, vs->pan.vol_l, n);
	//block_copy(out_r, out, n);
	//pan_gen(&vs->pan, out_l, out_r, out, n);
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
	ps->brightness = 1.0f;
	ps->a = 0.0f;
	ps->d = 2.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
	ps->mode_mix_amt = 0.1f;
	ps->h_coef = 0.f;
	ps->resonator_type = 2;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p10 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case VOLUME_SLIDER:		// volume
		ps->vol = midi_map(val, 0.f, 1.0f);
		update = 1;
		break;
	case MODWHEEL:		// filter cutoff
		svf2_ctrl_cutoff(&p->pmsynth->opf, logmap(midi_map(val, 0.f, 1.0f)));
		break;
	case KNOB_1: 		// filter resonance
		svf2_ctrl_resonance(&p->pmsynth->opf, midi_map(val, 0.f, 0.98f));
		break;
	case KNOB_2: // reflection adjust
		//ps->reflection_adjust = midi_map(val, 0.0f, 0.00001f);
		ps->brightness = midi_map(val,0.0f, 1.0f);
		update = 2;
		break;
	case KNOB_3: // harmonic mod
		ps->h_coef = midi_map(val, -1.0f, 1.0f);
		update = 4;
		break;
	case KNOB_4: // mode mixing
		ps->mode_mix_amt = midi_map(val, 0.0f, 1.0f);
		update = 3;
		break;
	case KNOB_5:
		ps->a = midi_map(val, 0.0f, 5.f);
		update = 7;
		break;
	case KNOB_6:
		ps->d = midi_map(val, 0.02f, 4.f);
		update = 7;
		break;
	case KNOB_7:
		ps->s = midi_map(val, 0.0f, 1.f);
		update = 7;
		break;
	case KNOB_8:
		ps->r = midi_map(val, 0.0f, 4.f);
		update = 7;
		break;
	case BUTTON_1:
		ps->resonator_type = 0;
		goto_next_patch(p);
		break;
	case BUTTON_2: //change resonator model 
		ps->resonator_type += 1;
		if (ps->resonator_type > 5){
			ps->resonator_type = 2;
		}
		current_resonator_type = ps->resonator_type;
		update_resonator();
		update = 6;
		break;
	case BUTTON_3: //goto next impulse sample
		ps->impulse_type += 1;
		if (ps->impulse_type > NUM_IMPULSES){
			ps->impulse_type = 0;
		}
		//update_impulse(ps->impulse_type);//TODO make this function!
		update = 8;
		break;
	case BUTTON_4: //goto previous impulse sample
		ps->impulse_type -= 1;
		if (ps->impulse_type < 0){
			ps->impulse_type = NUM_IMPULSES;
		}
		//update_impulse(ps->impulse_type);//TODO make this function!
		update = 8;
		break;
	case BUTTON_5: //solo impulse sample (toggle)
		ps->impulse_solo += 1;
		if (ps->impulse_solo > 1){
			ps->impulse_solo = 0;
		}
		update = 5;
		break;
	case BUTTON_6: //play demo song
		//TODO add demo song functionality
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
	if (update == 2) {
		update_voices(p, ctrl_reflection);
	}
	if (update == 3) {
		update_voices(p, ctrl_mode_mix_amt);
	}
	if (update == 4) {
		update_voices(p, ctrl_harm_coef);
	}
	if (update == 6) {
		update_voices(p, ctrl_resonator_type);
	}
	if (update == 7) {
		update_voices(p, ctrl_adsr);
	}
	if (update == 8) {
		update_voices(p, ctrl_brightness);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p10 pitch %d\r\n", val);
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
