/* main.c
 *
 * Author: Khalid Al-Hawaj
 * Date  : 12 Nov. 2023
 *
 * This file is structured to call different implementation of the same
 * algorithm/microbenchmark. The file will allocate 3 output arrays one
 * for: scalar naive impl, scalar opt impl, vectorized impl. As it stands
 * the file will allocate and initialize with random data one input array
 * of type 'byte'. To check correctness, the file allocate a 'ref' array;
 * to calculate this 'ref' array, the file will invoke a ref_impl, which
 * is supposed to be functionally correct and act as a reference for
 * the functionality. The file also adds a guard word at the end of the
 * output arrays to check for buffer overruns.
 *
 * The file will invoke each implementation n number of times. It will
 * record the runtime of _each_ invocation through the following Linux
 * API:
 *    clock_gettime(), with the clk_id set to CLOCK_MONOTONIC
 * Then, the file will calculate the standard deviation and calculate
 * an outlier-free average by excluding runtimes that are larger than
 * 2 standard deviation of the original average.
 */

/* Set features         */
#define _GNU_SOURCE

/* Standard C includes  */
/*  -> Standard Library */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/*  -> Scheduling       */
#include <sched.h>
/*  -> Types            */
#include <stdbool.h>
#include <inttypes.h>
/*  -> Runtimes         */
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/ref.h"
#include "impl/naive.h"
#include "impl/opt.h"

/* Include common headers */
#include "common/types.h"
#include "common/macros.h"

/* Include application-specific headers */
#include "include/types.h"

const int A_ROW = 2500;  // Number of rows for Matrix A
const int A_COL_B_ROW = 3000;  // Number of columns for Matrix A and rows for Matrix B
const int B_COL = 2100;  // Number of columns for Matrix B

int main(int argc, char** argv)
{
  /* Set the buffer for printf to NULL */
  setbuf(stdout, NULL);

  /* Arguments */
  int nthreads = 1;
  int cpu      = 0;

  int nruns    = 100;
  int nstdevs  = 3;

  /* Data */
  int mA_rows = A_ROW;
  int mAB_cols_rows = A_COL_B_ROW;
  int mB_cols = B_COL;
  int matrix_a_data_size = A_ROW * A_COL_B_ROW;
  int matrix_b_data_size = A_COL_B_ROW * B_COL;
  int data_size = A_ROW * B_COL;

  /* Parse arguments */
  /* Function pointers */
  void* (*impl_mmult_opt_ptr  )(void* args) = impl_mmult_opt;
  void* (*impl_mmult_naive_ptr)(void* args) = impl_mmult_naive;

  /* Chosen */
  void* (*impl)(void* args) = NULL;
  const char* impl_str      = NULL;

  bool help = false;
  for (int i = 1; i < argc; i++) {
    /* Implementations */
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {
      assert (++i < argc);
      if (strcmp(argv[i], "naive") == 0) {
        impl = impl_mmult_naive_ptr; impl_str = "mmult_naive";
      } else if (strcmp(argv[i], "opt"  ) == 0) {
        impl = impl_mmult_opt_ptr  ; impl_str = "mmult_opt"  ;
      } else {
        impl = NULL                 ; impl_str = "unknown"     ;
      }

      continue;
    }

    /* Rows of Matrix A */
    if (strcmp(argv[i], "-ar") == 0 || strcmp(argv[i], "--arows") == 0) {
        assert(++i < argc);
        mA_rows = atoi(argv[i]);
        continue;
    }

    /* Shared Dimension (columns of A and rows of B) */
    if (strcmp(argv[i], "-acbr") == 0 || strcmp(argv[i], "--acolsnbrows") == 0) {
        assert(++i < argc);
        mAB_cols_rows = atoi(argv[i]);
        continue;
    }

    /* Columns of Matrix B */
    if (strcmp(argv[i], "-bc") == 0 || strcmp(argv[i], "--bcols") == 0) {
        assert(++i < argc);
        mB_cols = atoi(argv[i]);
        continue;
    }

    /* Run parameterization */
    if (strcmp(argv[i], "--nruns") == 0) {
      assert (++i < argc);
      nruns = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "--nstdevs") == 0) {
      assert (++i < argc);
      nstdevs = atoi(argv[i]);

      continue;
    }

    /* Parallelization */
    if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nthreads") == 0) {
      assert (++i < argc);
      nthreads = atoi(argv[i]);

      continue;
    }

    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cpu") == 0) {
      assert (++i < argc);
      cpu = atoi(argv[i]);

      continue;
    }

    /* Help */
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      help = true;

      continue;
    }
  }

  if (help || impl == NULL) {
    if (!help) {
      if (impl_str != NULL) {
        printf("\n");
        printf("ERROR: Unknown \"%s\" implementation.\n", impl_str);
      } else {
        printf("\n");
        printf("ERROR: No implementation was chosen.\n");
      }
    }
    printf("\n");
    printf("Usage:\n");
    printf("  %s {-i | --impl} impl_str [Options]\n", argv[0]);
    printf("  \n");
    printf("  Required:\n");
    printf("    -i    | --impl      Available implementations = {naive}\n");
    printf("    \n");
    printf("  Options:\n");
    printf("    -h    | --help      Print this message\n");
    printf("    -n    | --nthreads  Set number of threads available (default = %d)\n", nthreads);
    printf("    -c    | --cpu       Set the main CPU for the program (default = %d)\n", cpu);
    printf("    -ar   | --arows      Size of input and output data (default = %d)\n", mA_rows);
    printf("    -abbr | --acolsnbrows      Size of input and output data (default = %d)\n", mAB_cols_rows);
    printf("    -bc   | --bcols      Size of input and output data (default = %d)\n", mB_cols);
    printf("         --nruns     Number of runs to the implementation (default = %d)\n", nruns);
    printf("         --stdevs    Number of standard deviation to exclude outliers (default = %d)\n", nstdevs);
    printf("\n");

    exit(help? 0 : 1);
  }

  /* Set our priority the highest */
  int nice_level = -20;

  printf("Setting up schedulers and affinity:\n");
  printf("  * Setting the niceness level:\n");
  do {
    errno = 0;
    printf("      -> trying niceness level = %d\n", nice_level);
    int __attribute__((unused)) ret = nice(nice_level);
  } while (errno != 0 && nice_level++);

  printf("    + Process has niceness level = %d\n", nice_level);

  /* If we are on an apple operating system, skip the scheduling  *
   * routine; Darwin does not support sched_set* system calls ... *
   *                                                              *
   * hawajkm: and here I was--thinking that MacOS is POSIX ...    *
   *          Silly me!                                           */
