//-----------------------------------------------------------------------------
/*

Physical Modelling Synthesizer

*/
//-----------------------------------------------------------------------------

#ifndef PMSYNTH_H
#define PMSYNTH_H

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "audio.h"
#include "io.h"

//-----------------------------------------------------------------------------

#define AUDIO_TS (1.f/AUDIO_FS)

#define PI (3.1415927f)
#define TAU (2.f * PI)
#define INV_TAU (1.f/TAU)

//-----------------------------------------------------------------------------
// global variables

// number of simultaneous voices
#define NUM_VOICES 16 // Max number of voices (polyphony)

int current_patch_no; // what midi channel patch is currently playing
int current_exciter_type; // for screen
int current_resonator_type;
int global_polyphony;


//-----------------------------------------------------------------------------
// benchmarks

void pow_benchmark(void);
void block_benchmark(void);

//-----------------------------------------------------------------------------
// block operations

void block_mul(float *out, float *buf, size_t n);
void block_mul_k(float *out, float k, size_t n);
void block_add(float *out, float *buf, size_t n);
void block_add_k(float *out, float k, size_t n);
void block_copy(float *dst, const float *src, size_t n);
void block_copy_mul_k(float *dst, const float *src, float k, size_t n);

//-----------------------------------------------------------------------------
// power functions

float pow2_int(int x);
float pow2_frac(float x);
float pow2(float x);
float powe(float x);

//-----------------------------------------------------------------------------
// Panning

struct pan {
	float vol_l;		// stereo left volume
	float vol_r;		// stereo right volume
};

void pan_init(struct pan *p);
void pan_ctrl(struct pan *p, float vol, float pan);
void pan_gen(struct pan *p, float *out_l, float *out_r, const float *in, size_t n);

//-----------------------------------------------------------------------------
// noise

struct noise {
	float b0, b1, b2, b3, b4, b5, b6;
	//float max;
	//uint32_t count;
};

void noise_init(struct noise *ns);
void noise_gen_white(struct noise *ns, float *out, size_t n);
void noise_gen_pink1(struct noise *ns, float *out, size_t n);
void noise_gen_pink2(struct noise *ns, float *out, size_t n);
void noise_gen_brown(struct noise *ns, float *out, size_t n);

//-----------------------------------------------------------------------------
// sine wave oscillators

// Sin Oscillator
struct sin {
	float freq;		// base frequency
	uint32_t x;		// current x-value
	uint32_t xstep;		// current x-step
};

float sin_eval(float x);
float cos_eval(float x);
float tan_eval(float x);

void sin_init(struct sin *osc);
void sin_ctrl_frequency(struct sin *osc, float freq);
void sin_gen(struct sin *osc, float *out, float *fm, size_t n);

// Goom Waves
struct gwave {
	float freq;		// base frequency
	uint32_t tp;		// s0f0 to s1f1 transition point
	float k0;		// scaling factor for slope 0
	float k1;		// scaling factor for slope 1
	uint32_t x;		// phase position
	uint32_t xstep;		// phase step per sample
};

void gwave_init(struct gwave *osc);
void gwave_ctrl_frequency(struct gwave *osc, float freq);
void gwave_ctrl_shape(struct gwave *osc, float duty, float slope);
void gwave_gen(struct gwave *osc, float *out, float *fm, size_t n);

//-----------------------------------------------------------------------------
// ADSR envelope

struct adsr {
	float s;		// sustain level
	float ka;		// attack constant
	float kd;		// decay constant
	float kr;		// release constant
	float d_trigger;	// attack->decay trigger level
	float s_trigger;	// decay->sustain trigger level
	float i_trigger;	// release->idle trigger level
	int state;		// envelope state
	float val;		// output value
};

// generators
void adsr_gen(struct adsr *e, float *out, size_t n);

// envelopes
void adsr_init(struct adsr *e, float a, float d, float s, float r);
void ad_init(struct adsr *e, float a, float d);

// modulation
void adsr_update(struct adsr *e, float a, float d, float s, float r);

// actions
void adsr_attack(struct adsr *e);
void adsr_release(struct adsr *e);
void adsr_idle(struct adsr *e);

