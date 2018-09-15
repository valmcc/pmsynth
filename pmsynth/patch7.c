//-----------------------------------------------------------------------------
/*

Patch 7

1-D Waveguide synth (string)

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "pmsynth.h"
#include "display.h"
#include "lcd.h"


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
	float stiffness; // all pass filter stiffness (1 = not stiff, 0 = very)
	float pickup_loc; // TODO
	float exciter_loc; // loaction of pluck/strike on string
	float brightness;
	float release;
	int exciter_type;
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
	if (v->note < 57){
		wg_set_samplerate(&vs->wg,2); // halve sample rate
	}
	if (v->note < 33){
		wg_set_samplerate(&vs->wg,4); // quarter sample rate
	}
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

static void ctrl_exciter_loc(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_pos(&vs->wg, ps->exciter_loc);
}

static void ctrl_brightness(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_brightness(&vs->wg, ps->brightness);
}

static void ctrl_exciter_type(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_exciter_type(&vs->wg, ps->exciter_type);
}

static void ctrl_adsr(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_update(&vs->wg.adsr, ps->a, ps->d, ps->s, ps->r);
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p7 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	memset(vs, 0, sizeof(struct v_state));

	adsr_init(&vs->wg.adsr, ps->a, ps->d, ps->s, ps->r);
	wg_init(&vs->wg);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_reflection(v);
	ctrl_stiffness(v);
	ctrl_pan(v);
	ctrl_brightness(v);
	ctrl_exciter_type(v);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p7 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	wg_ctrl_reflection(&vs->wg,0.0f);
	adsr_idle(&vs->wg.adsr);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p7 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	gpio_set(IO_LED_AMBER); // flash the led when a note comes in
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_attack(&vs->wg.adsr);

	
	// notes below C2 can't be processed as they make the delay line too large
	if (v->note > 24){
	// reflection
	wg_ctrl_reflection(&vs->wg,ps->reflection);
	// velocity
	wg_set_velocity(&vs->wg, (float)vel / 127.f);
	// stiffness
	wg_ctrl_brightness(&vs->wg,ps->brightness);
	// position
	wg_ctrl_pos(&vs->wg, ps->exciter_loc);
	wg_excite(&vs->wg);
	
	} else{
		DBG("note too low to handle!");
	}
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	gpio_clr(IO_LED_AMBER);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_reflection(&vs->wg,ps->reflection);
	adsr_release(&vs->wg.adsr);

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
	ps->reflection = -1.0f;
	ps->release = 0.0f;
	ps->stiffness = 1.0f;
	ps->exciter_type = 0.0f;
	ps->exciter_loc = 0.25f;
	ps->brightness = 1.0f;
	ps->a = 0.0f;
	ps->d = 2.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p7 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	// case 75:		// volume
	// 	ps->vol = midi_map(val, 0.f, 1.5f);
	// 	update = 1;
	// 	break;
	// case 72:		// left/right pan
	// 	ps->pan = midi_map(val, 0.f, 1.f);
	// 	update = 1;
	// 	break;
	case 91:
		ps->reflection = midi_map(val, -0.95f, -1.0f);
		update = 2;
		break;
	case 93:
		ps->stiffness = midi_map(val, 0.0f, 1.0f);
		update = 3;
		break;
	case 74:
		ps->exciter_loc = midi_map(val, 0.08f, 0.5f);
		update = 4;
		break;
	case 71: // exciter velocity
		ps->brightness = midi_map(val, 0.05f, 1.0f);
		update = 5;
		break;
	// case 73:
	// 	ps->release = midi_map(val, 0.1f, 0.0f);
	// 	break;
	case 73:
		ps->a = midi_map(val, 0.0f, 5.f);
		update = 7;
		break;
	case 75:
		ps->d = midi_map(val, 0.02f, 5.f);
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
	case 96:
		ps->exciter_type += 1;
		if (ps->exciter_type > 1){
			ps->exciter_type = 0;
		}
		current_resonator_type = ps->exciter_type;
		update_resonator();
		update = 6;
		break;
	case 97:
		ps->exciter_type = 0.0f;
		goto_next_patch(p);
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
	if (update == 4) {
		update_voices(p, ctrl_exciter_loc);
	}
	if (update == 5) {
		update_voices(p, ctrl_brightness);
	}
	if (update == 6) {
		update_voices(p, ctrl_exciter_type);
	}
	if (update == 7) {
		update_voices(p, ctrl_adsr);
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
