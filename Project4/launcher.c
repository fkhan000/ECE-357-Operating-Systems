#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


int main(int argc, char * argv[])
{
	
	//we want to pipe the output of wordgen to the input of wordsearch
	//and the output of wordsearch to the input of pager

	//we will store the file descriptors for the pipes in fds1 and fds2
	//with fsd1 for the first pipe and fds2 for the second
	int fds1[2];
	int fds2[2];


	pid_t pids[3];

	//we create our pipes

	if( (pipe(fds1) < 0) | (pipe(fds2) < 0))
	{
		fprintf(stderr, "Pipe Failed: %s", strerror(errno));
	}

	//to keep track of which file descriptors we need to redirect
	//we stored it in a 4x2 int array. The first element is for the 
	//I/O redirection for the first child process, the second for the second
	//and so on. However, since the second child process is piped on both ends
	//we need to do two I/O redirections and the second I/O redirection (where the
	//output of second process goes to) is stored in the last element of the array
	int pairs[4][2] = 
	{ 
		{fds1[1], 1},
		{fds1[0], 0},
		{fds2[0], 0},
		{fds2[1], 1}
	};

	for (int i = 0; i < 3; i ++)
	{
		//we create a child process
		if((pids[i] = fork()) < 0)
		{
			//and inform the user and terminate the program if this fails
			fprintf(stderr, "Error creating child process: %s\n", strerror(errno));
			exit(-1);
		}

		//else if we're in the child process
		else if (pids[i] == 0)
		{

			//we then need to close this process' connections to the other ends
			//of the pipes
			
			if(i == 0)
			{
				close(fds1[0]);
				close(fds2[0]);
				close(fds2[1]);
			}
			if(i == 1)
			{
				close(fds1[1]);
				close(fds2[0]);
			}
			if(i == 2)
			{
				close(fds1[1]);
				close(fds1[0]);
				close(fds2[1]);
			}

			//we perform our I/O redirection
			if (dup2(pairs[i][0], pairs[i][1]) < 0)
			{
				fprintf(stderr, "Error with redirection: %s\n", strerror(errno));
			}
			close(pairs[i][0]);
			//as mentioned above if we're in the second process
			//we perform a second redirection 
			if (i == 1)
			{
				if(dup2(pairs[3][0], pairs[3][1]) < 0)
				{
					fprintf(stderr, "Error with redirection: %s\n", strerror(errno));
				}
				close(pairs[3][0]);
			}

			switch(i)
			{
				case 0:
					execvp("./wordgen", (char* []){"./wordgen", argv[1], NULL});
				case 1:
					execvp("./wordsearch", (char* []){"./wordsearch", "words.txt", NULL});
				default:
					execvp("./pager", (char* []){"./pager", NULL});
			}


			//we launch the program
			//and if it fails we exit
			fprintf(stderr, "Error executing program: %s\n", strerror(errno));
			exit(-1);
		}
	}

	//we then also need to close the file descriptors
	//for the pipes in the parent process as well
	for(int i = 0; i < 2; i++)
	{
		close(fds1[i]);
		close(fds2[i]);
	}
	//once we have finished launching each of the programs
	//in the parent process we just wait and collect the statuses
	//of the child processes when they terminate

	int n = 3;
	int status;
	
	int pid;
	while(n > 0)
	{

		pid = wait(&status);

		printf("Child Process %d exited with status %d\n",  pid, WEXITSTATUS(status));
		--n;
	}

	return 0;
}

