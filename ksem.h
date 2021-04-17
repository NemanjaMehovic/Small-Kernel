#ifndef KSEM_H_
#define KSEM_H_

#include"semaphor.h"
class PCB;

typedef volatile struct semList
{
	volatile PCB* Element;
	volatile semList* Next;
	volatile semList* Prev;
}List;

class KernelSem
{
public:
	volatile int val;
	List* Head;
	List* Tail;

	KernelSem(int init=1);
	~KernelSem();

	int wait(Time maxTimeToWait);
	int signal(int n);
	void addList(PCB* element);
	PCB* removeListLast();
	void removeList(PCB* element);
};



#endif /* KSEM_H_ */
