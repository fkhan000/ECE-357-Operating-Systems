#include "tas.h"
#include "spinlock.h"
#include <unistd.h>
#include <stdio.h>

void spin_lock(struct spinlock *l)
{
	while(tas(l -> lock) != 0);
    	l -> current_holder =  getpid();
    	l -> num_locks++;

}
void spin_unlock(struct spinlock *l)
{
    	l -> current_holder = -1;
    	*(l -> lock) = 0;
}
