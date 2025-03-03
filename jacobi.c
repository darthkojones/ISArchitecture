#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

// Die Dimensionen sind zwar fix am Übungszettel vorgegeben,
// aber prinzipiell sollte man sie trotzdem im Programm nicht hart-coden.
// Wenn man sie als Konstanten definiert, kann man sie später leichter ändern.
// #define  ITERATION_COUNT  20
// #define  MATRIX_SIZE      25000

//#define  ITERATION_COUNT  500
//#define  MATRIX_SIZE      5000

//#define  ITERATION_COUNT  2000
//#define  MATRIX_SIZE      2500

//#define  ITERATION_COUNT  8000
//#define  MATRIX_SIZE      1250

//#define  ITERATION_COUNT  50000
//#define  MATRIX_SIZE      500

#define  ITERATION_COUNT  800000
#define  MATRIX_SIZE      125

// How many threads should be used
#define  THREAD_COUNT     1

// The number of measurements used to calculate the average execution time
#define  MEASUREMENTS     6

typedef struct  {
    int thread_id;
    double** matrix;
    int matrix_size;
    double* right_hand_side;
    double* solution;
    int range_start;
    int range_size;
} thread_info_t;

pthread_barrier_t barrier;

void solve_jacobi_serial(double** matrix, int matrix_size, double* right_hand_side, double* solution) {
    double* tempsolution = (double*)malloc(MATRIX_SIZE * sizeof(double));

    // Iteration loop
    for (int i = 0; i < ITERATION_COUNT; i++) {
        // loop through rows
        for (int j = 0; j < matrix_size; j++) {
            double sigma_value = 0;
            // loop through columns
            for (int k = 0; k < matrix_size; k++) {
                if (j != k) {
                    sigma_value += matrix[j][k] * solution[k];
                }
            }
            tempsolution[j] = (right_hand_side[j] - sigma_value) / matrix[j][j];
        }
        memcpy(solution, tempsolution, MATRIX_SIZE * sizeof(double));
    }
    free(tempsolution);
}

void* solve_jacobi_parallel(void* targ) {
    thread_info_t* info = (thread_info_t*)targ;
    double* tempsolution = (double*)malloc(MATRIX_SIZE * sizeof(double));

    // Iteration loop
    for (int i = 0; i < ITERATION_COUNT; i++) {
        // loop through rows
        for (int j = info->range_start; j < info->range_start + info->range_size; j++) {
            double sigma_value = 0;
            // loop through columns
            for (int k = 0; k < info->matrix_size; k++) {
                if (j != k) {
                    sigma_value += info->matrix[j][k] * info->solution[k];
                }
            }
            tempsolution[j] = (info->right_hand_side[j] - sigma_value) / info->matrix[j][j];
        }

        // Here we need to synchronise before we continue with the next iteration
        pthread_barrier_wait(&barrier);
        memcpy(info->solution + info->range_start, tempsolution + info->range_start, info->range_size * sizeof(double));
    }
    free(tempsolution);
    return NULL;
}

double gettime() {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return (double)time.tv_sec + (double)time.tv_nsec / 1000000000.0;
}

int main(int argc, char **argv) {
    // Declare variables
    int matrix_size = MATRIX_SIZE;
    double **matrix = (double**)malloc(matrix_size * sizeof(double*));
    for (int i = 0; i < matrix_size; i++) {
        matrix[i] = (double*)malloc(matrix_size * sizeof(double));
    }
    double* right_hand_side = (double*)malloc(matrix_size * sizeof(double));
    double* solution = (double*)malloc(matrix_size * sizeof(double));

    printf("matrix size: %d\niteration count: %d\nthread count: %d\n", matrix_size, ITERATION_COUNT, THREAD_COUNT);

    // Initialise barrier if more than one thread
    #if THREAD_COUNT > 1
        pthread_barrier_init(&barrier, NULL, THREAD_COUNT);
    #endif

    // measurement loop
    double avgtime = 0.0;
    for (int m = 0; m < MEASUREMENTS; m++) {
        // Initialize matrix and vectors
        for (int i = 0; i < matrix_size; i++) {
            // Jacobi only works correctly with strictly diagonally dominant matrices
            // Therefore we need to ensure that our random matrix is strictly diagonally dominant
            double row_sum = 0;
            for (int j = 0; j < matrix_size; j++) {
                matrix[i][j] = 1 + rand() % 20;
                if (j != i) {
                    row_sum += matrix[i][j];
                }
            }
            matrix[i][i] = row_sum + 1;
        }
        for (int i = 0; i < matrix_size; i++) {
            right_hand_side[i] = rand() % 20;
            solution[i] = right_hand_side[i];
        }

        // if only one thread then execute serial version
        #if THREAD_COUNT == 1
            double starttime = gettime(); // starttime
            solve_jacobi_serial(matrix, matrix_size, right_hand_side, solution);
            double endtime = gettime(); // endtime
            avgtime += endtime - starttime;
            printf("Time: %lf s\n", endtime - starttime);

        // if more than one thread create threads and execute parallel version
        #else
            thread_info_t infos[THREAD_COUNT];
            pthread_t threads[THREAD_COUNT];

            double starttime = gettime(); // starttime

            for (int i = 0; i < THREAD_COUNT; i++) {
                infos[i].thread_id = i;
                infos[i].matrix = matrix;
                infos[i].matrix_size = matrix_size;
                infos[i].right_hand_side = right_hand_side;
                infos[i].solution = solution;
                infos[i].range_start = (MATRIX_SIZE / THREAD_COUNT) * i;
                if (i == THREAD_COUNT - 1) {
                    infos[i].range_size = MATRIX_SIZE - infos[i].range_start;
                } else {
                    infos[i].range_size = (MATRIX_SIZE / THREAD_COUNT);
                }
                pthread_create(&threads[i], NULL, solve_jacobi_parallel, &infos[i]);
            }
            for (int i = 0; i < THREAD_COUNT; i++) {
                pthread_join(threads[i], NULL);
            }

            double endtime = gettime(); // endtime
            avgtime += endtime - starttime;
            printf("Time: %lf s\n", endtime - starttime);
            printf("Barrier activations per second: %lf\n", (double)ITERATION_COUNT / (endtime - starttime));
        #endif
    }

    printf("Average time: %lf s\n", avgtime / (double)MEASUREMENTS);

    #if THREAD_COUNT > 1
        pthread_barrier_destroy(&barrier);
    #endif

    for (int i = 0; i < matrix_size; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(right_hand_side);
    free(solution);

    return 0;
}