/******************************************************************************
 *
 * stft implements a short time fourier transform filterbank.
 * 
 *****************************************************************************/
#pragma once

#include <stdlib.h>

struct stft;
typedef void (*stft_callback)(void* user, float* real_imag, size_t count);

struct stft* stft_create(stft_callback callback, void* user, size_t n_fft, size_t hop_length, size_t buffer_size);
void stft_destroy(struct stft* this);

size_t stft_process(struct stft* this, const float* in, float* out, size_t count);
