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
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	ks_ctrl_frequency(&vs->ks, midi_to_frequency((float)v->note + ps->bend));
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

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p2 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));

	ks_init(&vs->ks);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_attenuate(v);
	ctrl_pan(v);

}

// stop the patch
static void stop(struct voice *v) {
	DBG("p2 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	//DBG("p2 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	gpio_set(IO_LED_AMBER);
	ks_pluck(&vs->ks);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
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
	ps->attenuate = 0.99f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p2 ctrl %d val %d\r\n", ctrl, val);

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
	case 97:
		current_patch_no -= 1; // increment to next patch
		term_print(&pmsynth_display.term, "       1D Waveguide\n",2);
		term_print(&pmsynth_display.term, "       Struck String\n",4);
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
