#include "ksem.h"
#include "pcb.h"
#include "thread.h"
#include "SCHEDULE.H"

extern volatile PCB* running;


KernelSem::KernelSem(int init)
{
	asm pushf;
	asm cli;
	val = init >= 0 ? init : 0;
	Head = Tail = 0;
	asm popf;
}

KernelSem::~KernelSem()
{
	asm pushf;
	asm cli;
	while(Head)
	{
		List* tmp = Head;
		Head = Head->Next;
		PCB* tmpPCB = (PCB*)tmp->Element;
		tmpPCB->blockTime = -1;
		tmpPCB->sem = 0;
		tmpPCB->blocked = 0;
		Scheduler::put(tmpPCB);
		delete (void*)tmp;
	}
	Tail = 0;
	val = 0;
	asm popf;
}

int KernelSem::wait(Time maxTimeToWait)
{
	asm pushf;
	asm cli;
	val--;
	running->blockTime = -1;
	if(val < 0)
	{
		running->sem = this;
		running->blocked = 1;
		running->blockTime = maxTimeToWait;
		addList((PCB*)running);
		dispatch();
	}
	asm popf;
	return running->blockTime != 0 ? 1 : 0;
}

int KernelSem::signal(int n)
{
	asm pushf;
	asm cli;
	if(n < 0)
	{
		asm popf;
		return n;
	}
	int retVal = 0;
	int num = n == 0 ? 1 : n;
	while(num > 0)
	{
		num--;
		val++;
		if(Head)
		{
			retVal++;
			PCB* tmpPCB = removeListLast();
			tmpPCB->sem = 0;
			tmpPCB->blocked = 0;
			Scheduler::put(tmpPCB);
		}
	}
	retVal = n == 0 ? 0 : retVal;
	asm popf;
	return retVal;
}

void KernelSem::addList(PCB* element)
{
	if(!Head)
	{
		Head = new List();
		Head->Next = Head->Prev = 0;
		Head->Element = element;
		Tail = Head;
	}
	else
	{
		List* tmp = new List();
		tmp->Next = Head;
		tmp->Prev = 0;
		Head->Prev = tmp;
		tmp->Element = element;
		Head = tmp;
	}
}

PCB* KernelSem::removeListLast()
{
	volatile PCB* tmp = 0;
	if(Tail != 0)
	{
		tmp = Tail->Element;
		List* tmpL = Tail;
		Tail = Tail->Prev;
		delete (void*)tmpL;
		if(Tail != 0)
			Tail->Next = 0;
		else
			Head = 0;
	}
	return (PCB*)tmp;
}

void KernelSem::removeList(PCB* element)
{
	List* tmp = Head;
	while(tmp && tmp->Element != element)
		tmp = tmp->Next;
	if(tmp == Head && tmp == Tail)
		Head = Tail = 0;
	else if(tmp == Head)
	{
		tmp->Next->Prev = 0;
		Head = tmp->Next;
	}
	else if(tmp == Tail)
	{
		tmp->Prev->Next = 0;
		Tail = tmp->Prev;
	}
	else
	{
		tmp->Next->Prev = tmp->Prev;
		tmp->Prev->Next = tmp->Next;
	}
	val++;
	delete (void*)tmp;
}
