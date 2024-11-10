


#include <stdlib.h>
#include <lib.h>
#include <stdio.h>
#include <lib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(char argc, char ** argv){

	message m;
	int ret, skip, pid_found, longest_path, i, forks, root;
	pid_t pid;
	int depth = 1;

	printf("Main process with PID:%d created by: %d\n", getpid(), getppid());		

	if (argc < 3){
		fprintf(stderr, "Usage %s <pid_to_skip> <number_of_forks>\n", argv[0]);
		return 1;
	}

	forks = atoi(argv[2]);
	for (i = 0; i < forks; i++){
		pid = fork();

		if (pid < 0){
			perror("Fork failed\n");
			exit(1);
		}

		else if (pid > 0){
			return 0;
		}

		else{
			depth++;
			printf("Process at depth %d with PID %d created by parent PID %d\n", depth, getpid(), getppid());	
		}
	}
		
		

	m.m1_i1 = atoi(argv[1]);
	ret = _syscall( MM, TEST, & m );
	longest_path = ret & 1023;
	pid_found = ret;
	pid_found >>= 10;
	printf("Excluding process with id: %d\n", skip);	
	printf("Pid of process with longest chain: %d\n", pid_found);
	printf("Length: %d\n", longest_path);
	
	return 0;
}
