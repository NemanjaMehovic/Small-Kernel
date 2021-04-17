#include "pcb.h"
#include "SCHEDULE.H"
#include "thread.h"
#include <dos.h>


extern volatile PCB* running;
extern volatile int lockF;

volatile ID PCB::id = 0;
volatile PCB* PCB::Head = 0;

//zad 2
volatile SignalId PCB::globalBlockedSignals[15];

//konstruktor samo za main nit
PCB::PCB()
{
	asm pushf;
	asm cli;
	myThread = 0;
	Next =0;
	done = 0;
	started = 1;
	st = 0;
	myID = PCB::id++;
	PCB::Head = this;
	time = defaultTimeSlice;
	blockTime = -1;
	sem = 0;
	blocked = 0;
	blockingPCB = 0;
	//zad 2
	HeadS = 0;
	ownerPCB = 0;
	for(int i = 0;i<15;i++)
	{
		blockedSignals[i] = 0;
		handlers[i] = 0;
	}
	registerHandler(0,PCB::killSignal);
	asm popf;
}

PCB::PCB(StackSize stackSize, Time timeSlice, Thread* thread)
{
	asm pushf;
	asm cli;
	myID=PCB::id++;
	if(stackSize > (16*defaultStackSize))
			stackSize = 16*defaultStackSize;
	Next = PCB::Head;
	PCB::Head = this;
	myThread = thread;
	st = new unsigned[stackSize];
	st[stackSize-1] = 0x200;
#ifndef BCC_BLOCK_IGNORE
	st[stackSize-2] = FP_SEG(PCB::wrapper);
	st[stackSize-3] = FP_OFF(PCB::wrapper);
	bp = FP_OFF(st+stackSize-12);
	sp = FP_OFF(st+stackSize-12);
	ss = FP_SEG(st+stackSize-12);
#endif
	done = 0;
	started = 0;
	time = timeSlice;
	blockTime = -1;
	sem = 0;
	blocked = 0;
	blockingPCB = 0;
	//zad 2
	ownerPCB = running;
	HeadS = 0;
	for(int i = 0;i<15;i++)
	{
		blockedSignals[i] = ownerPCB->blockedSignals[i];
		handlers[i] = 0;
		ListH* tmp = ownerPCB->handlers[i];
		while(tmp)
		{
			registerHandler(i,tmp->handler);
			tmp = tmp->Next;
		}
	}
	asm popf;
}

PCB::~PCB()
{
	asm pushf;
	asm cli;
	volatile PCB* tmp = PCB::Head;
	if(tmp == this)
	{
		PCB::Head = tmp->Next;
		tmp = 0;
	}
	while(tmp != 0 && tmp->Next != this)
		tmp = tmp->Next;
	if(tmp != 0 && tmp->Next != 0)
		tmp->Next = tmp->Next->Next;
	freeBlockedBy();
	//zad 2
	for(int i = 0; i < 15; i++)
		unregisterAllHandlers(i);
	while(HeadS)
	{
		ListS* tmpS = HeadS;
		HeadS = HeadS->Next;
		delete (void*)tmpS;
	}
	delete[] (void*)st;
	asm popf;
}

void PCB::start()
{
	asm pushf;
	asm cli;
	if(!done && !started)
	{
		started = 1;
		Scheduler::put(this);
	}
	asm popf;
}

void PCB::exit()
{
	asm pushf;
	asm cli;
	done = 1;
	started=0;
	freeBlockedBy();
	//zad 2
	if(ownerPCB != 0)
		ownerPCB->signal(1);
	if(!blockedSignals[2] && !PCB::globalBlockedSignals[2])
	{
		ListH *tmp = handlers[2];
		while(tmp)
		{
			tmp->handler();
			tmp = tmp->Next;
		}
	}
	asm popf;
}

void PCB::waitFinish()
{
	asm pushf;
	asm cli;
	if(!done && started)
	{
		running->blocked = 1;
		running->blockingPCB = this;
		dispatch();
	}
	asm popf;
}