// state
int adsr_is_active(struct adsr *e);

//-----------------------------------------------------------------------------
// Karplus Strong

#define KS_DELAY_BITS (6U)
#define KS_DELAY_SIZE (1U << KS_DELAY_BITS)

struct ks {
	float freq;		// base frequency
	float delay[KS_DELAY_SIZE];
	float k;		// attenuation and averaging constant 0 to 0.5
	uint32_t x;		// phase position
	uint32_t xstep;		// phase step per sample
};

void ks_init(struct ks *osc);
void ks_ctrl_frequency(struct ks *osc, float freq);
void ks_ctrl_attenuate(struct ks *osc, float attenuate);
void ks_pluck(struct ks *osc);
void ks_gen(struct ks *osc, float *out, size_t n);
//-----------------------------------------------------------------------------
// Waveguide synth

#define WG_DELAY_BITS (8U) //increased from 7
#define WG_DELAY_SIZE (1U << WG_DELAY_BITS) // this is the max delay size

struct wg {
	float freq;		// base frequency
	float delay_l[WG_DELAY_SIZE];		// left travelling wave
	float delay_r[WG_DELAY_SIZE];		//right travelling wave
	float r;		// reflection constant
	uint32_t epos; // excitation sample position (in wavetable)
	int estate; // excitement state (1 = excited, 0 = not)
	uint16_t einc; //how much to increment exciter sample pointer
	uint16_t ephase; //phase for increment exciter sample pointer
	uint32_t delay_len; // length of delay line
	float delay_len_frac; // extra fractional delay length
	float delay_len_total; // total fractional delay length
	float a; // all-pass filter coefficient
	uint32_t excite_pos;// excitement location
	float excite_loc; // percentage location of exciter
	uint32_t x_pos_l; // pointers for delay line
	uint32_t x_pos_r; // pointers for delay line
	uint32_t bridge_pos; // edge condition locations
	uint32_t nut_pos; // edge condition locations
	uint32_t pickup_pos; // pickup location
	uint32_t x_pos_l_2; // pointers to delay line for linear interpolation
	uint32_t x_pos_r_2; // pointers to delay line for linear interpolation
	float ap_state_1; // all pass filter previous values
	float ap_state_2; // all pass filter previous values
	float ap_stiff; // all pass filter "stiffness"
	float velocity;
	uint32_t downsample_amt; // downsampling by halving the length of the delay line
	float lp_coef_a; //not implemented - for breath control
	float lp_coef_b;
	int tube; //positive or negative reflection?

};

void wg_init(struct wg *osc);
void wg_ctrl_frequency(struct wg *osc, float freq);
void wg_ctrl_reflection(struct wg *osc, float reflection);
void wg_ctrl_stiffness(struct wg *osc, float stiffness);
void wg_ctrl_pos(struct wg *osc, float excite_loc);
void wg_ctrl_brightness(struct wg *osc, float brightness);
void wg_excite(struct wg *osc);
void wg_set_velocity(struct wg *osc, float velocity);
void wg_gen(struct wg *osc, float *out, size_t n);
void wg_set_samplerate(struct wg *osc, float downsample_amt);
void wg_exciter_type(struct wg *osc, int exciter_type);
//
//-----------------------------------------------------------------------------
//exciter

float mallet_lookup(int16_t x);
float mallet_gen(struct wg *osc);

//-----------------------------------------------------------------------------
// Woodwind synth

#define WW_DELAY_BITS (8U)
#define WW_DELAY_SIZE (1U << WW_DELAY_BITS)

struct ww {
	float freq;		// base frequency
	float flute_out_old;
	float dl_1_out; // output value from delay line 1
	float dl_1[WW_DELAY_SIZE];
	float dl_2[WW_DELAY_SIZE];
	float dl_1_len_total;
	float dl_2_len_total;
	float dl_1_len_frac;
	float dl_2_len_frac;
	uint32_t dl_1_len;
	uint32_t dl_2_len;
	uint32_t dl_1_ptr_in;
	uint32_t dl_2_ptr_in;
	uint32_t dl_1_ptr_out;
	uint32_t dl_2_ptr_out;
	int estate; // excitation state
	struct adsr adsr;
	struct noise ns;
	struct sin vibrato; 
	float dc_filt_in;
	float dc_filt_out;
	uint32_t downsample_amt; // downsampling by halving the length of the delay line
	float r_1; // reflection coefs
	float r_2;
	float lp_filter_coef;

};

