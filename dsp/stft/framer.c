/******************************************************************************
 *
 * framer implements a ringbuffer with overlap_add capability.
 * 
 *****************************************************************************/
#include <assert.h>
#include <memory.h>

#include "arm_math.h"

#include "framer.h"

size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

struct framer {
    float* buffer;
    float* zeros;
    size_t size;
    size_t read_idx;
    size_t write_idx;
};  

struct framer* framer_create(size_t size) {
    struct framer* this = (struct framer*) calloc(1, sizeof(struct framer));
    this->buffer = (float*) calloc(size, sizeof(float));
    this->zeros = (float*) calloc(size, sizeof(float));
    this->size = size;
    this->read_idx = 0;
    this->write_idx = 0;
    return this;
}

void framer_destroy(struct framer* this) {
    free(this->buffer);
    free(this->zeros);
    free(this);
}

// number of samples to read before empty
size_t framer_num_available(struct framer* this) {
    if (this->write_idx >= this->read_idx) {
        return this->write_idx - this->read_idx;
    } else {
        return (this->size - this->read_idx) + (this->write_idx - 0);
    }
}

// number of samples to write before full
size_t framer_num_free(struct framer* this) {
    return this->size - framer_num_available(this);
}

// write n samples of p into this.
void framer_write(struct framer* this, const float* p, size_t n) {
    assert(n <= framer_num_free(this));
    const size_t n1 = min(n, this->size - this->write_idx);
    memcpy(this->buffer + this->write_idx, p, n1 * sizeof(float));
    this->write_idx = (this->write_idx + n1) % this->size;
    if (n1 < n) {
        const size_t n2 = n - n1;
        memcpy(this->buffer + this->write_idx, p + n1, n2 * sizeof(float));
        this->write_idx = (this->write_idx + n2) % this->size;
    }
}

// write n zero samples into this
void framer_write_zeros(struct framer* this, size_t n) {
    framer_write(this, this->zeros, n);
}

// read n samples into p
void framer_read(struct framer* this, float* p, size_t n) {
    assert(n <= framer_num_available(this));
    const size_t n1 = min(n, this->size - this->read_idx);
    memcpy(p, this->buffer + this->read_idx, n1 * sizeof(float));
    this->read_idx = (this->read_idx + n1) % this->size;
    if (n1 < n) {
        const size_t n2 = n - n1;
        memcpy(p + n1, this->buffer + this->read_idx, n2 * sizeof(float));
        this->read_idx = (this->read_idx + n2) % this->size;
    }
}

// read n samples into p, consume h samples.
void framer_read_consume(struct framer* this, float* p, size_t n, size_t h) {
    assert(h <= n);
    framer_read(this, p, n);
    this->read_idx = (this->read_idx + this->size - (n - h)) % this->size;
}


// write h zeros, add n samples of p into this.
void framer_write_ola(struct framer* this, const float* p, size_t n, size_t h) {
    framer_write_zeros(this, h);
    assert(n <= framer_num_available(this));
    if (this->write_idx >= n) {
        float* d = this->buffer + (this->write_idx - n);
        arm_add_f32(d, p, d, n);
    } else {
        const size_t n1 = n - this->write_idx;
        float* d = this->buffer + this->size - n1;
        arm_add_f32(d, p, d, n1);
        arm_add_f32(this->buffer, p + n1, this->buffer, this->write_idx);
    }
}
