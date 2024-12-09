/* ref.c
 *
 * Author: Khaleel Alhaboub
 * Date  : 28 Nov. 2024
 *
 *  Implmentation of Naive mmult for ref
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

/* Reference Implementation */
void* impl_ref(void* args)
{
    /* Get the argument struct */
    args_t* parsed_args = (args_t*)args;

    /* Get all the arguments */
    register const float* matA = parsed_args->input_a;
    register const float* matB = parsed_args->input_b;
    register float* dest = parsed_args->output;
    register size_t rowsA = parsed_args->rowsA;
    register size_t colsA = parsed_args->colsA;
    register size_t colsB = parsed_args->colsB;

    // Initialize destination matrix
    for (size_t i = 0; i < rowsA; i++) {
        for (size_t j = 0; j < colsB; j++) {
            dest[i * colsB + j] = 0.0f; // Initialize to zero
        }
    }

    /* Naive matrix multiplication */
    for (register size_t i = 0; i < rowsA; i++) {
        for (register size_t j = 0; j < colsB; j++) {
            float sum = 0.0f;
            for (register size_t k = 0; k < colsA; k++) {
                float a_element = matA[i * colsA + k];
                float b_element = matB[k * colsB + j];

                sum += a_element * b_element;
            }
            dest[i * colsB + j] += sum;
        }
    }

    return NULL;
}
