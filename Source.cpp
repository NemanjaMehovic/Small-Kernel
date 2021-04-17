#include "pcb.h"
#include "inic.h"
#include "thread.h"

extern volatile PCB* running;
extern volatile PCB* timeKiller;
extern PCB* mainPCB;
extern int userMain (int argc, char* argv[]);
int retVal;
int argc2;
char** argv2;

class waitThread:public Thread
{
public:
	waitThread(int stackSize = 512, int timeSlice = 1):Thread(stackSize, timeSlice)
	{

	}
	virtual void run()
	{
		while(1);
	}
};

class userMainThread:public Thread
{
public:
	userMainThread(int stackSize = defaultStackSize*2, int timeSlice = 0):Thread(stackSize, timeSlice)
	{

	}
	virtual void run()
	{
		retVal = userMain(argc2, argv2);
	}
};

int main(int argc, char* argv[])
{
	mainPCB = new PCB();
	running = mainPCB;
	Thread* tmp = new waitThread();
	timeKiller = PCB::Head;
	Thread* tmp2 = new userMainThread();
	argc2 = argc;
	argv2 = argv;

	inic();

	tmp2->start();

	tmp2->waitToComplete();

	restore();


	delete tmp;
	delete tmp2;
	return retVal;
}

