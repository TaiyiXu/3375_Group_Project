#include "address_map_arm.h"
#include "lcd_driver.h"
#include "lcd_graphic.h"
#include <stdio.h>

#define GPIO_BASE 0xFF200060
#define _MHz 1000000
#define SEVEN_SEGMENT_DISPLAY_BASE 0xFF200020

volatile unsigned int *const hex3_hex0_ptr = (unsigned int *)HEX3_HEX0_BASE;
volatile unsigned int *const hex5_hex4_ptr = (unsigned int *)HEX5_HEX4_BASE;
volatile unsigned int *const gpio_ptr = (unsigned int *)GPIO_BASE;
volatile unsigned int *const seven_segment_display_ptr = (unsigned int *)SEVEN_SEGMENT_DISPLAY_BASE;

char lights_on[13] = "Lights On \0";
char lights_off[13] = "Lights Off\0";
char text[20];

volatile int count = 0;
// timer////////////////////////////////////////////////////////////////

typedef struct armTimer
{ // creating a defined data type containning these properties

    int load;    // max count
    int count;   // cuurent count
    int control; // control register for the timer
    int status;  // flag of the timer status(is timeout or not)

} armTimer;

armTimer *timer = (armTimer *)0xFFFEC600; // creating a armTimer object called timer

void set_timer(int interval)
{
    timer->load = interval * _MHz;
}

int readSwitch()
{
    volatile int *switchPointer = SW_BASE;
    return (*switchPointer) & 0x01;
}

void start_timer()
{
    // 011 = 0x3, Enable timer, continue mode, 100 MHz now
    timer->control = 3 + (1 << 8);
    timer->status = 1;
}

int check_timer()
{
    volatile int current_count = timer->count;
    return timer->load > current_count;
}

void wait_for_timer()
{
    while (check_timer())
    {
    }
}

char getRemainTime(int count)
{ // getting the seconds of how many time are left until the light turn
    sprintf(text, "%d", count);
    return 1;
}

// PIR////////////////////////////////////////////////////////////////

// LEDS////////////////////////////////////////////////////////////////

void lightDisplay(int value)
{
    int statusTable[2];

    statusTable[0] = 0x5C7171; // OFF
    statusTable[1] = 0x5C37;   // On

    int state = value; // the state of the light

    if (state == 0)
    {
        *seven_segment_display_ptr = statusTable[0]; // display oFF
    }
    else
    {
        *seven_segment_display_ptr = statusTable[1]; // display oN.
    }
}

void updateCount()
{
    count++;
}

// main function////////////////////////////////////////////////////////
int main(void)
{
    *(gpio_ptr + 1) = 0; // enbale the input for GPIO

    // Initialize LCD21
    init_spim0();
    init_lcd();
    clear_screen();

    while (1)
    {
        int gpio_previous = readSwitch();

        if (readSwitch())
        {
            set_timer(1);
            start_timer();
            lightDisplay(1);
            updateCount(count);
            getRemainTime(count);
            LCD_text(lights_on, 0);
            LCD_text(text, 1);
            refresh_buffer();
            wait_for_timer();
        }
        else
        {
            set_timer(1);
            start_timer();
            lightDisplay(0);
            updateCount(count);
            getRemainTime(count);
            LCD_text(lights_off, 0);
            LCD_text(text, 1);
            refresh_buffer();
            wait_for_timer();
        }
    }
}
