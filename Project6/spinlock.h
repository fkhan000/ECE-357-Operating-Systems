#include <unistd.h>
struct spinlock
{
    volatile char *lock;
    pid_t current_holder;
    int num_locks;
};

void spin_lock(struct spinlock *l);
void spin_unlock(struct spinlock *l);

