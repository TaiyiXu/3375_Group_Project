#include "address_map_arm.h"
#include "lcd_driver.h"
#include "lcd_graphic.h"
#include <stdio.h>

#define GPIO_BASE 0xFF200060
#define _200MHz 200000000
#define SEVEN_SEGMENT_DISPLAY_BASE 0xFF200020

volatile unsigned int *const hex3_hex0_ptr = (unsigned int *)HEX3_HEX0_BASE;
volatile unsigned int *const hex5_hex4_ptr = (unsigned int *)HEX5_HEX4_BASE;
volatile unsigned int *const gpio_ptr = (unsigned int *)GPIO_BASE;
volatile unsigned int *const seven_segment_display_ptr = (unsigned int *)SEVEN_SEGMENT_DISPLAY_BASE;
volatile int *MPcore_private_timer_ptr = (int *)MPCORE_PRIV_TIMER; // timer address
volatile unsigned int *const key_base = (unsigned int *)KEY_BASE;

char lights_on[13] = "Lights On \0";
char lights_off[13] = "Lights Off\0";
char text[20];

volatile int actual_count = 20;

int readSwitch()
{
    volatile int *switchPointer = SW_BASE;
    return (*switchPointer) & 0x01;
}

int getRemainTime()
{ // getting the seconds of how many time are left until the light turn
    int arrayLength = 0;
    int value = actual_count;

    while (value != 0)
    {
        value /= 10;
        arrayLength++;
    }

    switch (arrayLength)
    {
    case 3:
        sprintf(text, "Off after %d", actual_count);
        break;
    case 2:
        sprintf(text, "Off after 0%d", actual_count);
        break;
    case 1:
        sprintf(text, "Off after 00%d", actual_count);
        break;
    default:
        break;
    }
    actual_count--;
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

// timer////////////////////////////////////////////////////////////////
void private_timer(int counter)
{
    *(MPcore_private_timer_ptr) = counter;   // write to timer load register
    *(MPcore_private_timer_ptr + 2) = 0b011; // mode = 1 (auto), enable =
    while (*(MPcore_private_timer_ptr + 3) == 0)
        ;                                // wait for timer to expire
    *(MPcore_private_timer_ptr + 3) = 1; // reset timer flag bit
    return;
}

// main function////////////////////////////////////////////////////////
int main(void)
{
    *(gpio_ptr + 1) = 0; // enbale the input for GPIO
    int run_once = 1;
    int can_count = 0;

    // Initialize LCD21
    init_spim0();
    init_lcd();
    clear_screen();

    while (1)
    {
        if (*key_base)
        {
            actual_count = 20;
            can_count = 1;
        }

        if (actual_count == 0)
        {
            can_count = 0;
        }

        if (can_count)
        {
            run_once = 1;
            lightDisplay(1);
            getRemainTime();
            LCD_text(lights_on, 0);
            LCD_text(text, 1);
            refresh_buffer();
            private_timer(_200MHz);
        }
        else
        {
            if (run_once)
            {
                clear_screen();
                run_once = 0;
            }
            *(MPcore_private_timer_ptr + 2) = 0;
            lightDisplay(0);
            LCD_text(lights_off, 0);
            refresh_buffer();
        }
    }
}
