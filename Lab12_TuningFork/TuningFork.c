// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

/*
* Included for PLL configuration = 80 MHz
*/


/*
* GLOBAL Variables
*/
unsigned long volatile SW_LAST_STATE 	= 0;
unsigned short volatile OUT_STATE 		= 0; // 0 = LOW , 1 = Toggleing


// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode


// input from PA3, output from PA2, SysTick interrupts
/***********Sound_Init**************
* Initialize Output and Interrupts
*	PA2: Output
* PA3: Input.
* Rissing edge interrupt
* @input: 	void
* @output: 	void
***********************************/
void Sound_Init(void){ unsigned long volatile delay;
  SYSCTL_RCGC2_R |= 0x00000001; // activate port A
  delay = SYSCTL_RCGC2_R;
  /*
  GPIO_PORTA_PCTL_R &= ~0x00F00000; // Configure PA5 as GPIO
	GPIO_PORTA_AMSEL_R &= ~0x20;      // Disable analog functionality on PA5
	GPIO_PORTA_DIR_R |= 0x20;     		// make PA5 out
  GPIO_PORTA_DR8R_R |= 0x20;    		// can drive up to 8mA out
  GPIO_PORTA_AFSEL_R &= ~0x20;  		// disable alt funct on PA5
  GPIO_PORTA_DEN_R |= 0x20;     		// enable digital I/O on PA5
	*/
	GPIO_PORTA_LOCK_R =	0x4C4F434B; 		// unlock GPIO Port A
  GPIO_PORTA_CR_R = (0x04|0x08);      // Allow changes to PA3
	GPIO_PORTA_AMSEL_R &= ~(0x04|0x08); // Disable analog functionality on PA2 and PA3
	GPIO_PORTA_PCTL_R &= ~0x0000FF00; 	// Configure PA2 and PA3 as GPIO
	GPIO_PORTA_DIR_R &= ~0x08;     			// make PA3 In
	GPIO_PORTA_DIR_R |= 0x04;     			// make PA2 Out
	GPIO_PORTA_AFSEL_R &= ~(0x04|0x08); // Disable Alt funct on PA2 and PA3
  GPIO_PORTA_PDR_R |= 0x08;						// Enable weak pull-up on PA3 (Positive Logic)
	GPIO_PORTA_DEN_R |= (0x04|0x08);    // Enable digital I/O on PA2 and PA3
	GPIO_PORTA_DATA_R &= (0x04|0x08);  	// Make PA2 and PA3 Low
		
  NVIC_ST_CTRL_R = 0;           			// disable SysTick during setup
  NVIC_ST_RELOAD_R = 109077;     			// reload value for 500us (assuming 80MHz) -> Calculated by Relation of 3 (39999cycles are 1000ms then 2727ms is X cycles) -> X=109077 cycles
  NVIC_ST_CURRENT_R = 0;        			// any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0                
  NVIC_ST_CTRL_R = 0x00000007;  			// enable with core clock and interrupts
  EnableInterrupts();
}

/***********SysTick_Handler**************
* Action when Systick interrupt is launched. 
* If Rissing Edge is detected on Switch (PA3) -> Change PA2 behaviour
*		Low Level continuous -> Toggle (440 Hz - 2,727 ms)
*		Toggle (440 Hz) -> Low Level continuous
* It is evaluated at 880 Hz (1,13636 ms)
* @input: 	void
* @output: 	void
*************************/ 
void SysTick_Handler(void){
	unsigned long volatile CURRENT_STATUS = GPIO_PORTA_DATA_R&0x08;
	
	// Fix new behaviour. When PA3 detect rissing edge -> Change PA2 behaviour (Toggle -> Low , Low -> Toogle)
	if (CURRENT_STATUS)
	{
		if(SW_LAST_STATE != CURRENT_STATUS) 
		{ 
			if(OUT_STATE == 0) // Last was Low?
				OUT_STATE = 1;
			else
			{ // Apply Low?
				OUT_STATE = 0;
				GPIO_PORTA_DATA_R &= ~0x04;
			}
		}
	}
	// Apply Toggle?
	if(OUT_STATE == 1)
		GPIO_PORTA_DATA_R ^= 0x04;
	
  SW_LAST_STATE = CURRENT_STATUS;
}

int main(void){// activate grader and set system clock to 80 MHz
  PLL_Init(); 																				// Init System Bus Freq = 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 	// Not Touch this call. Internall function (Texas.h) for grader.
	Sound_Init();																				// Init Ports, Configure Interrupts 
  EnableInterrupts();   															// Enable Interrupts after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
