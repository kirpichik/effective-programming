#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <time.h>
#define ARRAY_SIZE (200 * 1024 * 1024 / sizeof(int))

int arr[ARRAY_SIZE];
int arr2[ARRAY_SIZE];

int main(int argc, const char *argv[]) {
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        arr[i] = rand() % ARRAY_SIZE;
    }
    unsigned int iter_num  = atoi(argv[1]);
    uint64_t start;
    uint64_t end;
    double aver = 0;
    uint64_t min = 0;
    for (int j = 0; j < 10; j++) {
    start = __rdtsc();
    for (int i = 0; i < iter_num; ++i) {
        arr2[arr[i]] = arr[i];
