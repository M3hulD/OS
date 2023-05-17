#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define KEY 12345
#define NUM_CHILDREN 2

int main() {
    int semid;
    struct sembuf acquire = {0, -1, SEM_UNDO};
    struct sembuf release = {0, 1, SEM_UNDO};

    // Create a semaphore set with a single semaphore
    semid = semget(KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    // Initialize the semaphore to 1 (unlocked)
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        return 1;
    }

    // Fork multiple child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            // Child process
            printf("Child %d attempting to enter critical section.\n", i);
            if (semop(semid, &acquire, 1) == -1) {
                perror("semop");
                return 1;
            }

            // Critical section: increment counter
            printf("Child %d in critical section.\n", i);
            // Do some critical section work here...
            sleep(1);
            printf("Child %d exiting critical section.\n", i);

            if (semop(semid, &release, 1) == -1) {
                perror("semop");
                return 1;
            }

            exit(0);
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    // Remove the semaphore set
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        return 1;
    }

    return 0;
}
