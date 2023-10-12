/******************************************************************************
 *
 * framer implements a ringbuffer with overlap_add capability.
 * 
 *****************************************************************************/
#pragma once

#include <stdlib.h>

struct framer;

struct framer* framer_create(size_t size);
void framer_destroy(struct framer* this);

size_t framer_num_available(struct framer* this);
size_t framer_num_free(struct framer* this);
void framer_write(struct framer* this, const float* p, size_t n);
void framer_write_zeros(struct framer* this, size_t n);
void framer_write_ola(struct framer* this, const float* p, size_t n, size_t h);
void framer_read(struct framer* this, float* p, size_t n);
void framer_read_consume(struct framer* this, float* p, size_t n, size_t h);
