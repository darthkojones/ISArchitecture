#include "smpi.h"
//#include "smpi.cpp"
#include <iostream>
#include <pthread.h>
#include <array>
#include <climits>
#include <random>
# include <ctime>

// matrix dimension
#define DIMENSION 400
// matrix index from Column and row
#define MATINDEX(row, col) ((row) * DIMENSION + (col))
// matrix size, since we are dealing with a square one, dimension squared
#define MATRIX_SIZE (DIMENSION * DIMENSION)

// Itterations for a test
int ITERATIONS = 30;
// how many threads
int THREAD_COUNT = 12;

// matrixes, a and b are the multipliers
// c is the result
int* A;
int* B;
int* C;
// smpi message data for row and column that will be multiplied my an mpi
// mpi executes operation on these
typedef struct {
    int row;
    int col;
    int index;
} data_t;
// this data contains the resulting cell value and its index on matrix C
// used to send result from mpi to main as a return value after operation is done
typedef struct {
    int res;
    int index;
} res_data_t;

//gets the cell value from a matrix
int get_matrix_element(int matrix[], int row, int col) {
    return matrix[row * DIMENSION + col];
}

// prints a matrix, adapted to C++ from C code in EX_1
void printMatrix(int* mat) {
    int min = INT_MAX, max = INT_MIN, sum = 0;

    for (int i = 0; i < DIMENSION; i++) {
        bool firstElement = true;

        for (int j = 0; j < DIMENSION; j++) {
            if (firstElement) {
                firstElement = false;
            } else {
                std::cout << "\t";
            }

            int e = mat[MATINDEX(i, j)];
            std::cout << e;

            if (e < min) {
                min = e;
            }
            if (e > max) {
                max = e;
            }
            sum += e;
        }
        std::cout << std::endl;
    }
    std::cout << "Minimaler Wert: " << min << "; Maximaler Wert: " << max << "; Summe aller Werte: " << sum << std::endl;
}

// gets the timestapm of a realTime clock
double get_timestamp() {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_nsec) / 1000000000.0;
}

// Thread function.
// here we initialise smpi,
// we distribute tasks to threads
// and we exechange data between threads using smpi( send and receive)
// we run this for a predefined Itterations as a test case.(Wer (einmal) misst, misst Mist)
void* threadFunc(void*) {
    for(int t = 0; t < ITERATIONS; t++){
        smpi_init();
        int rank = smpi_get_rank();

        if (rank == 0) {
            // Distribute work only if there are multiple threads
            if (THREAD_COUNT > 1) { 
                for (int i = 0; i < MATRIX_SIZE; i++) {
                    int row = i / DIMENSION;
                    int col = i % DIMENSION;
                    int targetThread = (i % (THREAD_COUNT - 1)) + 1;
                    data_t data = {row, col, i};
                    // line for debugging
                    //std::cout << "Thread " << rank << ": Sending data for C[" << row << "][" << col << "] to thread " << targetThread << std::endl;
                    if (smpi_send(&data, sizeof(data_t), targetThread) != SMPI_OK) {
                        std::cerr << "Thread " << rank << ": Error sending data to thread " << targetThread << std::endl;
                        continue;
                    }
                    // we wait to receive result of task
                    res_data_t res_data;
                    if (smpi_recv(&res_data, sizeof(res_data_t), &targetThread) != SMPI_OK) {
                        std::cerr << "Thread " << rank << ": Error receiving result from thread " << targetThread << std::endl;
                        continue;
                    }
                    // and we set the result value at its index
                    C[res_data.index] = res_data.res;
                    //std::cout << "Thread " << rank << ": Received result " << res_data.res << " for C[" << row << "][" << col << "] from thread " << targetThread << std::endl;
                }

                for (int i = 1; i < THREAD_COUNT; i++) {
                    data_t exitSignal = {-1, -1, -1};
                    smpi_send(&exitSignal, sizeof(data_t), i);
                }
            } else { // Single thread (THREAD_COUNT = 1)
                //std::cout << "Single thread calculation" << std::endl;
                for (int i = 0; i < MATRIX_SIZE; i++) {
                    int row = i / DIMENSION;
                    int col = i % DIMENSION;
                    int result = 0;
                    for (int k = 0; k < DIMENSION; k++) {
                        result += get_matrix_element(A, row, k) * get_matrix_element(B, k, col);
                    }
                    C[i] = result;
                }
            }
        } else {
            // Worker threads: receive data, compute the product for a cell, and send results back
            while (true) {
                // this data contains row of A, 
                // column of B 
                // and index of C so we have info on where the result belongs
                data_t data;
                int source;
                if (smpi_recv(&data, sizeof(data_t), &source) != SMPI_OK) {
                    //std::cerr << "Thread " << rank << ": Error receiving data from thread " << source << std::endl;
                    continue;
                }
                // no index means we dont have data to be calculated. so we break loop out
                if (data.index == -1) {
                    //std::cout << "Thread " << rank << ": Received exit signal. Exiting." << std::endl;
                    break;
                }
                // we multiply the product for the cell in C
                int result = 0;
                for (int k = 0; k < DIMENSION; k++) {
                    result += get_matrix_element(A, data.row, k) * get_matrix_element(B, k, data.col);
                }
                // we send result back to main along with its index
                res_data_t res_data = {result, data.index};
                if (smpi_send(&res_data, sizeof(res_data_t), 0) != SMPI_OK) {
                    std::cerr << "Thread " << rank << ": Error sending result to thread 0" << std::endl;
                }
            }
        }
        // we have to call this to finish the thread.
        smpi_finalize();
    }
    return nullptr;
}

int getRandom(){
    // random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 10);
    return dist(gen);
}

int main() {
    A = new int[MATRIX_SIZE];
    B = new int[MATRIX_SIZE];
    C = new int[MATRIX_SIZE];

    for (int i = 0; i < MATRIX_SIZE; i++) {
        A[i] = getRandom();
        B[i] = getRandom();
    }

    smpi_setup(THREAD_COUNT);
    pthread_t threads[THREAD_COUNT];

    std::cout << "Matrix size\t: " << MATRIX_SIZE << std::endl;
    std::cout << "Threads count\t: "<< THREAD_COUNT <<std::endl;
    std::cout << "Iterrations\t: " << ITERATIONS <<std::endl;
    // measurement start
    double totalTime = 0;
    double start = get_timestamp();
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, threadFunc, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    double end = get_timestamp();
    totalTime = end -start;

    // measurement end
    std::cout << "Average time\t: "<<totalTime << std::endl;

    // print matrix a
    /*std::cout << "Matrix A:" << std::endl;
    printMatrix(A);
    // print matrix b
    std::cout << "Matrix B:" << std::endl;
    printMatrix(B);
    // print matrix C
    std::cout << "Matrix C:" << std::endl;
    printMatrix(C);*/

    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}