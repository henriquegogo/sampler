#include <stdio.h>
#include <ao/ao.h>
#include "midi.h"
#include "sampler.h"

Sampler* samples[127]; 

void event_handler(snd_seq_event_t *event) {
    int note = event->data.note.note;
    float volume = 1.0;
    float speed = 1.0;
    Sampler_play(samples[note], volume, speed);
}

int main(int argc, char *argv[]) {
    ao_initialize();

    samples[60] = Sampler_init("samples/kick.wav");
    samples[64] = Sampler_init("samples/snare.wav");

    Midi *midi = Midi_init();
    Midi_listen(midi, event_handler);

    Midi_destroy(midi);
    
    for (int i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        if (samples[i] != NULL) Sampler_destroy(samples[i]);
    }

    ao_shutdown();

    return 0;
}
