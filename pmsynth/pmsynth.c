//-----------------------------------------------------------------------------
/*

Physical Modelling Synthesizer

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "pmsynth.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// voice operations

// lookup the voice being used for this channel and note.
struct voice *voice_lookup(struct pmsynth *s, uint8_t channel, uint8_t note) {
	for (unsigned int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		if (v->note == note && v->channel == channel) {
			return v;
		}
	}
	return NULL;
}

// allocate a new voice, possibly reusing a current active voice.
struct voice *voice_alloc(struct pmsynth *s, uint8_t channel, uint8_t note) {
	// validate the channel
	if (channel >= NUM_CHANNELS || s->patches[channel].ops == NULL) {
		DBG("no patch defined for channel %d\r\n", channel);
		return NULL;
	}
	// TODO: Currently doing simple round robin allocation.
	// More intelligent voice allocation to follow....
	struct voice *v = &s->voices[s->voice_idx];
	s->voice_idx += 1;
	//if (s->voice_idx == NUM_VOICES) {
	if (s->voice_idx >= global_polyphony) {
		s->voice_idx = 0;
	}
	// stop an existing patch on this voice
	if (v->patch) {
		v->patch->ops->stop(v);
	}
	// setup the new voice
	v->note = note;
	v->channel = channel;
	v->patch = &s->patches[channel];
	v->patch->ops->start(v);
	return v;

}

// stops all voices
void stop_voices(struct patch *p) {
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &p->pmsynth->voices[i];
		if (v->patch == p) {
			v->patch->ops->stop(v);
		}

		voice_alloc(p->pmsynth, 5, 42); // overwrites all voices with a dummy voice (channel 5)
	}
}


// run an update function for each voice using the patch
void update_voices(struct patch *p, void (*func) (struct voice *)) {
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &p->pmsynth->voices[i];
		if (v->patch == p) {
			func(v);
		}
	}
}

//-----------------------------------------------------------------------------
// key events

// handle a key down event
static void key_dn_handler(struct pmsynth *s, struct event *e) {
	DBG("key down %d\r\n", EVENT_KEY(e->type));
	gpio_set(IO_LED_AMBER);
}

// handle a key up event
static void key_up_handler(struct pmsynth *s, struct event *e) {
	DBG("key up %d\r\n", EVENT_KEY(e->type));
	gpio_clr(IO_LED_AMBER);
}

//-----------------------------------------------------------------------------
// midi events

// Currently unused.
// Serial MIDI events are handled as they are read from the buffer.
// MIDI events from USB might use this....

// handle a midi event
static void midi_handler(struct pmsynth *s, struct event *e) {
	DBG("midi %06x\r\n", EVENT_MIDI(e->type));
}

//-----------------------------------------------------------------------------
// audio request events

// handle an audio request event
static void audio_handler(struct pmsynth *s, struct event *e) {
	size_t n = EVENT_BLOCK_SIZE(e->type);
	int16_t *dst = e->ptr;

	//DBG("audio %08x %08x\r\n", e->type, e->ptr);

	// clear the output buffers
	float out_l[n], out_r[n];
	memset(out_l, 0, n * sizeof(float));
	memset(out_r, 0, n * sizeof(float));

	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		struct patch *p = v->patch;
		if (p && p->ops->active(v)) {
			// generate left/right samples
			float buf_l[n], buf_r[n];
			p->ops->generate(v, buf_l, buf_r, n);
			// accumulate in the output buffers
			block_add(out_l, buf_l, n);
			block_add(out_r, buf_r, n);
		}
	}

	// write the samples to the dma buffer
	audio_wr(dst, n, out_l, out_r);
	// record some realtime stats
	audio_stats(s->audio, dst);
}

//-----------------------------------------------------------------------------

// the main pmsynth event loop
int pmsynth_run(struct pmsynth *s) {
	while (1) {
		struct event e;
		if (!event_rd(&e)) {
			switch (EVENT_TYPE(e.type)) {
			case EVENT_TYPE_KEY_DN:
				key_dn_handler(s, &e);
				break;
			case EVENT_TYPE_KEY_UP:
				key_up_handler(s, &e);
				break;
			case EVENT_TYPE_MIDI:
				midi_handler(s, &e);
				break;
			case EVENT_TYPE_AUDIO:
				seq_exec(&s->seq0);
				audio_handler(s, &e);
				break;
			default:
				DBG("unknown event %08x %08x\r\n", e.type, e.ptr);
				break;
			}
		}
		// get and process serial midi messages
		midi_rx_serial(&s->midi_rx0, s->serial);
	}
	return 0;
}

//-----------------------------------------------------------------------------

// initialise the pmsynth state
int pmsynth_init(struct pmsynth *s, struct audio_drv *audio, struct usart_drv *serial) {
	int rc = 0;

	memset(s, 0, sizeof(struct pmsynth));
	s->audio = audio;
	s->serial = serial;

	// setup the midi receivers.
	s->midi_rx0.pmsynth = s;

	rc = event_init();
	if (rc != 0) {
		DBG("event_init failed %d\r\n", rc);
		goto exit;
	}

	// setting polyphony
	update_polyphony();


	// setup the patch operations
	//s->patches[0].ops = &patch10;
	s->patches[0].ops = &patch7;
	s->patches[1].ops = &patch2;
	s->patches[2].ops = &patch9;
	//s->patches[2].ops = &patch1;
	//s->patches[3].ops = &patch3;
	//s->patches[4].ops = &patch5;
	//s->patches[5].ops = &patch6;
	//s->patches[6].ops = &patch4;
	//s->patches[7].ops = &patch8;
	//s->patches[8].ops = &patch9;


	// setup the patch on each channel
	for (int i = 0; i < NUM_CHANNELS; i++) {
		struct patch *p = &s->patches[i];
		// call init for each patch
		if (p->ops) {
			p->pmsynth = s;
			memset(p->state, 0, PATCH_STATE_SIZE);
			p->ops->init(p);
		}
	}

	// setup the voices
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		v->idx = i;
		v->channel = 255;
		v->note = 255;
	}
	update_patch();
	update_exciter();
	update_resonator();
	// setup the sequencer
	//rc = seq_init(&s->seq0);
	//if (rc != 0) {
	// 	DBG("seq_init failed %d\r\n", rc);
	// 	goto exit;
	// }
	// s->seq0.pmsynth = s;

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
