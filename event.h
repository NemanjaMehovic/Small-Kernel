// File: event.h
#ifndef _event_h_
#define _event_h_


typedef unsigned char IVTNo;
typedef void interrupt (*interruptPointer)(...);

class KernelEv;

class Event
{
public:
	Event (IVTNo ivtNo);
	~Event ();
	void wait ();
protected:
	friend class KernelEv;
	void signal(); // can call KernelEv
private:
	KernelEv* myImpl;
};

class IVTEntry
{
	interruptPointer oldInt;
	interruptPointer newInt;
	KernelEv* owner;
	IVTNo entry;
	static IVTEntry *entrys[];
public:
	IVTEntry(IVTNo n, interruptPointer Int);
	~IVTEntry();
	void old();
	void signal();
	void setOwner(KernelEv* ev);
	void restoreInt();
	void setInt();
	static IVTEntry* getEntry(IVTNo n);
};


#define PREPAREENTRY(n, flag)\
void interrupt tmpInt##n(...); \
IVTEntry entry##n(n, tmpInt##n); \
void interrupt tmpInt##n(...)\
{\
	entry##n.signal();\
	if (flag == 1)\
		entry##n.old();\
}

#endif
