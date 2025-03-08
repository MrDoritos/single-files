/*
    gcc fibonacci.c -o fibonacci.o -g -static -lgmp -O2 && sudo ./fibonacci.o
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sched.h>
#include <gmp.h>

mpz_t a, b, c;
__uint64_t i = 0, bt = 2048;

void print_exit() {
    if (i == 0)
        return;

    printf("Iter:\n%llu\n", i);
    printf("\nNum:\n");
    mpz_out_str(stdout, 10, &c);
    printf("\n");
    exit(0);
}

void fibonacci() {
    mpz_inits(&a, &b, &c, '\0');
    mpz_init2(&c, bt);
    mpz_init2(&a, bt);
    mpz_init2(&b, bt);
    mpz_set_si(&b, 1);
    for (i = 0;; ++i) {
        mpz_add(&c, &a, &b);
        mpz_set(&a, &b);
        mpz_set(&b, &c);
    }
}

int main() {
    pid_t pid = fork();

    if (pid < 0)
        return -1;

    struct sched_param param;
    sched_getparam(pid, &param);
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(pid, SCHED_FIFO, &param);

    if (pid != 0) {
        signal(SIGQUIT, print_exit);
        fibonacci();
    }   

    signal(SIGQUIT, print_exit);

    usleep(1000000);
    kill(pid, SIGQUIT);

    return 0;
}