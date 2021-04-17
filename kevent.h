#ifndef KEVENT_H_
#define KEVENT_H_

#include "pcb.h"
#include "event.h"

class KernelEv
{
	PCB* owner;
	IVTNo entry;
	int val;
public:
	KernelEv(IVTNo n);
	~KernelEv();
	void wait();
	void signal();
};

#endif /* KEVENT_H_ */
