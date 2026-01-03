#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

int main(int c, char **v) {
    int p = atoi(v[1]); long long n = atoll(v[2]);
    long long *hits = mmap(NULL, 8, 3, 0x21, -1, 0);
    *hits = 0;
    for (int i = 0; i < p; i++) {
        if (fork() == 0) {
            long long count = 0;
            unsigned int seed = time(NULL) ^ getpid();
            for (long long j = 0; j < n / p; j++) {
                double x = (double)rand_r(&seed) / 2147483647 * 2 - 1;
                double y = (double)rand_r(&seed) / 2147483647 * 2 - 1;
                if (x * x + y * y <= 1) count++;
            }
            __sync_fetch_and_add(hits, count);
            exit(0);
        }
    }
    for (int i = 0; i < p; i++) wait(NULL);
    printf("Estimated Pi: %f\n", 4.0 * *hits / n);
    return 0;
}