#include <stdio.h>
#include <msp430.h>
#include <string.h>
#include <stdint.h>

#include "peripherals.h"

/**
 * header file
 */
void displayTime ();
void displayTemp(float inAvgTempC);
void config_timerA2(void);
void initADC(void);
void timeConvert (long unsigned int inTime);

// Temperature Sensor Calibration Reading at 30 deg C is stored
// at addr 1A1Ah. See end of datasheet for TLV table mapping
#define CALADC12_15V_30C *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration Reading at 85 deg C is stored
// at addr 1A1Ch See device datasheet for TLV table mapping
#define CALADC12_15V_85C *((unsigned int *)0x1A1C)
#define MA_PER_BIT   0.244 // =1.0A/4096


unsigned int in_temp, int_current;
float milliamps;
volatile unsigned int bits30, bits85;
volatile float degC_per_bit, temperatureDegC;
volatile long unsigned int pos, prev;
long unsigned int timer;
long unsigned int in_time;
long unsigned int month, day, hour, min, sec, remain;
//tContext g_sContext;

/**
 * main.c
 */
void main(void)
{
    int state =0;
    WDTCTL = WDTPW | WDTHOLD;  //
    configDisplay();
    Graphics_clearDisplay(&g_sContext); // Clear the display
    config_timerA2();
    configKeypad();
    initLeds();
    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR
    initADC();

    timer =864000;
    in_time =0;
    prev = 0;
    month =0;
    day =0;
    hour =0;
    min =0;
    sec =0;
    while(1)
    {
        char currkey = getKey();
        char cu;
        pos = ADC12MEM0 & 0x00FFF;

        if (currkey == '1'||currkey =='2'||currkey == '3'||currkey =='4'||currkey == '5')
        {

            state =1;
            pos = ADC12MEM0 & 0x00FFF;

        }

        if (currkey == '#')
        {
            state = 0;
            setLeds(0);
            currkey = ' ';
            timer = month*2592000+ (day-1)*86400 + hour*3600+min*60+sec;
        }
        switch(state)
        {
        case 0:  //normal
            ADC12CTL0 &= ~ADC12SC; // clear the start bit
            ADC12CTL0 |= ADC12SC; // Sampling and conversion start
            // Single conversion (single channel)
            // Poll busy bit waiting for conversion to complete
            while (ADC12CTL1 & ADC12BUSY)
                __no_operation();

            __enable_interrupt();
            displayTemp(temperatureDegC);
            prev = pos;
            timeConvert(timer);
            displayTime();
            __no_operation(); // SET BREAKPOINT HERE
            break;


        case 1: //editing

            if (currkey != 0x00 && currkey != ' ')
            {
                cu = currkey;
            }

                if (cu == '1') //add month -> add 60*60*24*30  (assume every month has 30 days)  -->will fix later
                {
                    setLeds(8);
                    month =  pos*11/4095;
                }
                else if (cu == '2') //add day -> add 60*60*24
                {
                    setLeds(4);
                    day = pos*29/4095;
                }
                else if (cu == '3') //add hour -> add 60*60
                {
                    setLeds(2);
                    hour = pos*23/4095;
                }
                else if (cu == '4') //add min -> add 60
                {
                    setLeds(1);
                    min =  pos*59/4095;
                }
                else if (cu == '5') //add second -> add1
                {
                    setLeds(15);
                    sec =  pos*59/4095;
                }

                displayTime();
            break;

        default:
            break;
        }



        Graphics_flushBuffer(&g_sContext);
    }
    //return 0;
}

