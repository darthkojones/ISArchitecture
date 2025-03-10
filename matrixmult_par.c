#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>

// Die Dimensionen sind zwar fix am Übungszettel vorgegeben,
// aber prinzipiell sollte man sie trotzdem im Programm nicht hart-coden.
// Wenn man sie als Konstanten definiert, kann man sie später leichter ändern.
// #define MATRIX_SIZE 10
// #define MATRIX_SIZE 12
// #define S_DIM 20
#define MATRIX_SIZE 1000
#define MAX_THREADS 12
#define TASKS_PER_THREAD 192

// Makro, welches einen 2-dimensionalen Index in einem 1-dimensionalen Index umwandelt
// x .. x-Index (Anzahl der Zeilen)
// y .. y-Index (Anzahl der Spalten)
// s .. Größe der y-Dimension (Max. Anzahl der Spalten)
#define MATINDEX(x,y,s) ((x)*(s)+(y))

// need a timer function, using the one from algo class with mr Corradini
uint64_t timestamp(){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return ((uint64_t)tp.tv_sec)*1000000000 + tp.tv_nsec;
}

// Die Matrizen A, B und C
// Diese als globale Variablen zu definieren, macht es für die Threads einfacher, darauf zuzugreifen
int *A, *B, *C;

// Struktur für die Datenübergabe an die Threads
typedef struct {
    int start; // too many x y i j k l m n o p q r s t u v w named variables confuse me
    int end;   // so i changed them to start and end
} thread_data_t;

void* thread_func(void* arg){
    thread_data_t* data = (thread_data_t*)arg; //cast the argument to the correct data type
    for(int i= data->start;i<data->end;i++){   //loop through the cells taking into account the start and end values
        int row = i / MATRIX_SIZE; //calculate the row
        int col = i % MATRIX_SIZE; //calculate the column by taking the remainder of the division
        int result = 0;
        for(int j = 0; j < MATRIX_SIZE; j++){
            result += A[MATINDEX(row,j,MATRIX_SIZE)] * B[MATINDEX(j,col,MATRIX_SIZE)];
        }
        C[MATINDEX(row,col,MATRIX_SIZE)] = result;
    }
    return NULL;
}

// Gebe die übergebene Matrix aus
// mat .. Matrix,
// x .. Anzahl der Zeilen
// y .. Anzahl der Spalten
void printMatrix(int* mat, int x, int y) {
    int min = INT_MAX, max = INT_MIN, sum = 0;
    // Schleife über alle Zeilen
    for (int i = 0; i < x; i++) {
        int firstElement = 1;
        // Schleife über alle Spalten
        for (int j = 0; j < y; j++) {
            // Für das erste Element müssen wir davor kein '\t' ausgeben
            if (firstElement) {
                firstElement = 0;
            } else {
              //  printf("\t");
            }
            int e = mat[MATINDEX(i,j,y)];
            // Gebe Element aus
            //printf("%d", e);
            // Berechne min
            if (e < min) {
                min = e;
            }
            // Berechne max
            if (e > max) {
                max = e;
            }
            // Berechne Summe
            sum += e;
        }
        //printf("\n");
    }
    // Gebe min, max und summe aus
    printf("Minimaler Wert: %d; Maximaler Wert: %d; Summe aller Werte: %d\n", min, max, sum);
}

void run_parallel_matrix_mult(int num_threads) {
    int totalCells = MATRIX_SIZE * MATRIX_SIZE;
    int cellsPerThread = totalCells / num_threads;
    int remainder = totalCells % num_threads;

    pthread_t threads[num_threads];
    thread_data_t threadData[num_threads];
    int currentIndex = 0;
    for (int t = 0; t < num_threads; t++) {
        threadData[t].start = currentIndex;
        threadData[t].end = currentIndex + cellsPerThread;
        if (t == num_threads - 1) {
            threadData[t].end += remainder;
        }
        currentIndex = threadData[t].end;
        pthread_create(&threads[t], NULL, thread_func, &threadData[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

int main() {
    // Alloziere Speicher für die Matrizen
    A = malloc(sizeof(int)*MATRIX_SIZE*MATRIX_SIZE);
    B = malloc(sizeof(int)*MATRIX_SIZE*MATRIX_SIZE);
    C = malloc(sizeof(int)*MATRIX_SIZE*MATRIX_SIZE);

    for (int i=0; i<MATRIX_SIZE*MATRIX_SIZE; i++){
        A[i] = rand () % 10 + 1;
        B[i] = rand () % 10 + 1;
    }

    // Measure time for 1, 2, 4, 8, and 12 threads
    int thread_counts[] = {1, 2, 4, 8, 12};
    for (int i = 0; i < 5; i++) {
        int num_threads = thread_counts[i];
        uint64_t start_parallel = timestamp();

        run_parallel_matrix_mult(num_threads);

        uint64_t end_parallel = timestamp();
        uint64_t time_parallel = end_parallel - start_parallel;

        printf("Parallel Matrix Multiplication Time with %d threads: %lu ns\n", num_threads, time_parallel);
    }

    // Gebe Ergebnis aus
    printMatrix(C, MATRIX_SIZE, MATRIX_SIZE);

    free(A);
    free(B);
    free(C);

    return 0;
}