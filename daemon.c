#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <resolv.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

int flag_do = 0;
int flag_stop = 0;
sem_t* sem_p;
char* sem_name = "/mysem";

void sigintHandler(int signum){
	printf("Signal number %d has been caught! \n", signum);
	flag_stop = 1;
}

void sighupHandler(int signum){
	printf("Signal number %d has been caught! \n", signum);
	flag_do = 1;	
}

void sigchldHandler(int signum){
	sem_post(sem_p); // unlocking the semaphore when a child process exits
}

int Daemon(char* argv[]) {
	signal(SIGINT, sigintHandler); // signal 2 stops the daemon
	signal(SIGHUP, sighupHandler); // signal 1 makes the daemon execute all the commands
	signal(SIGCHLD, sigchldHandler); // lets the semaphore be unlocked as soon as a child exits
	
	printf("Daemon function has been called \n");
	pid_t pid;
	char* argv2[] = {argv[1], NULL};	
	sem_p = sem_open(sem_name, O_CREAT); // creating a new named semaphore 
	sem_post(sem_p); // just in case 
		
	while(1) {
		pause();		
		if (flag_do == 1){
			printf("Working...\n");			
			int fd = open(argv[1], O_RDONLY, S_IRWXU); // read only permission
			if (fd < 0) {
				printf("Error opening an input file has occured!\n");
				exit(1);
			}	
			
			char buf[1024];			
			read(fd, buf, sizeof(buf));
			close(fd);
			char* input_commands[1024];
			int input_commands_count = 0; 
			
			char* command = strtok(buf, "\n");
			printf("List of input commands:\n");
			while (command != NULL){
				input_commands[input_commands_count] = command;
				input_commands[input_commands_count][strlen(command)] = '\0';
				command = strtok(NULL, "\n");
				printf("%s\n", input_commands[input_commands_count]);
				input_commands_count++;
			}
			
			fd = open("daemon_output.txt", O_RDWR|O_CREAT, S_IRWXU);
			ftruncate(fd, 0); // if we have used the program before, the file may be filled with previous data
			// so we clean it up
			
			for (int i = 0; i < input_commands_count; i++){
				pid = fork(); // new child process
				if (pid == 0){
					sem_wait(sem_p);					
					lseek(fd, 0, SEEK_END);
					dup2(fd, 1); // redirecting the output from stdout to our file
					printf("Command %d (%s):\n", i + 1, input_commands[i]); 
					execve(input_commands[i], argv2, NULL);
				}							
				else if (pid > 0 ){
					int status;
					wait(&status); // zombie fix
				}
			}	
			close(fd);		
		}		
		flag_do = 0; // lets us use the daemon multiple times
		
		if (flag_stop == 1) { // if we are done working with the daemon
			sem_wait(sem_p);
			sem_post(sem_p); // unlocking the semaphore
			sem_unlink(sem_name); // removing the named semaphore 
			printf("Stopping the daemon\n");
			exit(0);
		}		
	}
	return 0;
}

int main(int argc, char* argv[])
{
	pid_t parpid;
	if((parpid=fork())<0){                   
		printf("\ncan't fork"); 
		exit(1);               
	}
	else if (parpid!=0){
		int status;
		wait(&status); // zombie fix
		exit(0); // parpid != 0, ending the parent process
	}
	setsid(); // setting the child process to the new session (disconnecting it from the shell)
	printf("Hey, this is daemon. My pid is %i\n", getpid());
	Daemon(argv); // daemon call
    return 0;
}
