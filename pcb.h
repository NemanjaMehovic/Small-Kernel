#ifndef PCB_H_
#define PCB_H_
#include "thread.h"
#include "ksem.h"

typedef volatile struct signalList
{
	volatile SignalId id;
	volatile signalList* Next;
	volatile signalList* Prev;
}ListS;

typedef volatile struct handlerList
{
	volatile SignalHandler handler;
	volatile handlerList* Next;
	volatile handlerList* Prev;
}ListH;

class PCB
{
	volatile static ID id;
	volatile unsigned *st;
	void freeBlockedBy();
public:
	volatile unsigned sp;
	volatile unsigned ss;
	volatile unsigned bp;
	volatile unsigned done;
	volatile unsigned started;
	volatile int time;

	ID myID;
	Thread* myThread;
	volatile PCB* blockingPCB;
	volatile PCB* Next;
	volatile int blockTime;
	volatile KernelSem* sem;
	volatile int blocked;
	volatile static PCB *Head;

	PCB();
	PCB(StackSize stackSize, Time timeSlice, Thread* thread);
	~PCB();

	void exit();
	void start();
	void waitFinish();

	static Thread* GetById(ID find);
	static void wrapper();

	//zad 2

	volatile PCB* ownerPCB;
	ListS *HeadS;
	ListH *handlers[15];
	volatile SignalId blockedSignals[15];
	volatile static SignalId globalBlockedSignals[15];

	void signal(SignalId signal);
	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId id);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);

	void blockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	void unblockSignal(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);

	static void killSignal();
};



#endif /* PCB_H_ */
