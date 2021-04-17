#include "event.h"
#include "kevent.h"
#include <dos.h>
#include <IOSTREAM.H>

IVTEntry* IVTEntry::entrys[256];

Event::Event(IVTNo ivtNo)
{
	myImpl = new KernelEv(ivtNo);
}

Event::~Event()
{
	delete myImpl;
}

void Event::wait()
{
	myImpl->wait();
}

void Event::signal()
{
	myImpl->signal();
}

IVTEntry* IVTEntry::getEntry(IVTNo n)
{
	return entrys[n];
}

IVTEntry::IVTEntry(IVTNo n, interruptPointer Int)
{
	asm pushf;
	asm cli;
	newInt = Int;
	entry = n;
	entrys[n] = this;
#ifndef BCC_BLOCK_IGNORE
	oldInt = getvect(n);
#endif
	setInt();
	asm popf;
}

IVTEntry::~IVTEntry()
{
	asm pushf;
	asm cli;
	entrys[entry] = 0;
	restoreInt();
	asm popf;
}


void IVTEntry::old()
{
	asm pushf;
	asm cli;
	oldInt();
	asm popf;
}

void IVTEntry::signal()
{
	asm pushf;
	asm cli;
	if(owner)
		owner->signal();
	asm popf;
}

void IVTEntry::setOwner(KernelEv* o)
{
	asm pushf;
	asm cli;
	owner = o;
	asm popf;
}

void IVTEntry::restoreInt()
{
	asm pushf;
	asm cli;
#ifndef BCC_BLOCK_IGNORE
	setvect(entry, oldInt);
#endif
	asm popf;
}

void IVTEntry::setInt()
{
	asm pushf;
	asm cli;
#ifndef BCC_BLOCK_IGNORE
	setvect(entry, newInt);
#endif
	asm popf;
}
