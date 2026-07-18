#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_CHILDREN 3
#define TIMEOUT_SECONDS 3

int main(void) {
    pid_t children[NUM_CHILDREN];

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            int work_time = (i == 1) ? 10 : 2;
            printf("[Child %d] PID=%d starting, will run for %d s\n",
                   i, getpid(), work_time);
            sleep(work_time);
            printf("[Child %d] PID=%d finished work\n", i, getpid());
            exit(EXIT_SUCCESS);
        } else {
            children[i] = pid;
            printf("[Parent] Launched child %d with PID=%d\n", i, pid);
        }
    }

    for (int i = 0; i < NUM_CHILDREN; i++) {
        time_t start = time(NULL);
        int status;
        pid_t result;

        while (1) {
            result = waitpid(children[i], &status, WNOHANG);

            if (result == children[i]) {
                if (WIFEXITED(status))
                    printf("[Parent] Child %d (PID=%d) exited normally, code=%d\n",
                           i, children[i], WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    printf("[Parent] Child %d (PID=%d) killed by signal %d\n",
                           i, children[i], WTERMSIG(status));
                break;
            }

            if (difftime(time(NULL), start) > TIMEOUT_SECONDS) {
                printf("[Parent] Child %d (PID=%d) exceeded timeout, sending SIGTERM\n",
                       i, children[i]);
                kill(children[i], SIGTERM);
                sleep(1);
                waitpid(children[i], &status, 0);
                printf("[Parent] Child %d (PID=%d) reaped after termination\n",
                       i, children[i]);
                break;
            }
            usleep(200000);
        }
    }

    printf("[Parent] All children handled, no zombies remain.\n");
    return 0;
}
