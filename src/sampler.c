#include "sampler.h"

static int bits_from_sf_format(int format) {
    int bits = 16;

    switch (format & SF_FORMAT_SUBMASK) {
        case SF_FORMAT_PCM_16:
            bits = 16;
            break;
        case SF_FORMAT_PCM_24:
            bits = 24;
            break;
        case SF_FORMAT_PCM_32:
            bits = 32;
            break;
        case SF_FORMAT_PCM_S8:
            bits = 8;
            break;
        case SF_FORMAT_PCM_U8:
            bits = 8;
            break;
        default:
            bits = 16;
            break;
    } 

    return bits;
}

struct thread_args {
    Sampler *sampler;
    float volume;
    float speed;
};

Sampler* Sampler_init(char filename[]) {
    Sampler *sampler = malloc(sizeof(Sampler));

    if (filename != NULL) {
        SF_INFO file_info;
        SNDFILE *file = sf_open(filename, SFM_READ, &file_info); 
        sampler->buffer_size = file_info.channels * file_info.frames * sizeof(short);
        sampler->buffer = malloc(sampler->buffer_size * sizeof(short));
        sf_read_short(file, sampler->buffer, sampler->buffer_size);
        sf_close(file);

        memset(&sampler->format, 0, sizeof(sampler->format));
        sampler->format.bits = bits_from_sf_format(file_info.format);
        sampler->format.channels = file_info.channels;
        sampler->format.rate = file_info.samplerate;
        sampler->format.byte_format = AO_FMT_LITTLE;
    }

    return sampler;
}

static void play(struct thread_args *args) {
    Sampler *sampler = args->sampler;
    float volume = args->volume;
    float speed = args->speed;
    free(args);

    ao_sample_format format = sampler->format;
    format.rate = sampler->format.rate * speed; // Set speed

    short *buffer = malloc(sampler->buffer_size * sizeof(short));
    for (long i = 0; i < sampler->buffer_size; ++i) {
        buffer[i] = sampler->buffer[i] * volume; // Set volume
    }

    ao_device *device = ao_open_live(ao_default_driver_id(), &format, NULL);
    ao_play(device, (char *)buffer, sampler->buffer_size);
    ao_close(device);

    free(buffer);
}

void Sampler_play(Sampler *sampler, float volume, float speed) {
    if (sampler != NULL) {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->sampler = sampler;
        args->volume = volume;
        args->speed = speed;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, (void *)play, args);
        pthread_detach(thread_id);
    }
}

void Sampler_destroy(Sampler *sampler) {
    free(sampler->buffer);
    free(sampler);
}
