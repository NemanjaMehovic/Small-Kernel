#include "inic.h"
#include "SCHEDULE.H"
#include "pcb.h"
#include "ksem.h"

volatile unsigned tsp;
volatile unsigned tss;
volatile unsigned tbp;

volatile int brojac = 20;
volatile int zahtevana_promena_konteksta = 0;
volatile int lockF = 0;
volatile PCB* running = 0;
volatile PCB* timeKiller = 0;
volatile PCB* tmpPCB = 0;
ListH* tmpH = 0;
ListS* tmpS = 0;
ListS* tmpHeadS = 0;
PCB* mainPCB = 0;


extern void tick();

void interrupt timer()
{
	if(!zahtevana_promena_konteksta)
	{
		tmpPCB = PCB::Head;
		while(tmpPCB)
		{
			if(tmpPCB->sem)
			{
				if(tmpPCB->blockTime >= 0)
					tmpPCB->blockTime--;
				if(tmpPCB->blockTime == 0)
				{
					tmpPCB->sem->removeList((PCB*)tmpPCB);
					tmpPCB->sem = 0;
					tmpPCB->blocked = 0;
					Scheduler::put((PCB*)tmpPCB);
				}
			}
			tmpPCB = tmpPCB->Next;
		}
	}
	if(!lockF)
	{
		if (!zahtevana_promena_konteksta && brojac>=0)
			brojac--;
		if (brojac == 0 || zahtevana_promena_konteksta) {
			asm {
				mov tbp, bp
				mov tsp, sp
				mov tss, ss
			}

			running->sp = tsp;
			running->ss = tss;
			running->bp = tbp;

			if(!running->done && !running->blocked && running != timeKiller)
				Scheduler::put((PCB*) running);
			do
			{
				tmpH = 0;
				tmpS = 0;
				tmpHeadS = 0;

				running = Scheduler::get();
				if(running == 0)
					running = timeKiller;

				tsp = running->sp;
				tss = running->ss;
				tbp = running->bp;

				brojac = running->time;

				asm {
					mov bp, tbp
					mov sp, tsp
					mov ss, tss
				}

				lockF = 1;
				asm sti;
				tmpHeadS = running->HeadS;
				while(tmpHeadS)
				{
					if(!running->blockedSignals[tmpHeadS->id] && !PCB::globalBlockedSignals[tmpHeadS->id])
					{
						tmpH = running->handlers[tmpHeadS->id];
						while(tmpH)
						{
							tmpH->handler();
							tmpH = tmpH->Next;
						}
						if(running->done)
						{
							delete (PCB*)running;
							running = 0;
							break;
						}
						tmpS = tmpHeadS;
						if(running->HeadS == tmpHeadS)
							running->HeadS = running->HeadS->Next;
						if(tmpHeadS->Next)
							tmpHeadS->Next->Prev = tmpHeadS->Prev;
						if(tmpHeadS->Prev)
							tmpHeadS->Prev->Next = tmpHeadS->Next;
						tmpHeadS = tmpHeadS->Next;
						delete (void*)tmpS;
					}
					else
						tmpHeadS = tmpHeadS->Next;
				}
				asm cli;
				lockF = 0;
			}while(!running);
		}
		if(!zahtevana_promena_konteksta)
		{
			asm int 60h;
			tick();
		}

		zahtevana_promena_konteksta = 0;
	}
	else
		zahtevana_promena_konteksta = 1;
}

unsigned oldTimerOFF, oldTimerSEG; // stara prekidna rutina

// postavlja novu prekidnu rutinu
void inic(){
	asm{
		cli
		push es
		push ax

		mov ax,0
		mov es,ax

		mov ax, word ptr es:0022h
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax

		mov word ptr es:0022h, seg timer
		mov word ptr es:0020h, offset timer

		mov ax, oldTimerSEG
		mov word ptr es:0182h, ax
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}

// vraca staru prekidnu rutinu
void restore(){
	asm {
		cli
		push es
		push ax

		mov ax,0
		mov es,ax


		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}

void lockFlag()
{
	lockF=1;
}
void unlockFlag()
{
	lockF=0;
	if(zahtevana_promena_konteksta)
		dispatch();
}
