#include "thread.h"
#include "pcb.h"
#include "inic.h"

extern volatile int zahtevana_promena_konteksta;
extern volatile PCB* running;

Thread::Thread(StackSize stackSize, Time timeSlice)
{
	myPCB = new PCB(stackSize, timeSlice, this);
}

Thread::~Thread()
{
	delete myPCB;
}

void Thread::start()
{
	myPCB->start();
}

void Thread::waitToComplete()
{
	myPCB->waitFinish();
}

ID Thread::getId()
{
	return myPCB->myID;
}

ID Thread::getRunningId()
{
	return running->myID;
}

Thread* Thread::getThreadById(ID id)
{
	return PCB::GetById(id);
}

void dispatch()
{
	asm pushf;
	asm cli;
	zahtevana_promena_konteksta = 1;
	timer();
	asm popf;
}
//zad 2
void Thread::blockSignal(SignalId signal)
{
	myPCB->blockSignal(signal);
}

void Thread::blockSignalGlobally(SignalId signal)
{
	PCB::blockSignalGlobally(signal);
}

void Thread::unblockSignal(SignalId signal)
{
	myPCB->unblockSignal(signal);
}

void Thread::unblockSignalGlobally(SignalId signal)
{
	PCB::unblockSignalGlobally(signal);
}

void Thread::signal(SignalId signal)
{
	myPCB->signal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler)
{
	myPCB->registerHandler(signal,handler);
}

void Thread::unregisterAllHandlers(SignalId id)
{
	myPCB->unregisterAllHandlers(id);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2)
{
	myPCB->swap(id,hand1,hand2);
}
