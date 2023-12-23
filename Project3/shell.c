#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>

//this function performs our file redirections
//it takes in redirects which is an array of pointers
//to the names of the files to redirect stdin, stdout, and stderr to
//if redirects[i] is an empty string then we don't perform redirection
void redirect(char* redirects[])
{
	//we store the file descriptors and the possible modes
	//we can open these files in
	int fd[3];

	int modes[5] = {O_RDONLY, O_WRONLY | O_CREAT | O_TRUNC, O_WRONLY | O_CREAT | O_TRUNC, 
		O_WRONLY | O_CREAT | O_APPEND, O_WRONLY | O_CREAT | O_APPEND};

	//these are the corresponding original file descriptors for stdin, stdout, and stderr
	int orig[5] = {0, 1, 2, 1, 2};

	//since we only have 3 files that can be redirected, we can have at most 3 redirections
	//the index variable lets us store the file descriptor of the redirected file in
	//its corresponding slot
	int index = 0;

	//we iterate through our list of filenames
	for(int i = 0; i < 5; i++)
	{
		//if redirects[i] is just an empty string that means we don't perform
		//that type of redirection
		if (strcmp(redirects[i] ,"") == 0)
		{
			//and we move on
			continue;
		}

		//we try to open the file with the corresponding mode stored in the modes array
		if ((fd[index] = open(redirects[i], modes[i])) < 0)
		{
			//if we are unable to do so we let the user know
			fprintf(stderr, "Can't open file %s : %s\n", redirects[i], strerror(errno));
			//and then exit with a 1 exist status
			exit(1);
		}
		else
		{
			//else we redirect all actions performed to the corresponding "standard" file descriptor
			//to this file
			dup2(fd[index], orig[i]);
			index ++;
		}
	}
	
}

//this function parses through the command string read from the shell/bash script
//and separates the arguments used by the command and the arguments used 
//for file direction. After that, it creates a new child process where it executes
//the command

//returns the exit status of the child process spawned to execute the command
int execute(char* str)
{
	//we store the list of arguments for the command here
	char *arg_list[50];

	//the arguments are separated by spaces so we can split the command string
	//by spaces to get the arguments
	char* delim = " ";
	char *command;
	char *token;
	int index = 0;
	int pid;

	token = strtok(str, delim);
	int status;
	pid_t cpid;
	struct rusage usage;
	//and the redirect arguments here
	char* redirects[5] = {"", "", "", "", ""};



	//we'll read in the command
	//and if we find

	//< in beginning, > in beginning,
	//2> in beginning, >> in beginning,
	//or 2>> beginning we store the filenames in redirects

	while(token != NULL)
	{
		//>> means redirect stdout
		if ((strlen(token) > 2) & (strncmp(">>", token, 2) == 0))
		{
			//we read in the address of the second position of the argument string
			//so that only store the actual filename
			redirects[3] = &token[2];
		}
		//> means redirect stdout
		else if ((strlen(token) > 2) & (strncmp("2>", token, 2) == 0))
		{
			redirects[2] = &token[2];
		}
		//means redirect stderr
		else if ((strlen(token) > 1) & (strncmp(">", token, 1) == 0))
		{
			redirects[1] = &token[1];
		}
		//means redirect stdin
		else if ((strlen(token) > 1) & (strncmp("<", token, 1) == 0))
		{
			redirects[0] = &token[1];	
		}

		//means redirect stderr
		else if ((strlen(token) > 3) & (strncmp("2>>", token, 3) == 0))
		{
			redirects[0] = &token[3];
		}
		
		else
		{
			//if we got here then that means the argument was an arg for the command
			//and not a redirection argument so we store it in arg_list
			arg_list[index] = token;
			index ++;
		}
		
		//and we move on to the next argument
		token = strtok(NULL, delim);
	}

	//before passing arg_list to execvp, we have to terminate it with a NULL pointer
	arg_list[index] = NULL;

	//we start the clock (this measures real time)
	clock_t start = clock();

	//we fork and create a child process
	switch (pid = fork())
	{
		//if pid is -1, we weren't able to fork and we let the user know
		case -1:
				fprintf(stderr, "Fork Failed %s", strerror(errno));return -1;

		//if pid is 0 then we're in the child process
		case 0:
				//so we perform our file redirection
				redirect(redirects);
				//and execute the command
				execvp(arg_list[0], arg_list);
				//if we got here then we weren't able to execute the command because
				//execvp wipes out our program file and replaces it with the program file
				//for the command. We let the user know and then exit so that we terminate the 
				//child process
				fprintf(stderr, "Execution of Command %s Failed : %s\n", arg_list[0], strerror(errno));
				exit(127);
		default:
				//and if we're here then that means we're in the parent process
				//we wait for the child process to finish
				//once it does, we're going to save the status of the child process
				//and the user and system time taken to execute the process in usage
				if ((cpid = wait3(&status, 0, &usage)) < 0)
				{      
					//if cpid < 0 then the wait failed and we let the user know
					fprintf(stderr, "Wait Failed : %s\n", strerror(errno));
				}

				else
				{
					//if the status is nonzero then the command encountered either an error
					//or an interrupt
					if(status != 0)
					{
						//if an interrupt occured
						if (WIFSIGNALED(status))
						{
							fprintf(stderr, "Child process %d, exited with signal %d\n", 
								cpid, WTERMSIG(status));

							status = WTERMSIG(status);
						}
						//or if an error occured while executing the command
						else
						{
							//we let the user know
							fprintf(stderr, "Child process %d exited with return val %d\n",
								cpid, WEXITSTATUS(status));

							//and keep the exit status of the child process
							status = WEXITSTATUS(status);
						}
					}

					else
					{
						//if status is 0 then we successfully executed the command
						fprintf(stderr, "Child process %d exited normally\n", cpid);
					}
					//we report the real, user, and system time taken to execute the command
					fprintf(stderr, "Real: %f, User: %ld.%.6ds,  Sys: %ld.%.6ds\n",
						((double) (clock() - start)/CLOCKS_PER_SEC),
						usage.ru_utime.tv_sec,
						usage.ru_utime.tv_usec,
						usage.ru_stime.tv_sec,
						usage.ru_stime.tv_usec
						);
				}
	}
	//and return the exit status
	return status;
}

