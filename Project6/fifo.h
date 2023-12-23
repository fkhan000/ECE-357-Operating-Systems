#include "spinlock.h"

struct wait
{
        pid_t write_list[64];
        pid_t read_list[64];
        int r_index;
        int w_index;
};


#define MYFIFO_BUFSIZ 1000
struct fifo
{
	int size;
	unsigned long arr[MYFIFO_BUFSIZ];
	int r_idx;
	int w_idx;
	struct spinlock *l;//* l;
	struct wait w;
};


void wait_init(struct wait *w);

void fifo_init(struct fifo *f);
/* Initialize the shared memory FIFO *f including any required underlying
* initializations (such as calling cv_init). The FIFO will have a static
* fifo length of MYFIFO_BUFSIZ elements. #define this in fifo.h.
*Avalue of 1K is reasonable. In most cases, simply setting the
* entire struct fifo to 0 will suffice as initialization.
*/

void fifo_wr(struct fifo *f,unsigned long d);
/* Enqueue the data word d into the FIFO, blocking unless and until the
* FIFO has room to accept it. (i.e. block until !full)
* Wake up a reader which was waiting for the FIFO to be non-empty
*/
unsigned long fifo_rd(struct fifo *f);
/* Dequeue the next data word from the FIFO and return it. Block unless
* and until there are available words. (i.e. block until !empty)
* Wake up a writer which was waiting for the FIFO to be non-full
*/