void PCB::freeBlockedBy()
{
	PCB* tmpPCB = (PCB*)PCB::Head;
	while(tmpPCB)
	{
		if(tmpPCB->blocked && tmpPCB->blockingPCB == this)
		{
			tmpPCB->blocked = 0;
			tmpPCB->blockingPCB = 0;
			Scheduler::put(tmpPCB);
		}
		tmpPCB = (PCB*)tmpPCB->Next;
	}
}

Thread* PCB::GetById(ID find)
{
	Thread *tmpT = 0;
	asm pushf;
	asm cli;
	volatile PCB* tmpP = PCB::Head;
	while(tmpP->myID != find && tmpP != 0)
		tmpP = tmpP->Next;
	if(tmpP != 0)
		tmpT = tmpP->myThread;
	asm popf;
	return tmpT;
}

void PCB::wrapper()
{
	running->myThread->run();
	running->exit();
	dispatch();
}
//zad 2
void PCB::blockSignal(SignalId signal)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15)
		this->blockedSignals[signal] = 1;
	asm popf;
}
void PCB::blockSignalGlobally(SignalId signal)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15)
		PCB::globalBlockedSignals[signal] = 1;
	asm popf;
}
void PCB::unblockSignal(SignalId signal)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15)
		this->blockedSignals[signal] = 0;
	asm popf;
}
void PCB::unblockSignalGlobally(SignalId signal)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15)
		PCB::globalBlockedSignals[signal] = 0;
	asm popf;
}

void PCB::signal(SignalId signal)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15 && handlers[signal])
	{
		ListS *tmp = new ListS();
		tmp->Next = 0;
		tmp->id = signal;
		if(!HeadS)
		{
			tmp->Prev = 0;
			HeadS = tmp;
		}
		else
		{
			ListS *i = HeadS;
			while(i->Next)
				i = i->Next;
			tmp->Prev = i;
			i->Next = tmp;
		}
	}
	asm popf;
}

void PCB::registerHandler(SignalId signal, SignalHandler handler)
{
	asm pushf;
	asm cli;
	if(signal >= 0 && signal < 15)
	{
		ListH *tmp = new ListH();
		tmp->Next = 0;
		tmp->handler = handler;
		if(!handlers[signal])
		{
			tmp->Prev = 0;
			handlers[signal] = tmp;
		}
		else
		{
			ListH *i = handlers[signal];
			while(i->Next)
				i = i->Next;
			tmp->Prev = i;
			i->Next = tmp;
		}
	}
	asm popf;
}

void PCB::unregisterAllHandlers(SignalId id)
{
	asm pushf;
	asm cli;
	if(id >= 0 && id < 15)
		while(handlers[id])
		{
			ListH *tmp = handlers[id];
			handlers[id] = handlers[id]->Next;
			delete (void*)tmp;
		}
	asm popf;
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2)
{
	asm pushf;
	asm cli;
	if(id >= 0 && id < 15 && hand1 != hand2)
	{
		ListH *val1,*val2,*tmp;
		val1 = val2 = 0;
		tmp = handlers[id];
		while(tmp)
		{
			if(tmp->handler == hand1)
				val1 = tmp;
			else if(tmp->handler == hand2)
				val2 = tmp;
			tmp = tmp->Next;
		}
		if(val1 != 0 && val2 != 0)
		{
			tmp = val1->Prev;
			val1->Prev = val2->Prev;
			val2->Prev = tmp;
			if(val1->Prev != 0)
				val1->Prev->Next = val1;
			if(val2->Prev != 0)
				val2->Prev->Next = val2;
			tmp = val1->Next;
			val1->Next = val2->Next;
			val2->Next = tmp;
			if(val1->Next != 0)
				val1->Next->Prev = val1;
			if(val2->Next != 0)
				val2->Next->Prev = val2;
			if(handlers[id] == val1)
				handlers[id] = val2;
			else if(handlers[id] == val2)
				handlers[id] = val1;
		}
	}
	asm popf;
}

void PCB::killSignal()
{
	running->exit();
}
