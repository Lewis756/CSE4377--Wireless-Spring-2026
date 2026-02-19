#ifndef WIRELESS_H_
#define WIRELESS_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    dc = 2,
    sine = 0,
    bpsk = 1,
    qpsk = 2,
    epsk  = 3,
    qam = 4
} MODE;

extern uint16_t rawI;
extern uint16_t rawQ;
extern float dcI;
extern float dcQ;
extern uint8_t mode;
extern uint16_t data;

void sendDacI(float v);
void sendDacQ(float v);
void writeDacAB(uint16_t rawI, uint16_t rawQ);
void ISR();
void bitSymbol(uint8_t size);


#endif /* WIRELESS_H_ */
