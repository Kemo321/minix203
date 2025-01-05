#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <lib.h>



int main(char argc, char ** argv){
    message m;
    m.m1_i1 = getpid();
    m.m1_i2 = atoi(argv[1]);
    _syscall(MM, SETGROUP, &m);
    while(1);
    return 0;
}
