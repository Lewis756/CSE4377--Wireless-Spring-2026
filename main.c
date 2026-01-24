#include "clock.h"
#include "tm4c123gh6pm.h"
#include "uart0.h"
//initialize hardware

void intHW()
{
    initSystemClockTo40Mhz();
    initUart0();
}

int main()
{   
    intHW();
}
