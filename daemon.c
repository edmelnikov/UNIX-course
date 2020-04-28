#include <stdio.h> 
#include <string.h> 
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

int Daemon() {
	signal(SIGINT, signalHandler);
	printf("Daemon function has been called \n");
	int fd; // file descriptor
	int str_length; // the length of a written string
	char text[] = "Output from daemon's write() function\n";
	while(1){ // here we are waiting for a signal which changes the flag to 1
		if (flag){
			fd = open("daemon_output.txt", O_RDWR|O_CREAT, S_IRWXU); // we create, if the file does not exist, read/write, file owner has read, write, exec permissions
			ftruncate(fd, 0); // cleans up the file (or causes the file to have a size of 0 bytes)
			lseek(fd, 0, SEEK_END); // set the file offset to the size of the file plus offset
			str_length = write(fd, text, sizeof(text)); // writing our text to the file 
			printf("The string of the size of %i was successfully written to the output file \n", str_length);
			exit(0); // ending the daemon process
		}
	}
	return 0;
}

int main()
{
	pid_t parpid;
    if((parpid=fork())<0){                   
		printf("\ncan't fork"); 
		exit(1);               
	}
    else if (parpid!=0) 
    	exit(0);            // parpid != 0, ending the parent process
    setsid();           // setting the child process to the new session (disconnecting it from the shell)
    printf("Hey, this is daemon. My pid is %i\n", getpid());
	Daemon();           // daemon call
    return 0;
}