void timeConvert (long unsigned int inTime)
{

       remain =0;

       month = (inTime /(2592000));
       remain = inTime - ( month*(2592000));
       day = (remain / (86400)) +1;
       remain = remain - ((day-1)*(86400));
       hour = remain/3600;
       remain = remain-(hour*3600);
       min = remain /60;
       sec = remain - (min*60);

}
void displayTime ()
{
    unsigned char disp[2];
    //month code
    Graphics_drawStringCentered(&g_sContext, "Date: ", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);

    if (month ==0)
    {
        Graphics_drawStringCentered(&g_sContext, "JAN", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==1)
    {
        Graphics_drawStringCentered(&g_sContext, "FEB", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==2)
    {
        Graphics_drawStringCentered(&g_sContext, "MAR", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==3)
    {
        Graphics_drawStringCentered(&g_sContext, "APR", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==4)
    {
        Graphics_drawStringCentered(&g_sContext, "MAY", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }

    else if (month ==5)
    {
        Graphics_drawStringCentered(&g_sContext, "JUN", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==6)
    {
        Graphics_drawStringCentered(&g_sContext, "JUL", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==7)
    {
        Graphics_drawStringCentered(&g_sContext, "AUG", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==8)
    {
        Graphics_drawStringCentered(&g_sContext, "SEP", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==9)
    {
        Graphics_drawStringCentered(&g_sContext, "OCT", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else if (month ==10)
    {
        Graphics_drawStringCentered(&g_sContext, "NOV", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }
    else
    {
        Graphics_drawStringCentered(&g_sContext, "DEC", AUTO_STRING_LENGTH, 40, 25, OPAQUE_TEXT);
    }


    disp[1] = day%10+48;
    disp[0] = day/10+48;
    Graphics_drawStringCentered(&g_sContext, disp, 2, 60, 25, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);

    Graphics_drawStringCentered(&g_sContext, "Time: ", AUTO_STRING_LENGTH, 48, 35, OPAQUE_TEXT);
    disp[1] = hour%10+48;
    disp[0] = hour/10+48;
    Graphics_drawStringCentered(&g_sContext, disp, 2, 25, 45, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, ":", AUTO_STRING_LENGTH, 35, 45, OPAQUE_TEXT);
    disp[1] = min%10+48;
    disp[0] = min/10+48;
    Graphics_drawStringCentered(&g_sContext, disp, 2, 45, 45, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, ":", AUTO_STRING_LENGTH, 55, 45, OPAQUE_TEXT);

    disp[1] = sec%10+48;
    disp[0] = sec/10+48;
    Graphics_drawStringCentered(&g_sContext, disp, 2, 65, 45, OPAQUE_TEXT);

    // Refresh the display so it shows the new data
    Graphics_flushBuffer(&g_sContext);

}


void displayTemp(float inAvgTempC)
{
    Graphics_drawStringCentered(&g_sContext, "Temperature:", AUTO_STRING_LENGTH, 48, 60, OPAQUE_TEXT);
    unsigned char disp[5];
    float F;
    disp[0] = ' ';
    disp[1] = ' ';

    disp[2] = (int) inAvgTempC %10 + 48;
    disp[3] = '.';
    disp[4] = (int)(inAvgTempC*10) %10 + 48;

    if (inAvgTempC >=10)
    {
        disp[1] = ((int)(inAvgTempC/10))%10+48;
    }
    if (inAvgTempC >=100)
    {
        disp[0] = ((int)(inAvgTempC/100))%10+48;
    }


    Graphics_drawStringCentered(&g_sContext, " C", AUTO_STRING_LENGTH, 60, 70, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, disp, 5, 40, 70, OPAQUE_TEXT);

    F = (inAvgTempC*9)/5 + 32;

    disp[0] = ' ';
    disp[1] = ' ';

    disp[2] = (int) F %10 + 48;
    disp[3] = '.';
    disp[4] = (int)(F*10) %10 + 48;

    if (F >=10)
    {
        disp[1] = ((int)(F/10))%10+48;
    }
    if (F >=100)
    {
        disp[0] = ((int)(F/100))%10+48;
    }
    Graphics_drawStringCentered(&g_sContext, " F", AUTO_STRING_LENGTH, 60, 80, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, disp, 5, 40, 80, OPAQUE_TEXT);
    Graphics_flushBuffer(&g_sContext);

}

//initialize adc   -> 2 sensors
void initADC(void)
{
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_9|ADC12REFON|ADC12ON|ADC12MSC;
    ADC12CTL1 = ADC12SHP|ADC12CONSEQ_1;
    ADC12MCTL0 = ADC12SREF_1|ADC12INCH_0;
    ADC12MCTL1 = ADC12SREF_1|ADC12INCH_10|ADC12EOS;
    ADC12IE = BIT1;

    P6SEL |= BIT0;
    P6DIR |= BIT0;
    __delay_cycles(100);  // delay to allow Ref to settle
    ADC12CTL0 |= ADC12ENC;

}

void config_timerA2(void)
{
    TA2CTL = TASSEL_1 + MC_1 + ID_0; // ACLK, divider of 1, up mode
    TA2CCR0 = 32768;                  // 32768 ACLK periods = 1 seconds
    TA2CCTL0 = CCIE; // Enable interrupt on Timer A2
}

//ADC Interupt Function
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    // Move the results for both channels into global variables
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));
    //prev = pos;
    in_temp = ADC12MEM1; // Read results from conversion
    temperatureDegC = (float)((long)in_temp - bits30) * degC_per_bit +30.0;
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void)
{
    // You probably still want to keep track of time

    //displayTime (timer);
    timer++;
    ADC12CTL0 |= ADC12SC;
    // Do you still need to poll the busy bit?  No!  If you configured interrupts,
    // the ADC12 ISR will fire when the conversion is complete.
    // If you did not configure interrupts, you could poll the busy bit here.
}
