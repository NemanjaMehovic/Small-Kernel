#ifndef INIC_H_
#define INIC_H_

#define lock asm cli

#define unlock asm sti

void interrupt timer();
void inic();
void restore();
void lockFlag();
void unlockFlag();

#endif /* INIC_H_ */
