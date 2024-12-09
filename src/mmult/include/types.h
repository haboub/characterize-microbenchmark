/* types.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 13 Nov. 2023
 * 
 * This file contains all required types decalartions.
*/

#ifndef __INCLUDE_TYPES_H_
#define __INCLUDE_TYPES_H_

typedef struct {
  float* input_a;  // Pointer to the first input matrix
  float* input_b;  // Pointer to the second input matrix
  float* output;   // Pointer to the output matrix

  size_t rowsA;
  size_t colsA;
  size_t colsB;

  size_t size;

  int     cpu;
  int     nthreads;
} args_t;

#endif //__INCLUDE_TYPES_H_