void ww_init(struct ww *osc);
void ww_set_samplerate(struct ww *osc, float downsample_amt);
void ww_ctrl_frequency(struct ww *osc, float freq);
void ww_ctrl_attenuate(struct ww *osc, float attenuate);
void ww_update_coefficients(struct ww *osc, float lp_filter_coef, float r_1, float r_2);
void ww_blow(struct ww *osc);
void ww_gen(struct ww *osc, float *out, size_t n);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 2D waveguide

#define MESH_LENGTH 8 
#define MESH_WIDTH 8

struct junction {
	float vJ; // junction velocity
	float vN; // velocity going north into junction
	float vS; // velocity going south into junction
	float vE; // velocity going east into junction
	float vW; // velocity going west into junction
	float vN1; // temp variables
	float vS1;
	float vE1;
	float vW1;
};

struct wg_2d {
	float freq;		// base frequency
	float delay[KS_DELAY_SIZE];
	float k;		// attenuation and averaging constant 0 to 0.5
	uint32_t x;		// phase position
	uint32_t xstep;		// phase step per sample
	struct junction mesh[MESH_LENGTH][MESH_WIDTH];

	int estate; // excitement state (1 = excited, 0 = not)
	uint32_t epos; // excitation sample position (in wavetable)
	uint16_t einc; //how much to increment exciter sample pointer
	uint16_t ephase; //phase for increment exciter sample pointer
};

void wg_2d_init(struct wg_2d *osc);
void wg_2d_ctrl_frequency(struct wg_2d *osc, float freq);
void wg_2d_ctrl_attenuate(struct wg_2d *osc, float attenuate);
void wg_2d_pluck(struct wg_2d *osc);
void wg_2d_gen(struct wg_2d *osc, float *out, size_t n);
// exciter
float mallet_gen_2d(struct wg_2d *osc);


//-----------------------------------------------------------------------------
// Low Pass Filters

struct svf {
	float kf;		// constant for cutoff frequency
	float kq;		// constant for filter resonance
	float bp;		// bandpass state variable
	float lp;		// low pass state variable
};

void svf_ctrl_cutoff(struct svf *f, float cutoff);
void svf_ctrl_resonance(struct svf *f, float resonance);
void svf_init(struct svf *f);
void svf_gen(struct svf *f, float *out, const float *in, size_t n);

struct svf2 {
	float ic1eq, ic2eq;	// state variables
	float g;		// constant for cutoff frequency
	float k;		// constant for filter resonance
};

void svf2_ctrl_cutoff(struct svf2 *f, float cutoff);
void svf2_ctrl_resonance(struct svf2 *f, float resonance);
void svf2_init(struct svf2 *f);
void svf2_gen(struct svf2 *f, float *out, const float *in, size_t n);

//-----------------------------------------------------------------------------
// Note Sequencer

// per sequence state machine
struct seq_sm {
	uint8_t *prog;		// program memory
	int pc;			// program counter
	int s_state;		// sequencer state
	int op_state;		// operation state
	int duration;		// operation duration
};

// sequencer
struct seq {
	struct pmsynth *pmsynth;	// pointer back to the parent pmsynth state
	float beats_per_min;
	float secs_per_tick;
	float tick_error;
	uint32_t ticks;
	struct seq_sm m0;
};

int seq_init(struct seq *s);
void seq_exec(struct seq *s);

//-----------------------------------------------------------------------------
// midi

// midi message receiver
struct midi_rx {
	struct pmsynth *pmsynth;	// pointer back to the parent pmsynth state
	void (*func) (struct midi_rx * midi);	// event function
	int state;		// rx state
	uint8_t status;		// message status byte
	uint8_t arg0;		// message byte 0
	uint8_t arg1;		// message byte 1
};

