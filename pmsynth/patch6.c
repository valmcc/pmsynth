//-----------------------------------------------------------------------------
/*

Patch 6

Dummy patch (Does nothing)

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "pmsynth.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	int algo;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");


//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p6 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// stop the patch
static void stop(struct voice *v) {
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	return 0;
	//do nothing
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	float out[n];

	// do nothing
	block_copy(out_l, out, n);
	block_copy(out_l, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	// nothing
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	// do nothing
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	// do nothing
}

//-----------------------------------------------------------------------------

const struct patch_ops patch6 = {
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
