//-----------------------------------------------------------------------------
/*

Patch 7

Waveguide synth

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct wg wg;
	struct pan pan;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
	float reflection; // reflection (1 = perfect, 0 = no reflection)
	float stiffness;
	float pickup_pos;
	float exciter_pos;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_frequency(&vs->wg, midi_to_frequency((float)v->note + ps->bend));
}

static void ctrl_reflection(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_reflection(&vs->wg, ps->reflection);
}

static void ctrl_stiffness(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_stiffness(&vs->wg, ps->stiffness);
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
	DBG("p7 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));

	wg_init(&vs->wg);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_reflection(v);
	ctrl_stiffness(v);
	ctrl_pan(v);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p7 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p7 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	gpio_set(IO_LED_AMBER); // flash the led when a note comes in

	// notes below C4 can't be processed as they make the delay line too large
	if (v->note > 47){
	struct v_state *vs = (struct v_state *)v->state;
	wg_excite(&vs->wg);
	
	} else{
		DBG("note too low to handle!");
	}
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
	wg_gen(&vs->wg, out, n);
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 1.0f;
	ps->pan = 0.5f;
	ps->bend = 0.0f;
	ps->reflection = -0.99f;
	ps->stiffness = 1.0f;
	//ps->pickup_pos = 1.0f;
	//ps->exciter_pos = 1.0f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p7 ctrl %d val %d\r\n", ctrl, val);

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
		ps->reflection = midi_map(val, -0.95f, -1.0f);
		update = 2;
		break;
	case 6:
		ps->stiffness = midi_map(val, 0.0f, 1.0f);
		update = 3;
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
		update_voices(p, ctrl_stiffness);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p7 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	update_voices(p, ctrl_frequency);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch7 = {
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