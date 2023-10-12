/******************************************************************************
 *
 * stft implements a short time fourier transform filterbank.
 * 
 *****************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <memory.h>

#include "arm_math.h"

#include "stft.h"
#include "framer.h"

struct stft {
    stft_callback callback;
    void* user;
    size_t n_fft;
    size_t hop_length;
    float* frame;
    float* real_imag;
    float* window;
    struct framer* input_framer;
    struct framer* output_framer;
    arm_rfft_fast_instance_f32 rfft_instance;
};

struct stft* stft_create(stft_callback callback, void* user, size_t n_fft, size_t hop_length, size_t buffer_size) {
    struct stft* this = (struct stft*) calloc(1, sizeof(struct stft));
    this->callback = callback;
    this->user = user;
    this->n_fft = n_fft;
    this->hop_length = hop_length;
    
    // initialize the transform
    arm_status status = arm_rfft_fast_init_f32(&this->rfft_instance, n_fft);
    assert(status == ARM_MATH_SUCCESS);

    // initialize the window with a sqrt(Hann)
    this->window = (float*) calloc(n_fft, sizeof(float));
    arm_hanning_f32(this->window, n_fft);
    for (int i = 0; i < n_fft; ++i) {
        this->window[i] = (float)sqrt((double)this->window[i]);
    }

    // scale the window according to the hop length
    float* ola_win_scale = (float*) calloc(n_fft, sizeof(float));
    for (size_t h = 0; h < n_fft; h += hop_length) {
        for (size_t i = 0; i < n_fft; i++) {
            ola_win_scale[i] += this->window[(i+h)%n_fft] * this->window[(i+h)%n_fft];
        }
    }
    for (size_t i = 0; i < n_fft; i++) {
        this->window[i] /= (float)sqrt((double)ola_win_scale[i]);
    }
    free(ola_win_scale);

    // initialize the I/O framers and add n_fft of zeros
    this->input_framer = framer_create(buffer_size);
    this->output_framer = framer_create(buffer_size);
    framer_write_zeros(this->input_framer, n_fft);
    framer_write_zeros(this->output_framer, n_fft);

    // working vectors
    this->frame = (float*) calloc(n_fft, sizeof(float));
    this->real_imag = (float*) calloc(n_fft, sizeof(float));

    return this;
}

void stft_destroy(struct stft* this) {
    free(this->frame);
    free(this->real_imag);
    free(this->window);
    framer_destroy(this->input_framer);
    framer_destroy(this->output_framer);
    free(this);
}

size_t stft_process(struct stft* this, const float* in, float* out, size_t n) {
    framer_write(this->input_framer, in, n);
    while (framer_num_available(this->input_framer) >= this->n_fft) {
        // forward transform
        framer_read_consume(this->input_framer, this->frame, this->n_fft, this->hop_length);
        arm_mult_f32(this->frame, this->window, this->frame, this->n_fft);
        arm_rfft_fast_f32(&this->rfft_instance, this->frame, this->real_imag, 0);
        // call delegate
        if (this->callback) {
            this->callback(this->user, this->real_imag, this->n_fft);
        }
        // inverse transform
        arm_rfft_fast_f32(&this->rfft_instance, this->real_imag, this->frame, 1);
        arm_mult_f32(this->frame, this->window, this->frame, this->n_fft);
        framer_write_ola(this->output_framer, this->frame, this->n_fft, this->hop_length);
    }
    if (framer_num_available(this->output_framer) < (this->n_fft + n)) {
        printf("dsp: the output buffer underflowed: %d\n", (int)framer_num_available(this->output_framer));
        return n;
    }
    framer_read(this->output_framer, out, n);
    return 0;
}

