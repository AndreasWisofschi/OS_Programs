/*
Andreas Wisofschi
CS 3305 Assignment 4
March 19, 2024
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int waitTime;
    int turnaroundTime;
    int isCompleted;
} Process;

void readProcesses(const char *filename, Process **processes, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    *count = 0;
    while (fgets(line, sizeof(line), file)) {
        (*count)++;
    }

    *processes = (Process *)malloc(sizeof(Process) * (*count));
    if (!*processes) {
        perror("Memory allocation error");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    rewind(file);
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < *count) {
        sscanf(line, "P%d,%d", &((*processes)[i].id), &((*processes)[i].burstTime));
        (*processes)[i].arrivalTime = i;
        (*processes)[i].remainingTime = (*processes)[i].burstTime;
        (*processes)[i].waitTime = 0;
        (*processes)[i].turnaroundTime = 0;
        (*processes)[i].isCompleted = 0;
        i++;
    }
    fclose(file);
}

void simulateFCFS(Process *processes, int count) {
    printf("First Come First Served\n");
    int currentTime = 0, completed = 0, i = 0;
    float totalWaitTime = 0, totalTurnaroundTime = 0;
    
    while (completed < count) {
        if (!processes[i].isCompleted && processes[i].arrivalTime <= currentTime) {
            while (processes[i].remainingTime > 0) {
                printf("T%d : P%d - Burst left %2d, Wait time %2d, Turnaround time %2d\n",
                       currentTime, processes[i].id, processes[i].remainingTime,
                       processes[i].waitTime, processes[i].turnaroundTime);
                for (int j = 0; j < count; j++) {
                    if (!processes[j].isCompleted && processes[j].arrivalTime <= currentTime) {
                        if (j != i) {
                            processes[j].waitTime++;
                        }
                        processes[j].turnaroundTime++;
                    }
                }
                processes[i].remainingTime--;
                currentTime++;
            }
            processes[i].isCompleted = 1;
            completed++;
        } else {
            currentTime++;
        }
        i = (i + 1) % count;
    }

    for (i = 0; i < count; i++) {
        totalWaitTime += processes[i].waitTime;
        totalTurnaroundTime += processes[i].turnaroundTime;
    }
    printf("Total average waiting time:     %.1f\n", totalWaitTime / count);
    printf("Total average turnaround time:  %.1f\n", totalTurnaroundTime / count);
}

void simulateSJF(Process *processes, int count) {
    printf("Shortest Job First\n");
    int currentTime = 0, completed = 0;
    float totalWaitTime = 0, totalTurnaroundTime = 0;

    while (completed < count) {
        int idx = -1;
        int minBurstTime = INT_MAX;

        // Find the shortest job
        for (int i = 0; i < count; ++i) {
            if (!processes[i].isCompleted && processes[i].arrivalTime <= currentTime && processes[i].remainingTime < minBurstTime) {
                minBurstTime = processes[i].remainingTime;
                idx = i;
            }
        }
    
        // If no process is ready, increment time
        if (idx == -1) {
            currentTime++;
            continue;
        }

        // Before executing the shortest job for one tick, update wait and turnaround time for all arrived processes
        for (int i = 0; i < count; i++) {
            if (!processes[i].isCompleted && processes[i].arrivalTime <= currentTime) {
                if (i != idx) {
                    processes[i].waitTime++;
                }
                processes[i].turnaroundTime++;
            }
        }

        Process *current = &processes[idx];

        // Execute the process
        printf("T%d : P%d  - Burst left  %2d, Wait time %2d, Turnaround time %2d\n",
               currentTime, current->id, current->remainingTime - 1, current->waitTime, current->turnaroundTime);

        current->remainingTime--;
        if (current->remainingTime == 0) {
            current->isCompleted = 1;
            completed++;
            totalWaitTime += current->waitTime;
            totalTurnaroundTime += current->turnaroundTime; // Here turnaround time has already been incremented correctly
        }

        currentTime++;
    }

    printf("Total average waiting time:     %.1f\n", totalWaitTime / count);
    printf("Total average turnaround time:  %.1f\n", totalTurnaroundTime / count);
}

void simulateRR(Process *processes, int count, int quantum) {
    printf("Round Robin with Quantum %d\n", quantum);
    int currentTime = 0, completed = 0;
    float totalWaitTime = 0, totalTurnaroundTime = 0;
    int processIndex = 0; //index

    // Initialize process
    int* executedTicks = (int*)malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) {
        executedTicks[i] = 0; // No ticks executed initially
    }

    while (completed < count) {
        int executed = 0; 

        for (int i = 0; i < count; i++) { 
            int idx = (processIndex + i) % count; // Round Robin scheduling https://www.scaler.com/topics/round-robin-scheduling-program-in-c/
            if (!processes[idx].isCompleted && processes[idx].arrivalTime <= currentTime) {
                int execTime = quantum < processes[idx].remainingTime ? quantum : processes[idx].remainingTime;
                processes[idx].remainingTime -= execTime;
                executedTicks[idx] += execTime;
                executed = 1; // Mark as executed

                for (int t = 0; t < execTime; t++) {
                    // Update wait and turnaround time for all processes
                    for (int j = 0; j < count; j++) {
                        if (j != idx && !processes[j].isCompleted && processes[j].arrivalTime <= currentTime) {
                            processes[j].waitTime++;
                        }
                    }
                    currentTime++; // Increment current time after each tick
                    processes[idx].turnaroundTime = currentTime - processes[idx].arrivalTime; // Update turnaround time
                    printf("T%d : P%d - Burst left %2d, Wait time %2d, Turnaround time %2d\n",
                           currentTime, processes[idx].id, processes[idx].remainingTime,
                           processes[idx].waitTime, processes[idx].turnaroundTime);
                }

                if (processes[idx].remainingTime == 0) {
                    processes[idx].isCompleted = 1;
                    completed++;
                }

                processIndex = (idx + 1) % count; // Move to the next process for the next iteration
                break; // Break the loop since we've executed a process this tick
            }
        }

        if (!executed) {
            //still increment currentTime and print a tick
            printf("T%d : No process executed\n", currentTime);
            currentTime++;
        }
    }

    for (int i = 0; i < count; i++) {
        totalWaitTime += processes[i].waitTime;
        totalTurnaroundTime += processes[i].turnaroundTime;
    }

    printf("Total average waiting time: %.1f\n", totalWaitTime / count);
    printf("Total average turnaround time: %.1f\n", totalTurnaroundTime / count);

    free(executedTicks);
}


// Main function
// Accept 3 args in total (fcfs, shortestjob first, and round robin)
int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        printf("Usage: ./assignment-4 -f assignment-4-input.csv\n");
        printf("       ./assignment-4 -s assignment-4-input.csv\n");
        printf("       ./assignment-4 -r <quantum> assignment-4-input.csv\n");
        return 1;
    }

    Process *processes = NULL;
    int processCount = 0;
    readProcesses(argv[argc - 1], &processes, &processCount);

    if (strcmp(argv[1], "-f") == 0) {
        simulateFCFS(processes, processCount);
    } else if (strcmp(argv[1], "-s") == 0) {
        simulateSJF(processes, processCount);
    } else if (strcmp(argv[1], "-r") == 0 && argc == 4) {
        int quantum = atoi(argv[2]);
        simulateRR(processes, processCount, quantum);
    } else {
        printf("Invalid arguments.\n");
        free(processes);
        return 1;
    }

    free(processes);
    return 0;
}
