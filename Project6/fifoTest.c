#include "fifo.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

static struct fifo *f;
static struct spinlock *l;

void wakeup(int sig)
{
	return;
}

unsigned long stream_read(unsigned long prev, unsigned long current)
{
	/*
	if ((prev != 1) && (prev >= current))
	{
		printf("%lu %lu\n", prev, current);
	}
	*/
	if ((prev == 1) | (prev >= current))
	{
		return 1;
	}
	return current;
}
void acid_test(int num_writers, int num_iter)
{
	pid_t writers[num_writers];
	pid_t reader;
	unsigned long d;

	int index = 0;
	for(int i = 0; i < num_writers; i++)
	{
		if( (writers[i] = fork()) < 0)
		{
			perror("Error Executing Fork, Aborting\n");
                        exit(1);
		}
		else if(writers[i] == 0)
		{
			for(int j = 0; j < num_iter; j++)
			{
				
				d = (i+1)*100000 + j;
				fifo_wr(f, d);
			}
			exit(0);
		}
	}
	

	int num_running = num_writers;
	int status;
	pid_t proc;
	index = 0;
	unsigned long streams[num_writers];
	unsigned long received[num_writers*num_iter];
	memset(streams, 0, sizeof(streams));


	while(num_running != 0)
	{
		for(int i =0; i < num_writers; i++)
		{
			if ( (proc = waitpid(writers[i], &status, WNOHANG)) > 0)
			{
				printf("Writer %d completed\n", i);
				num_running --;
			}
		}

		
		
		if(num_running != 0)
		{
			d = fifo_rd(f);
			streams[ ((int) (d/100000)) - 1] = stream_read(streams[((int) (d/100000))- 1], d);
			received[index ++] = d;
		}
		else
		{
			while(f -> size > 0)
			{
				d = fifo_rd(f);

				//printf( "%lu, %lu\n", d, streams[((int) (d/100000)) - 1]);
                        	streams[((int) (d/100000)) - 1] = stream_read(streams[((int) (d/100000)) - 1], d);
				received[index ++] = d;
			}
		}
		
	}
	
	printf("Reader 0 completed\n");
	if (index != num_writers*num_iter)
	{
		printf("Uh-oh. We only received %d entries when we should have gotten %d entries\n", index, num_writers*num_iter);
		return;
	}

	printf("Number of entries recieved (%d) matches number of entries written to FIFO (%d)!\n", index, num_writers*num_iter);

	
	for(int i = 0; i < num_writers*num_iter; i++)
	{
		for(int j = i + 1; j < num_writers*num_iter; j++)
		{
			if(received[i] == received[j])
			{
				printf("Uh-oh. There was a duplicate entry found in the FIFO!\n");
				return;
			}
		}	
	}
	
	int passed;
	for(int i = 0; i < num_writers*num_iter; i++)
	{
		passed = 0;
		for(int j = 0; j <num_writers*num_iter; j++)
		{
			if(received[i] == ((i + 1)*100000 + j))
			{
				passed = 1;
				break;
			}
		}
		if(passed == 0)
		{
			printf("Uh oh. Corrupted entry found while reading from FIFO!\n");
		}
	}

	for(int i = 0; i < num_writers; i++)
	{
		if (streams[i] == 1)
		{
                        printf("Uh-oh data in FIFO received in incorrect order!\n");
                        return;
                }
        }
        printf("All data in FIFO received in correct order!\n");
	
	
}
int main(int argc, char * argv[])
{
	signal(SIGUSR1, wakeup);

	printf("Beginning test with %s writers, %s items each\n", argv[1], argv[2]);

	if ((f = mmap(NULL, sizeof *f, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) < 0)
	{
		 perror("Unable To Create New Map, Aborting\n");
                 exit(-1);
	}
	
	if ((l = mmap(NULL, sizeof *l, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) < 0)
        {
                 perror("Unable To Create New Map, Aborting\n");
                 exit(-1);
        }

	f -> l = l;

	fifo_init(f);
	
	acid_test(atoi(argv[1]), atoi(argv[2]));
	return 0;
}
