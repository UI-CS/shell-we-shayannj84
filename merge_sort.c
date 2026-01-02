#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

void merge(int *a, int s, int m, int e) {
    int n1 = m - s + 1, n2 = e - m;
    int *L = malloc(n1 * sizeof(int)), *R = malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = a[s + i];
    for (int j = 0; j < n2; j++) R[j] = a[m + 1 + j];
    int i = 0, j = 0, k = s;
    while (i < n1 && j < n2) a[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
    free(L); free(R);
}

void p_merge_sort(int *a, int s, int e) {
    if (s >= e) return;
    int m = s + (e - s) / 2;
    
    // Threshold to prevent creating too many processes
    if (e - s < 100) {
        p_merge_sort(a, s, m);
        p_merge_sort(a, m + 1, e);
    } else {
        pid_t left_pid = fork();
        if (left_pid == 0) {
            p_merge_sort(a, s, m);
            exit(0);
        }
        
        pid_t right_pid = fork();
        if (right_pid == 0) {
            p_merge_sort(a, m + 1, e);
            exit(0);
        }
        
        // Wait for both children to finish (JOIN)
        waitpid(left_pid, NULL, 0);
        waitpid(right_pid, NULL, 0);
    }
    merge(a, s, m, e);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    
    // Using explicit flags to ensure Shared Memory works on all Linux systems
    int *a = mmap(NULL, n * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    srand(time(NULL));
    printf("Original: ");
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        if (i < 5) printf("%d ", a[i]);
    }
    printf("\n");

    p_merge_sort(a, 0, n - 1);
    
    printf("Sorted first 5: ");
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);
    printf("\n");

    munmap(a, n * sizeof(int));
    return 0;
}