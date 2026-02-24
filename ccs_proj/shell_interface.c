// shell_interface.c  (YOUR FILE, with FILTER added)
// Only changes:
//  1) add prototypes for filter functions
//  2) add FILTER / NOFILTER / FSTATUS commands (or keep your FILTER and make it toggle)
//  3) when filter turns on: call setFilterStatus() and setSampleRate(FS*4) if you have it
//     (If you don’t have a setSampleRate/timer function in this project, ignore that part)

#include "tm4c123gh6pm.h"
#include "uart0.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "wait.h"
#include <math.h>
#include "spi1.h"
#include "gpio.h"
#include "interface_functions.h"
#include "shell_interface.h"
#include "wireless.h"

extern uint32_t delta_phase;
extern uint16_t bpskSymbol;
extern uint16_t ReadConstellation;

uint8_t input[256];

// ---------------- ADD: prototypes (match wireless.c) ----------------
void setFilterStatus(void);          // toggles filter (from the file I gave you)
extern bool filter;                  // filter flag lives in wireless.c
// OPTIONAL: if you have a timer/sample-rate function in this project, uncomment and use it
// void setSampleRate(uint32_t rate);

void shell(void)
{
    USER_DATA data;

    sine_values();

    int i;
    for(i = 0; i < 256; i++)
    {
        input[i] = rand();
    }
    setTransmitBuffer(input, 256);

    while (true)
    {
        getsUart0(&data);
        parseFields(&data);
        bool valid = false;

        if (isCommand(&data, "RAW", 2))
        {
            valid = true;

            char channel = getFieldChar(&data, 1);
            int32_t value = getFieldInteger(&data, 2);

            if (channel >= 'a' && channel <= 'z')
                channel -= 32;

            if ((channel != 'I' && channel != 'Q') || (value < 0 || value > 4095))
            {
                valid = false;
            }
            else
            {
                if (channel == 'I')
                {
                    rawI = value;
                    putsUart0("\r\n RAW I set \r\n");
                    writeDacAB(rawI, rawQ);
                }
                else
                {
                    rawQ = value;
                    putsUart0("\r\n RAW Q set \r\n");
                    writeDacAB(rawI, rawQ);
                }
            }
        }

        if (isCommand(&data, "DC", 2))
        {
            valid = true;
            float temp = 0;
            char channel = getFieldChar(&data, 1);
            int32_t value = getFieldInteger(&data, 2);

            if (channel >= 'a' && channel <= 'z')
                channel -= 32;

            if ((channel != 'I' && channel != 'Q') || (value < -500 || value > 500))
            {
                valid = false;
            }
            else
            {
                temp = mvToV(value);
                if (channel == 'I')
                {
                    dcI = temp;
                    rawI = voltageToDacCode(dcI);
                    putsUart0("\r\n DC I set \r\n");
                }
                else
                {
                    dcQ = temp;
                    rawQ = voltageToDacCode(dcQ);
                    putsUart0("\r\n DC Q set \r\n");
                }
                writeDacAB(rawI, rawQ);
            }
        }

        if (isCommand(&data, "PHASE", 2))
        {
            valid = true;

            int32_t value = getFieldInteger(&data, 1);

            setPhase(value);
            putsUart0("\r\n PHASE SET \r\n");
            mode = sine;
        }

        if (isCommand(&data, "SINCOS", 0))
        {
            valid = true;
            mode = sine;
            setPhase(10000);
            putsUart0("\r\n DISPLAYING SIN COS \r\n");
        }

        if (isCommand(&data, "BPSK", 0))
        {
            valid = true;
            mode = bpsk;
            putsUart0("\r\n DISPLAYING BPSK \r\n");
        }

        if (isCommand(&data, "QPSK", 0))
        {
            valid = true;
            mode = qpsk;
            putsUart0("\r\n DISPLAYING QPSK \r\n");
        }

        if (isCommand(&data, "EPSK", 0))
        {
            valid = true;
            mode = epsk;
            putsUart0("\r\n DISPLAYING EPSK \r\n");
        }

        if (isCommand(&data, "QAM", 0))
        {
            valid = true;
            mode = qam;
            putsUart0("\r\n DISPLAYING QAM \r\n");
        }

        if (isCommand(&data, "OFF", 0))
        {
            valid = true;
            putsUart0("\r\n OFF \r\n");
            mode = dc;
        }

        if (isCommand(&data, "RATE", 2))
        {
            valid = true;

            int32_t value = getFieldInteger(&data, 1);

            setSymbolRate(value);
            putsUart0("\r\n TRANSMISSION RATE SET \r\n");
        }

        // ---------------- ADD: FILTER CONTROL ----------------
        // Your original code called setFilterStatus(1) but your wireless.c function is setFilterStatus(void).
        // So we toggle it here, and print ON/OFF status.

        if (isCommand(&data, "FILTER", 0))
        {
            valid = true;

            setFilterStatus();   // toggles filter + resets FIR state

            if (filter)
            {
                putsUart0("\r\n FILTER ON \r\n");

                // OPTIONAL: match your other project behavior: ISR rate x4 when filter enabled.
                // If you have a timer config function, do it here:
                // setSampleRate(FS * 4);
            }
            else
            {
                putsUart0("\r\n FILTER OFF \r\n");

                // OPTIONAL: revert ISR rate back:
                // setSampleRate(FS);
            }
        }

        // OPTIONAL: explicit OFF command if you want it:
        if (isCommand(&data, "NOFILTER", 0))
        {
            valid = true;
            if (filter)
                setFilterStatus(); // toggle off
            putsUart0("\r\n FILTER OFF \r\n");
            // setSampleRate(FS);
        }

        // OPTIONAL: status print
        if (isCommand(&data, "FSTATUS", 0))
        {
            valid = true;
            putsUart0(filter ? "\r\n FILTER=ON \r\n" : "\r\n FILTER=OFF \r\n");
        }

        if (!valid)
        {
            putsUart0("\r\n ERROR: Invalid command \r\n");
        }
    }
}
