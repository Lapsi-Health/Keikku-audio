#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arm_math.h"

#include "framer.h"
#include "stft.h"

// TODO: add build file
// cc stft_test.c framer.c stft.c -I/Users/adev/src/CMSIS-DSP/Include -L/Users/adev/src/CMSIS-DSP/build/Source -lCMSISDSP; ./a.out

void test_callback(void* user, float* real_imag, size_t count) {
    printf("callback\n");
}

int main() {
    
    float input[256];
    float output[256];
    for (int i = 0; i < 256; ++i) {
        input[i] = i;
    }

    struct stft* stft_test = stft_create(&test_callback, NULL, 64, 16, 1024);
    stft_process(stft_test, input, output, 256);
    for (int i = 0; i < 256; ++i) {
        printf("%d, %f\n", i, output[i]); // input should fall through at latency = n_fft + hop_length
    }
    stft_destroy(stft_test);
}