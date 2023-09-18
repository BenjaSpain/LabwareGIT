// BranchingFunctionsDelays.c Lab 6
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to 
// toggle an LED while a button is pressed and 
// turn the LED on when the button is released.  
// This lab will use the hardware already built into the LaunchPad.
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// built-in connection: PF0 connected to negative logic momentary switch, SW2
// built-in connection: PF1 connected to red LED
// built-in connection: PF2 connected to blue LED
// built-in connection: PF3 connected to green LED
// built-in connection: PF4 connected to negative logic momentary switch, SW1

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

#include "TExaS.h"

//#include <stdio.h>

/*
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
*/

#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC)) // Data of PortF GPIOs[0-7]
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108)) // Active clock Ports Register
#define SYSCTL_RCGC2_GPIOF      0x00000020  															// Mask to Clock register PTF
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520)) // Unlock write access to GPIOCR register when we write to the register value 0x4C4F434B

#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524)) // Enable/Disable write to GPIOAFSEL, GPIOPUR, GPIOPDR, GPIODEN
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528)) // Enable/Disable Anaglog Functions
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C)) // Select specific peripheral signal for each GPIO pin when use alternative function mode. Use in conjuntion with GPIOAFSEL
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400)) // Pin Direction (1->Out, 0->In) 
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420)) // Use of Alternate Function for Pin. (1->Enable, 0->Disable)
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510)) // Internal Pull-Up of Pins. (1->Enable. 0->Disable)
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C)) // Digital Enable. (1->Enable, 0-> Disble)


// Global Variables
unsigned long In; 	// Input from PF4
unsigned long Out; 	// Output to PF2 (blue LED)

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortF_Init(void);				// Init Port F
void Delay100ms(void);				// Delay 100 ms. Assumes 6 cycles per loop (100ms/12.5ns/6).


int main(void){
	
  TExaS_Init(SW_PIN_PF4, LED_PIN_PF2);  // Activate grader and set system clock to 80 MHz
  // initialization goes here
	PortF_Init();													// Init Port F (SW1 (PF4) and LED (PF2)
  EnableInterrupts();          					// Enable interrupts for the grader
	
	while(1){ 
		// Delay 100 ms
		Delay100ms();									// 100 ms delay each loop -> ERROR ESTO NO ESTA FUNCIONANDO... EL test INDICA QUE ES DEMASIADO RAPIDO EL BUCLE									
		// Read SW1
		In = GPIO_PORTF_DATA_R&0x10;  	// Read PF4 into Sw1
		In = In>>2;
		if (In&0x04) 										// If SW1 Not Pressed (PF4 == 1) 	-> LED (PF2) is ON
			GPIO_PORTF_DATA_R = In;
		else														// If SW1 Pressed (PF4 == 0) 			-> Togle LED (PF2)
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04;
	}
	return 0;
}


void PortF_Init(void)
{
	volatile unsigned long delay;
	
	//Step 1: Activate/Reset Clock for the Port F (SYSCTL_RCGC2_R).
	SYSCTL_RCGC2_R 			|= SYSCTL_RCGC2_GPIOF;		// PortF Clock Gating Control Enable = SET Bit 5 of RCGC2
  delay 							= SYSCTL_RCGC2_R;					// Short delay after the clock activation 
	
	//Step 2: Unlock GPIOs on PortF
	GPIO_PORTF_LOCK_R 	= 0x4C4F434B; 						// Enables write access to GPIOCR. Unlock GPIOCR
	GPIO_PORTF_CR_R 		= 0x14;        						// Allow changes to PF4 and PF2 Configuration Registers (GPIOAFSEL, GPIOPUR, GPIOPDR, GPIODEN) 
	
	//Step 3: Disable Analog function
	//	Clear PF4 and PF2 bits in AMSEL. We use it for digital I/O (AMSEL egister)
	GPIO_PORTF_AMSEL_R 	= 0x00; 									// Disable analog function
	
	//Step 4: Let use as GPIO (regular digital function). Clear bits PF4 and PF2 in PCTL Register
	GPIO_PORTF_PCTL_R		= 0x00000000;   					// Clear bit PCTL	
	
	//Step 5: Set Direction of the Pins (DIR register). SW1: PTF4 -> Input. LED: PTF2 -> Output
	GPIO_PORTF_DIR_R 		= 0x04;

	//Step 6: Disable alternate functions. Clear bits of PTF2 and PTF4 in AFSEL Register. 
	GPIO_PORTF_AFSEL_R 	= 0x00;
	
	//Step 7. Activate Internal PULL UP Resistor for PTF4. Set bit PTF4 in PUR Register. 	*Note -> In the EV_KIT we need set bits in PUR register for the swith inputs to have internall pull-up resistor
	GPIO_PORTF_PUR_R 		= 0x10;          					// Enable pullup resistors on PF4 (Input GPIO)
	
	//Step 8: Enable Digital use. Set PTF2 and PTF4 bits in DEN Register
	GPIO_PORTF_DEN_R 		= 0x14;         					// Enable digital pins PF4 and PF2
}


// 100 ms delay each loop -> ERROR ESTO NO ESTA FUNCIONANDO... EL test INDICA QUE ES DEMASIADO RAPIDO EL BUCLE
/*
* The following C function can be used to delay. The number 1333333 assumes 6 cycles per loop (100ms/12.5ns/6). 
* The Keil optimization is set at Level 0 (-O0) and the “Optimize for Time” mode is unchecked.
*/
// void Delay100ms(unsigned long time){
void Delay100ms(){
  unsigned long i;
	unsigned long delay=100;
	
	while(delay > 0){
		i = 1333333;
		while(i > 0){
			i--;
		}
		delay--;
	}
	
}
