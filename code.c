#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define SEM_NAME "/exemple_semaphore"
#define NUM_CHILDREN 4

sem_t *sem;

void handle_sigusr1(int sig) {
    printf("Le fils %d a reçu SIGUSR1\n", getpid());
}

void handle_sigusr2(int sig) {
    printf("Le parent a reçu SIGUSR2 du fils\n");
}

void child_process(int index) {
    signal(SIGUSR1, handle_sigusr1);
    sem_wait(sem);
    printf("Le fils %d commence à travailler\n", getpid());
    sleep(2 + index);
    kill(getppid(), SIGUSR2);
    sem_post(sem);
    exit(EXIT_SUCCESS);
}

int main() {
    pid_t children[NUM_CHILDREN];
    int i;

    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open a échoué");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < NUM_CHILDREN; i++) {
        children[i] = fork();
        if (children[i] == -1) {
            perror("fork a échoué");
            exit(EXIT_FAILURE);
        } else if (children[i] == 0) {
            child_process(i);
        }
    }

    signal(SIGUSR2, handle_sigusr2);

    for (i = 0; i < NUM_CHILDREN; i++) {
        kill(children[i], SIGUSR1);
    }

    sem_post(sem);

    for (i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    sem_close(sem);
    sem_unlink(SEM_NAME);

    printf("Le processus parent a terminé\n");
    return EXIT_SUCCESS;
}

