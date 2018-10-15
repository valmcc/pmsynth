//-----------------------------------------------------------------------------
/*

Patch 2

Karplus Strong Testing

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
	struct ks ks;
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
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	ks_ctrl_frequency(&vs->ks, midi_to_frequency((float)v->note - ps->bend));
}

static void ctrl_attenuate(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	ks_ctrl_attenuate(&vs->ks, ps->attenuate);
}

static void ctrl_pan(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	pan_ctrl(&vs->pan, ps->vol, ps->pan);
}

static void ctrl_adsr(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_update(&vs->ks.adsr, ps->a, ps->d, ps->s, ps->r);
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p2 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));
	struct p_state *ps = (struct p_state *)v->patch->state;

	adsr_init(&vs->ks.adsr, ps->a, ps->d, ps->s, ps->r);
	ks_init(&vs->ks);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_attenuate(v);
	ctrl_pan(v);

}

// stop the patch
static void stop(struct voice *v) {
	DBG("p2 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_idle(&vs->ks.adsr);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	//DBG("p2 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	gpio_set(IO_LED_AMBER);
	adsr_attack(&vs->ks.adsr);
	ks_pluck(&vs->ks);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	gpio_clr(IO_LED_AMBER);
	adsr_release(&vs->ks.adsr);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	return 1;
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	struct v_state *vs = (struct v_state *)v->state;
	float out[n];
	ks_gen(&vs->ks, out, n);
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 1.f;
	ps->pan = 0.5f;
	ps->bend = 0.f;
	ps->attenuate = 0.995f;
	ps->a = 0.0f;
	ps->d = 1.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p2 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case VOLUME_SLIDER:		// volume
		ps->vol = midi_map(val, 0.f, 1.0f);
		update = 1;
		break;
	case MODWHEEL:		// filter cutoff
		svf2_ctrl_cutoff(&p->pmsynth->opf, logmap(midi_map(val, 1.0f, 0.0f)));
		break;
	case KNOB_1: 		// filter resonance
		svf2_ctrl_resonance(&p->pmsynth->opf, midi_map(val, 0.f, 0.98f));
		break;
	case KNOB_2:
		ps->attenuate = midi_map(val, 0.87f, 1.f);
		update = 2;
		break;
	case 97:
		goto_next_patch(p);
		break;
	case KNOB_5:
		ps->a = midi_map(val, 0.0f, 0.5f);
		update = 7;
		break;
	case KNOB_6:
		ps->d = midi_map(val, 0.02f, 1.f);
		update = 7;
		break;
	case KNOB_7:
		ps->s = midi_map(val, 0.0f, 1.f);
		update = 7;
		break;
	case KNOB_8:
		ps->r = midi_map(val, 0.0f, 1.f);
		update = 7;
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
	if (update == 2) {
		update_voices(p, ctrl_attenuate);
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

const struct patch_ops patch2 = {
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