//this function implements our shell by reading either from
//standard input or from a given bash script file
int main(int argc, char* argv[])
{
	char str[1000];
	char str_copy[1000];
	char dir[1024];
	char *token;
	char* delim = " ";
	int status = 0;
	int lines = 0;
	FILE *fp = stdin;

	//if the user gave a bash script to read commands from
	if (argc == 2)
	{
		//we check to see if it can be opened
		if ( (fp = fopen(argv[1], "r")) == NULL) 
		{
			//and if not, we let the user know and exit the program with exit status 1
			fprintf(stderr, "Unable to open given script file %s", strerror(errno));
			exit(1);
		}
	}
	//while we haven't reached the end of the file (for the shell that would be when the user hits
	//control D)
	while(fgets(str, 1000, fp) != NULL)
	{
		//we read in the next line

		lines = ftell(fp);
		//and then strip off the new line character at the end of our string
		str[strcspn(str, "\n")] = 0;

		//since we're going to tokenize the string, we're going to save a copy of it that
		//we can pass in to the execute function
		strcpy(str_copy, str);

		//if the line is blank or is a comment
		if ((strlen(str) == 0)  | (str[0] == '#'))
		{
			//we skip
			continue;
		}
		//we get in the beginning of the command
		token = strtok(str, delim);

		//the commands cd, pwd, and exit are built in commands for the shell
		//and aren't passed into the execute function

		//if the command is the cd command
		if (strcmp(token, "cd") == 0)
		{
			//we get the next arugment
			token = strtok(NULL, delim);

			//if there was no additional argument and the command was just cd
			if (token == NULL)
			{
				//we cd into the path stored in the environment variable HOME
				token = getenv("HOME");
			}
			//we attempt to cd into the directory
			if (chdir(token) < 0)
			{
				//if unsuccessful we let the user know
				fprintf(stderr, "Unable to cd into given directory %s:%s\n", token, strerror(errno));
			}
		}

		//if the command is pwd
		else if (strcmp(token, "pwd") == 0)
		{
			//we display the current path
			if (getcwd(dir, sizeof(dir)) == NULL)
			{
				fprintf(stderr, "Unable to read current path: %s\n", strerror(errno));
			}
			printf("%s\n", dir);
		}
		//if the command is exit
		else if (strcmp(token, "exit") == 0)
		{
			//we get the next argument
			token = strtok(NULL, delim);

			if (token == NULL)
			{
				//if there was no next argument we exit the shell
				//with the status given by the last spawned child process
				exit(status);
			}
			else
			{
				//else we exit with the token given by the user
				exit(atoi(token));
			}
		}

		else
		{
			//else we use the execvp system call to execute the command
			status = execute(str_copy);
		}

	}
	
	//if we had opened a bash script
	if (argc == 2)
	{
		//we close it
		fclose(fp);
	}
	//and return the status
	return status;
}