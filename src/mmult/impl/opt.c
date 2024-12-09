/* opt.c
 *
 * Author: Khaleel Alhaboub
 * Date  : 28 Nov. 2024
 *
 *  Implmentation of opt mmult
 */

/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "../include/types.h"
#include <stddef.h> // For size_t

static inline size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

/* Opt Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void* impl_mmult_opt(void* args)
{
  // Get the argument struct
  args_t* parsed_args = (args_t*)args;

  // Get all the arguments
  const float* matA = (const float*)parsed_args->input_a;
  const float* matB = (const float*)parsed_args->input_b;
  float* dest = (float*)parsed_args->output;
  size_t rowsA = parsed_args->rowsA;
  size_t colsA = parsed_args->colsA;
  size_t colsB = parsed_args->colsB;

  // Initialize destination matrix
  for (size_t i = 0; i < rowsA; i++) {
      for (size_t j = 0; j < colsB; j++) {
          dest[i * colsB + j] = 0.0f;
      }
  }

  // Block size
  const size_t BLOCK_SIZE = 16;

  for (size_t ii = 0; ii < rowsA; ii += BLOCK_SIZE) {
      for (size_t jj = 0; jj < colsB; jj += BLOCK_SIZE) {
          for (size_t kk = 0; kk < colsA; kk += BLOCK_SIZE) {
              for (size_t i = ii; i < min(ii + BLOCK_SIZE, rowsA); i++) {
                  for (size_t j = jj; j < min(jj + BLOCK_SIZE, colsB); j++) {
                      float val = dest[i * colsB + j]; // Start with existing value
                      for (size_t k = kk; k < min(kk + BLOCK_SIZE, colsA); k++) {
                          val += matA[i * colsA + k] * matB[k * colsB + j];
                      }
                      dest[i * colsB + j] = val; // Write back the accumulated value
                  }
              }
          }
      }
  }

  return NULL;
}
#pragma GCC pop_options
