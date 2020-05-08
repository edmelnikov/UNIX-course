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

int flag = 0;

void signalHandler(int signum){
	printf("Signal number %d was caught! \n", signum);
	flag = 1;
}

int Daemon(char* argv[]) {
	signal(SIGINT, signalHandler);
	printf("Daemon function has been called \n");
	int fd; // file descriptor
	int str_length;
	char buf[128];
	fd = open(argv[1], O_RDONLY, S_IRWXU); // read only permission
	if (fd < 0) {
		printf("Error has occured");
		exit(1);
	}
	str_length = read(fd, buf, 128);
	close(fd);
	buf[str_length - 1] = '\0';
	printf("The command is %s\n", buf);
	pause();
	if (flag){
		fd = open("daemon_output.txt", O_RDWR|O_CREAT, S_IRWXU); // we create, if the file does not exist, read/write, file owner has read, write, exec permissions
		if (fd < 0) {
			printf("Error has occured");
			exit(1);
		}
		int saveStdout = dup(1); // saving stdout 
		dup2(fd, 1); // redirecting the output from stdout to our file		
		char* argv2[] = {argv[1], NULL};
		execve(buf, argv2, NULL); // the first element of argv array must start with the filename associated with the file being executed
		dup2(saveStdout, 1); // redirecting the output back to stdout
		printf("Error has occured in execve()\n"); 
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
		wait(&status);
    	exit(0);            // parpid != 0, ending the parent process
    }
    setsid();           // setting the child process to the new session (disconnecting it from the shell)
    printf("Hey, this is daemon. My pid is %i\n", getpid());
	Daemon(argv);           // daemon call
    return 0;
}
