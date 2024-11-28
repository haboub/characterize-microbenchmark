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
    register const byte* matA = parsed_args->input_a;
    register const byte* matB = parsed_args->input_b;
    register byte* dest = parsed_args->output;
    register size_t rowsA = parsed_args->rowsA;
    register size_t colsA = parsed_args->colsA;
    register size_t colsB = parsed_args->colsB;

    /* Naive matrix multiplication */
    for (register size_t i = 0; i < rowsA; i++) {
        for (register size_t j = 0; j < colsB; j++) {
            int sum = 0;
            for (register size_t k = 0; k < colsA; k++) {
                int a_element = 0;
                int b_element = 0;

                // Read 4 bytes to construct the integer
                memcpy(&a_element, matA + (i * colsA + k) * sizeof(int), sizeof(int));
                memcpy(&b_element, matB + (k * colsB + j) * sizeof(int), sizeof(int));

                sum += a_element * b_element;
            }
            // Store the sum in dest
            memcpy(dest + (i * colsB + j) * sizeof(int), &sum, sizeof(int));
        }
    }

    return NULL;
}