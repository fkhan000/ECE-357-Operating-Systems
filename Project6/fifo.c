#include "fifo.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

void wait_init(struct wait* w)
{
	w ->w_index = 0;
	w ->r_index = 0;
}

void fifo_init(struct fifo *f)
{
	f -> size = 0;
	f -> r_idx = 0;
	f -> w_idx = 0;
	wait_init( &(f -> w));
	
	f -> l -> lock = mmap(NULL, sizeof *(f -> l ->lock), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*(f -> l -> lock) = 0;
	f -> l ->current_holder = -1;
	f -> l ->num_locks = 0;
}


void fifo_wr(struct fifo *f,unsigned long d)
{
	
	while(f -> size >= MYFIFO_BUFSIZ)
	{
		sigset_t mask, oldmask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		
		spin_lock((f-> l));
		f->w.write_list[f->w.w_index++] = getpid();
		
		spin_unlock((f->l));		
		
		//suspend SIGUSR1, in test file would need a signal handler
		//for SIGUSER1
		sigsuspend(&oldmask);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}

	spin_lock((f->l));
	f -> arr[f->w_idx++] = d;
	f ->w_idx %= MYFIFO_BUFSIZ;
	f -> size ++;

	for(int i = 0; i < f->w.r_index; i++)
	{
		kill(f->w.read_list[i], SIGUSR1);
		f -> w.read_list[i] = -1;
	}
	f -> w.r_index = 0;
	spin_unlock((f->l));
}

unsigned long fifo_rd(struct fifo *f)

{
	unsigned long d;

        while(f -> size <= 0)
        {
                sigset_t mask, oldmask;
                sigemptyset(&mask);
                sigaddset(&mask, SIGUSR1);
                sigprocmask(SIG_BLOCK, &mask, &oldmask);

		spin_lock((f-> l));
                f->w.read_list[f->w.r_index++] = getpid();
		
		spin_unlock((f->l));

                sigsuspend(&oldmask);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
        }
	spin_lock((f->l));
        d = f -> arr[f->r_idx++];
	f -> r_idx %= MYFIFO_BUFSIZ;
        f -> size--;
        for(int i = 0; i < f->w.w_index; i++)
        {
                kill(f->w.write_list[i], SIGUSR1);
		f ->w.write_list[i] = -1;
        }
	f -> w.w_index = 0;
	spin_unlock((f->l));
	return d;
}
