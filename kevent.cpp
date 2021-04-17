#include "kevent.h"
#include "pcb.h"
#include "thread.h"
#include "event.h"
#include "SCHEDULE.H"

extern volatile PCB* running;


KernelEv::KernelEv(IVTNo n)
{
	asm pushf;
	asm cli;
	entry = n;
	val = 0;
	owner = (PCB*)running;
	IVTEntry::getEntry(entry)->setOwner(this);
	asm popf;
}

KernelEv::~KernelEv()
{
	asm pushf;
	asm cli;
	if(val < 0)
	{
		owner->blocked = 0;
		Scheduler::put(owner);
	}
	IVTEntry::getEntry(entry)->setOwner(0);
	asm popf;
}

void KernelEv::signal()
{
	asm pushf;
	asm cli;
	val = (val+1) < 2 ? (val+1) : val;
	if(val == 0)
	{
		owner->blocked = 0;
		Scheduler::put(owner);
	}
	asm popf;
}

void KernelEv::wait()
{
	asm pushf;
	asm cli;
	if(running == owner)
	{
		val--;
		if(val < 0)
		{
			owner->blocked = 1;
			dispatch();
		}
	}
	asm popf;
}
