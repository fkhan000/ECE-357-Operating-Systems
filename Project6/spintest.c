#include "spinlock.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/wait.h>

static int *glob_var; 
static struct spinlock* l;

void spawn(int num_child, int num_iter)
{

	pid_t pids[num_child];
	for(int i = 0; i < num_child; i++)
	{
		if((pids[i] = fork()) < 0)
		{
			perror("Error Executing Fork, Aborting\n");
			munmap(glob_var, sizeof *glob_var);
			exit(1);
		}
		
		else if (pids[i] == 0)
		{
			for(int j = 0; j < num_iter; j++)
			{
				spin_lock(l);
				++(*glob_var);
				spin_unlock(l);
			}
			exit(0);
		}
	}
}

int main(int argc, char* argv[])
{
	
	if ((glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) < 0)
		{
			perror("Unable To Create New Map, Aborting\n");
			exit(-1);
		}

	if ((l = mmap(NULL, sizeof *l, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) < 0)
                {
                        perror("Unable To Create New Map, Aborting\n");
                        exit(-1);
                }
	
	l -> lock = malloc(sizeof (l -> lock));
	*(l -> lock) = 0;
	l -> current_holder = -1;
	l -> num_locks = 0;
	*glob_var = 0;

	spawn(atoi(argv[1]), atoi(argv[2]));
	
	int status;
	pid_t pid;
	int n = atoi(argv[1]);
	

	while(n > 0)
	{
		pid = wait(&status);
		printf("Child with PID %ld exited with status %d\n", (long) pid, status);
		printf("Current value pointed to by glob_var: %d\n", *glob_var);
		//printf("%d\n", l -> num_locks);
		--n;
	}
	munmap(glob_var, sizeof *glob_var);
	return 0;
}



