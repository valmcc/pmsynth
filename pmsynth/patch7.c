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
	int impulse_type;
	int impulse_solo;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	//if (v->note < 57){
	if (v->note < 48){
		wg_set_samplerate(&vs->wg,2); // halve sample rate
	}
	if (v->note < 33){
		wg_set_samplerate(&vs->wg,4); // quarter sample rate
	}
	wg_ctrl_frequency(&vs->wg, midi_to_frequency((float)v->note - ps->bend));
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

static void ctrl_impulse_solo(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_impulse_solo(&vs->wg, ps->impulse_solo);
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

static void ctrl_impulse_type(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	wg_ctrl_impulse_type(&vs->wg, ps->impulse_type);
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
	ctrl_impulse_solo(v);
	ctrl_exciter_type(v);
	ctrl_impulse_type(v);
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
	//gpio_set(IO_ATTACK_LED);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	adsr_attack(&vs->wg.adsr);

	
	// notes below C2 can't be processed as they make the delay line too large
	if (v->note > 24){
	// reflection
	wg_ctrl_impulse_type(&vs->wg,ps->impulse_type);
	wg_ctrl_reflection(&vs->wg,ps->reflection);
	// velocity
	wg_set_velocity(&vs->wg, (float)vel / 127.f);
	// stiffness
	wg_ctrl_impulse_solo(&vs->wg,ps->impulse_solo);
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
	//gpio_clr(IO_ATTACK_LED);
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
	block_copy(out_l, out, n);
	block_mul_k(out_l, vs->pan.vol_l, n);
	//block_copy(out_r, out, n);
	//pan_gen(&vs->pan, out_l, out_r, out, n); // pan function is currently slow (no stereo used so not used)
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 1.0f;
	ps->pan = 0.5f;
	ps->bend = 0.0f;
	ps->reflection = -0.99f;
	ps->release = 0.0f;
	ps->stiffness = 1.0f;
	ps->exciter_type = 0;
	ps->exciter_loc = 0.25f;
	ps->brightness = 1.0f;
	ps->a = 0.0f;
	ps->d = 2.0f;
	ps->s = 1.0f;
	ps->r = 1.0f;
	ps->impulse_type = 0;
	ps->impulse_solo = 0;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p7 ctrl %d val %d\r\n", ctrl, val);

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
		ps->reflection = midi_map(val, -0.95f, -1.0f);
		update = 2;
		break;
	case KNOB_3:
		ps->stiffness = midi_map(val, 0.0f, 1.0f);
		update = 3;
		break;
	case KNOB_4:
		ps->exciter_loc = midi_map(val, 0.08f, 0.5f);
		update = 4;
		break;
	case KNOB_5:
		ps->a = midi_map(val, 0.0f, 0.5f);
		update = 7;
		break;
	case KNOB_6:
		ps->d = midi_map(val, 0.02f, 5.f);
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
		ps->exciter_type = 0;
		current_resonator_type = 3;
		goto_next_patch(p);
		break;
	case BUTTON_2: //change resonator model (in this case changes the refection to invert)
		ps->exciter_type += 1;
		if (ps->exciter_type > 1){
			ps->exciter_type = 0;
		}
		current_resonator_type = ps->exciter_type;
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
		update_voices(p, ctrl_stiffness);
	}
	if (update == 4) {
		update_voices(p, ctrl_exciter_loc);
	}
	if (update == 5) {
		update_voices(p, ctrl_impulse_solo);
	}
	if (update == 6) {
		update_voices(p, ctrl_exciter_type);
	}
	if (update == 7) {
		update_voices(p, ctrl_adsr);
	}
	if (update == 8) {
		update_voices(p, ctrl_impulse_type);
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
