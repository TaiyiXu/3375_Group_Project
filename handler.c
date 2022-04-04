#include "address_map_arm.h"
#include "lcd_driver.h"
#include "lcd_graphic.h"

#define GPIO_BASE 0xFF200060
#define HEX3_HEX0_BASE 0XFF200020
#define HEX5_HEX4_BASE 0XFF200030
#define HERTZ 2000000 // the timer is 200MHZ but it's too slow
#define SEVEN_SEGMENT_DISPLAY_BASE 0xFF200020

volatile unsigned int *const hex3_hex0_ptr = (unsigned int *)HEX3_HEX0_BASE;
volatile unsigned int *const hex5_hex4_ptr = (unsigned int *)HEX5_HEX4_BASE;
volatile unsigned int *const gpio_ptr = (unsigned int *)GPIO_BASE;
volatile unsigned int *const seven_segment_display_ptr = (unsigned int *)SEVEN_SEGMENT_DISPLAY_BASE;
volatile unsigned int *const seven_segment_display_ptr = (unsigned int *)SEVEN_SEGMENT_DISPLAY_BASE;

char lights_on[13] = "Lights On \0";
char lights_off[13] = "Lights Off\0";
// timer////////////////////////////////////////////////////////////////

typedef struct armTimer
{ // creating a defined data type containning these properties

    int load;    // max count
    int count;   // cuurent count
    int control; // control register for the timer
    int status;  // flag of the timer status(is timeout or not)

} armTimer;

armTimer *timer = (armTimer *)0xFFFEC600; // creating a armTimer object called timer

void setTimer(int interval)
{
    timer->load = HERTZ; // should be count to 1 second
}

int readSwitch()
{
    volatile int *switchPointer = SW_BASE;
    return (*switchPointer) & 0x01;
}

void startTimer()
{
    timer->control = 3 + (1 << 8); // write 011 to the enable continue and interrupt
    timer->status = 1;             // set the time-out flag to 0
}

char getRemainTime(int count)
{ // getting the seconds of how many time are left until the light turn
    char remainTime = 600 - count;
    return remainTime;
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

// main function////////////////////////////////////////////////////////
int main(void)
{
    *(gpio_ptr + 1) = 0; // enbale the input for GPIO
    init_spim0();
    init_lcd();
    clear_screen();

    while (1)
    {
        int count = 0;
        int gpio_previous = *(gpio_ptr);

        if (*(gpio_ptr))
        {

            while (count < 600)
            {
                startTimer();
                lightDisplay(1);
                count++;
                LCD_text(lights_on, 0);
                LCD_text(getRemainTime(count), 1);
                refresh_buffer();
                if (*(gpio_ptr) != gpio_previous && count >= 500)
                {
                    count = 0;
                }
            }
        }
        else
        {

            while (count < 600)
            {
                startTimer();
                lightDisplay(0);
                count++;
                LCD_text(lights_off, 0);
                refresh_buffer();

                if (*(gpio_ptr) != gpio_previous && count >= 500)
                {
                    count = 0;
                }
            }
            // Initialize LCD21
        }
    }
}