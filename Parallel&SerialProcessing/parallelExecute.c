#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/types.h>

// Function to check if a number is prime
int is_prime(long num) {
    if (num < 2)
        return 0;
    for (long i = 2; i <= sqrt(num); i++) {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

// Function to count primes and calculate the sum in a range
void count_primes_and_sum(long start, long end, int *count, long *sum) {
    *count = 0;
    *sum = 0;

    for (long i = start; i <= end; i++) {
        if (is_prime(i)) {
            (*count)++;
            (*sum) += i;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <parallel_flag> <min_value> <max_value>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int parallel_flag = atoi(argv[1]);
    long min_value = atol(argv[2]);
    long max_value = atol(argv[3]);

    if (min_value < 0 || max_value > 1000001 || min_value >= max_value) {
        fprintf(stderr, "Invalid input values\n");
        return EXIT_FAILURE;
    }

    long range = (max_value - min_value + 1) / 4;

    pid_t parent_pid = getpid();

    if (parallel_flag) {
        printf("Process id: %d\n", parent_pid);

        for (int i = 0; i < 4; i++) {
            pid_t pid = fork();

            if (pid == 0) {  // Child process
                long start = min_value + i * range;
                long end = (i == 3) ? max_value : min_value + (i + 1) * range;

                int count;
                long sum;
                count_primes_and_sum(start, end, &count, &sum);

                printf("pid: %d, ppid %d - Count and sum of prime numbers between %ld and %ld are %d and %ld\n",
                       getpid(), getppid(), start, end, count, sum);

                exit(EXIT_SUCCESS);
            } else if (pid < 0) {
                perror("fork");
                return EXIT_FAILURE;
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0); // Wait for the child process to finish
            }
        }
    } else {
        // Serial execution
        printf("Process id: %d\n", parent_pid);

        for (int i = 0; i < 4; i++) {
            long start = min_value + i * range;
            long end = (i == 3) ? max_value : min_value + (i + 1) * range;

            int count;
            long sum;
            count_primes_and_sum(start, end, &count, &sum);

            printf("pid: %d, ppid %d - Count and sum of prime numbers between %ld and %ld are %d and %ld\n",
                   getpid(), getppid(), start, end, count, sum);
        }
    }

    return EXIT_SUCCESS;
}