void midi_rx_serial(struct midi_rx *midi, struct usart_drv *serial);
float midi_map(uint8_t val, float a, float b);
float midi_to_frequency(float note);
float midi_pitch_bend(uint16_t val);

//-----------------------------------------------------------------------------
// events

// event type in the upper 8 bits
#define EVENT_TYPE(x) ((x) & 0xff000000U)
#define EVENT_TYPE_KEY_DN (1U << 24)
#define EVENT_TYPE_KEY_UP (2U << 24)
#define EVENT_TYPE_MIDI (3U << 24)
#define EVENT_TYPE_AUDIO (4U << 24)

// key number in the lower 8 bits
#define EVENT_KEY(x) ((x) & 0xffU)
// midi message in the lower 3 bytes
#define EVENT_MIDI(x) ((x) & 0xffffffU)
// audio block size in the lower 16 bits
#define EVENT_BLOCK_SIZE(x) ((x) & 0xffffU)

struct event {
	uint32_t type;		// the event type
	void *ptr;		// pointer to event data (or data itself)
};

int event_init(void);
int event_rd(struct event *event);
int event_wr(uint32_t type, void *ptr);

//-----------------------------------------------------------------------------
// voices

//#define VOICE_STATE_SIZE 1024
#define VOICE_STATE_SIZE 4096
// had to make this larger 

struct voice {
	int idx;		// index in table
	uint8_t note;		// current note
	uint8_t channel;	// current channel
	struct patch *patch;	// patch in use
	uint8_t state[VOICE_STATE_SIZE];	// per voice state
};

struct voice *voice_lookup(struct pmsynth *s, uint8_t channel, uint8_t note);
struct voice *voice_alloc(struct pmsynth *s, uint8_t channel, uint8_t note);
void stop_voices(struct patch *p);
void update_voices(struct patch *p, void (*func) (struct voice *));

//-----------------------------------------------------------------------------
// patches

// patch operations
struct patch_ops {
	// voice functions
	void (*start) (struct voice * v);	// start a voice
	void (*stop) (struct voice * v);	// stop a voice
	void (*note_on) (struct voice * v, uint8_t vel);
	void (*note_off) (struct voice * v, uint8_t vel);
	int (*active) (struct voice * v);	// is the voice active
	void (*generate) (struct voice * v, float *out_l, float *out_r, size_t n);	// generate samples
	// patch functions
	void (*init) (struct patch * p);
	void (*control_change) (struct patch * p, uint8_t ctrl, uint8_t val);
	void (*pitch_wheel) (struct patch * p, uint16_t val);
};

#define PATCH_STATE_SIZE 128

struct patch {
	struct pmsynth *pmsynth;	// pointer back to the parent pmsynth state
	const struct patch_ops *ops;
	uint8_t state[PATCH_STATE_SIZE];	// per patch state
};

// implemented patches
extern const struct patch_ops patch0;
extern const struct patch_ops patch1;
extern const struct patch_ops patch2;
extern const struct patch_ops patch3;
extern const struct patch_ops patch4;
extern const struct patch_ops patch5;
extern const struct patch_ops patch6;
extern const struct patch_ops patch7;
extern const struct patch_ops patch8;
extern const struct patch_ops patch9;

//-----------------------------------------------------------------------------


// number of concurrent channels
#define NUM_CHANNELS 16

struct pmsynth {
	struct audio_drv *audio;	// audio output
	struct usart_drv *serial;	// serial port for midi interface
	struct midi_rx midi_rx0;	// midi rx from the serial port
	struct seq seq0;	// note sequencer
	struct patch patches[NUM_CHANNELS];	// current patch set
	struct voice voices[NUM_VOICES];	// voices
	int voice_idx;		// FIXME round robin voice allocation
};

int pmsynth_init(struct pmsynth *s, struct audio_drv *audio, struct usart_drv *midi);
int pmsynth_run(struct pmsynth *s);

//-----------------------------------------------------------------------------
// Handler functions

void goto_next_patch(struct patch *p);
void update_resonator();
void update_patch();
void update_polyphony();
void update_exciter();

//-----------------------------------------------------------------------------

#endif				// PMSYNTH_H

//-----------------------------------------------------------------------------
