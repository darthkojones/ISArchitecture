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

// Serial matrix multiplication function
void serial_matrix_mult() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int result = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                result += A[MATINDEX(i, k, MATRIX_SIZE)] * B[MATINDEX(k, j, MATRIX_SIZE)];
            }
            C[MATINDEX(i, j, MATRIX_SIZE)] = result;
        }
    }
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
               // printf("\t");
            }
            int e = mat[MATINDEX(i,j,y)];
            // Gebe Element aus
           // printf("%d", e);
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
       // printf("\n");
    }
    // Gebe min, max und summe aus
    printf("Minimaler Wert: %d; Maximaler Wert: %d; Summe aller Werte: %d\n", min, max, sum);
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

    // Measure time for serial matrix multiplication
    uint64_t start_serial = timestamp();

    serial_matrix_mult();

    uint64_t end_serial = timestamp();
    uint64_t time_serial = end_serial - start_serial;

    printf("Serial Matrix Multiplication Time: %lu ns\n", time_serial);

    // Gebe Ergebnis aus
    printMatrix(C, MATRIX_SIZE, MATRIX_SIZE);

    free(A);
    free(B);
    free(C);

    return 0;
}