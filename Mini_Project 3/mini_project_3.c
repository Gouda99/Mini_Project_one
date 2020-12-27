/*
 * MiniProject3.c
 *
 *  Created on:  17/12/2020
 *      Author: GOUDA
 */
#include "common_macros.h"
#include "std_types.h"
#include "micro_config.h"
#include "lcd.h"
uint8 g_edgeCount = 0;
uint16 g_timeHigh = 0;
uint16 g_timePeriod = 0;
uint16 g_timePeriodPlusHigh = 0;


typedef enum
{
	LOW_LEVEL,ANY_CHANGE,FALLING,RISING
}INT0_edge_type;

void INT0_init(void)
{
	SREG  &= ~(1<<7);                   
	DDRD  &= (~(1<<PD2));              
	GICR  |= (1<<INT0);                 
	MCUCR |= (1<<ISC00) | (1<<ISC01);  
	SREG  |= (1<<7);               
}

void Timer1_init_normalMode(void)
{
	TCNT1 = 0;
	TCCR1A = (1<<FOC1A) | (1<<FOC1B);
	TCCR1B |= (1<<CS10);
}

void INT0_setEdgeDetectionType(const INT0_EdgeType EdgeType)
{
	MCUCR=(MCUCR & 0xFC) | (EdgeType);
}


ISR (INT0_vect)
{
	g_edgeCount++;
	if(g_edgeCount == 1)
	{
		/*
		 * Clear the timer counter register to start measurements from the
		 * first detected rising edge
		 */
		TCNT1 = 0;
		/* Detect falling edge */
		INT0_setEdgeDetectionType(FALLING);
	}
	else if(g_edgeCount == 2)
	{
		/* Store the High time value */
		g_timeHigh = TCNT1;
		/* Detect rising edge */
		INT0_setEdgeDetectionType(RISING);
	}
	else if(g_edgeCount == 3)
	{
		/* Store the Period time value */
		g_timePeriod = TCNT1;
		/* Detect falling edge */
		INT0_setEdgeDetectionType(FALLING);
	}
	else if(g_edgeCount == 4)
	{
		/* Store the Period time value + High time value */
		g_timePeriodPlusHigh = TCNT1;
		/* Clear the timer counter register to start measurements again */
		TCNT1 = 0;
		/* Detect rising edge */
		INT0_setEdgeDetectionType(RISING);
	}
}

int main()
{
	uint32 dutyCycle = 0;
	LCD_init();
	Timer1_Init_normalMode();
	INT0_init();
	while(1)
	{
		if(g_edgeCount == 4)
		{

			TCNT1 = 0;
			TCCR1B = 0;
			SREG &= ~(1<<7);
			g_edgeCount = 0;
			LCD_displayString("Duty = ");
			/* calculate the dutyCycle */
			dutyCycle = ((float)(g_timePeriodPlusHigh-g_timePeriod) / (g_timePeriodPlusHigh - g_timeHigh)) * 100;
			/* display the dutyCycle on LCD screen */
			LCD_intgerToString(dutyCycle);
			LCD_displayCharacter('%');
		}
	}
}