#if !defined(__APPLE__)
  /* Set scheduling to reduce context switching */
  /*    -> Set scheduling scheme                */
  printf("  * Setting up FIFO scheduling scheme and high priority ... ");
  pid_t pid    = 0;
  int   policy = SCHED_FIFO;
  struct sched_param param;

  param.sched_priority = sched_get_priority_max(policy);
  int res = sched_setscheduler(pid, policy, &param);
  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }

  /*    -> Set affinity                         */
  printf("  * Setting up scheduling affinity ... ");
  cpu_set_t cpumask;

  CPU_ZERO(&cpumask);
  for (int i = 0; i < nthreads; i++) {
    CPU_SET(cpu + i, &cpumask);
  }

  res = sched_setaffinity(pid, sizeof(cpumask), &cpumask);

  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }
#endif
  printf("\n");

  /* Statistics */
  __DECLARE_STATS(nruns, nstdevs);

  /* Initialize Rand */
  srand(0xdeadbeef);

  /* Datasets */
  /* Allocation and initialization */
  float* src1   = __ALLOC_INIT_DATA(float, matrix_a_data_size * sizeof(float));
  float* src2   = __ALLOC_INIT_DATA(float, matrix_b_data_size * sizeof(float));
  float* ref    = __ALLOC_INIT_DATA(float, data_size + 4);
  float* dest   = __ALLOC_DATA(float, data_size + 4);

  /* Setting a guards, which is 0xdeadcafe.
     The guard should not change or be touched. */
  __SET_FLOAT_GUARD(ref , data_size);
  __SET_FLOAT_GUARD(dest, data_size);

  /* Generate ref data */
  /* Arguments for the functions */
  args_t args_ref;

  args_ref.size     = data_size;
  args_ref.output   = ref;
  args_ref.input_a  = src1;
  args_ref.input_b  = src2;
  args_ref.rowsA    = mA_rows;
  args_ref.colsA    = mAB_cols_rows;
  args_ref.colsB    = mB_cols;

  args_ref.cpu      = cpu;
  args_ref.nthreads = nthreads;

  /* Running the reference function */
  impl_ref(&args_ref);

  /* Execute the requested implementation */
  /* Arguments for the function */
  args_t args;

  args.size     = data_size;
  args.rowsA    = mA_rows;
  args.colsA    = mAB_cols_rows;
  args.colsB    = mB_cols;
  args.input_a  = src1;
  args.input_b  = src2;
  args.output   = dest;

  args.cpu      = cpu;
  args.nthreads = nthreads;

  /* Start execution */
  printf("Running \"%s\" implementation:\n", impl_str);

  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __SET_START_TIME();
    (*impl)(&args);
    __SET_END_TIME();
    runtimes[i] = __CALC_RUNTIME();
  }
  printf("Finished\n");

  /* Verfication */
  printf("  * Verifying results .... ");
  bool match = __CHECK_FLOAT_MATCH(ref, dest, data_size, 1e-5f);
  bool guard = __CHECK_FLOAT_GUARD(     dest, data_size);
  if (match && guard) {
    printf("Success\n");
  } else if (!match && guard) {
    printf("Fail, but no buffer overruns\n");
  } else if (match && !guard) {
    printf("Success, but failed buffer overruns check\n");
  } else if(!match && !guard) {
    printf("Failed, and failed buffer overruns check\n");
  }

  /* Running analytics */
  uint64_t min     = -1;
  uint64_t max     =  0;

  uint64_t avg     =  0;
  uint64_t avg_n   =  0;

  uint64_t std     =  0;
  uint64_t std_n   =  0;

  int      n_msked =  0;
  int      n_stats =  0;

  for (int i = 0; i < num_runs; i++)
    runtimes_mask[i] = true;

  printf("  * Running statistics:\n");
  do {
    n_stats++;
    printf("    + Starting statistics run number #%d:\n", n_stats);
    avg_n =  0;
    avg   =  0;

    /*   -> Calculate min, max, and avg */
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] < min) {
          min = runtimes[i];
        }
        if (runtimes[i] > max) {
          max = runtimes[i];
        }
        avg += runtimes[i];
        avg_n += 1;
      }
    }
    avg = avg / avg_n;

    /*   -> Calculate standard deviation */
    std   =  0;
    std_n =  0;

    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        std   += ((runtimes[i] - avg) *
                  (runtimes[i] - avg));
        std_n += 1;
      }
    }
    std = sqrt(std / std_n);

    /*   -> Calculate outlier-free average (mean) */
    n_msked = 0;
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] > avg) {
          if ((runtimes[i] - avg) > (nstd * std)) {
            runtimes_mask[i] = false;
            n_msked += 1;
          }
        } else {
          if ((avg - runtimes[i]) > (nstd * std)) {
            runtimes_mask[i] = false;
            n_msked += 1;
          }
        }
      }
    }

    printf("      - Standard deviation = %" PRIu64 "\n", std);
    printf("      - Average = %" PRIu64 "\n", avg);
    printf("      - Number of active elements = %" PRIu64 "\n", avg_n);
    printf("      - Number of masked-off = %d\n", n_msked);
  } while (n_msked > 0);
  /* Display information */
  printf("  * Runtimes (%s): ", __PRINT_MATCH(match));
  printf(" %" PRIu64 " ns\n"  , avg                 );

  /* Dump */
  printf("  * Dumping runtime informations:\n");
  FILE * fp;
  char filename[256];
  strcpy(filename, impl_str);
  strcat(filename, "_runtimes.csv");
  printf("    - Filename: %s\n", filename);
  printf("    - Opening file .... ");
  fp = fopen(filename, "w");

  if (fp != NULL) {
    printf("Succeeded\n");
    printf("    - Writing runtimes ... ");
    fprintf(fp, "impl,%s", impl_str);

    fprintf(fp, "\n");
    fprintf(fp, "num_of_runs,%d", num_runs);

    fprintf(fp, "\n");
    fprintf(fp, "runtimes");
    for (int i = 0; i < num_runs; i++) {
      fprintf(fp, ", ");
      fprintf(fp, "%" PRIu64 "", runtimes[i]);
    }

    fprintf(fp, "\n");
    fprintf(fp, "avg,%" PRIu64 "", avg);
    printf("Finished\n");
    printf("    - Closing file handle .... ");
    fclose(fp);
    printf("Finished\n");
  } else {
    printf("Failed\n");
  }
  printf("\n");

  /* Manage memory */
  free(src1);
  free(src2);
  free(dest);
  free(ref);

  /* Finished with statistics */
  __DESTROY_STATS();

  /* Done */
  return 0;
}
