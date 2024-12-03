#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <lib.h>


int main(char argc, char ** argv){
    int depth = 0;
    message m;
    pid_t pid;
    int i;
    

    for (i = 0; i < 4; i ++){
        pid = fork();
        if (pid < 0){
            exit(1);
        }
        else if (pid > 0){
            m.m1_i1 = getpid();
            m.m1_i2 = depth % 3;
            _syscall( MM, SETGROUP, &m );
            while(1);
        }

        else{
            depth++;
        }
    }
    return 0;

}