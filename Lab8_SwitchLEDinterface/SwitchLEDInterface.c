// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi
// Modified by:
//	Benjamin Canton Dominguez
//	2023-09-22

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
//#include "tm4c123gh6pm.h"
// Defines for PortE Registers Access
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))

// ***** 2. Global Declarations Section *****
// Global Variables
unsigned long In; 								// Input of PE0 (SW1)
unsigned long Pasa1=0, Pasa2=0; 	// Auxiliar variable for debug

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
	void Init_PE0_PE1(void); 	// ADDED -> Init PE1 and PE0 (ports, Clocks, etc)
//void delayms(unsigned long ms);
// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  EnableInterrupts();           // Enable interrupts for the grader	
	Init_PE0_PE1();								// Init PE1 and PE0
	//	PE0 -> Switch, 10 Kohm Pull-Down, Positive Logic
	// 	PE1 -> LED con 470 ohm, Positive Logic
	GPIO_PORTE_DATA_R |= 0x02; 	// Initial Status: LED1 = ON (PE1=High)
  while(1){
//		delayms(1000);
		In = GPIO_PORTE_DATA_R&0x01;  // Read PE0
		if (In&0x01) {								// If SW1 (PE0) Pressed -> Togle LED (PE1) (Friendly way)
			GPIO_PORTE_DATA_R = GPIO_PORTE_DATA_R^0x02;
			Pasa1 = Pasa1^1;
		}
		else {											  // If SW1 Not Pressed -> LED1 OFF
		  GPIO_PORTE_DATA_R &= ~0x02;
			Pasa2 = Pasa2^1;
		}
  }
	return 0;
}

/************************************
* Function: Init_PE0_PE1						*
* Description: INIT Ports E1 and E2	*
*	Input:	Nothing										*
* Return: Nothing										*
************************************/
void Init_PE0_PE1(void)
{
	 unsigned long volatile delay;

  SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  // GPIO_PORTE_DIR_R |= 0x10;         // PE4 output -> CAMBIAR
  GPIO_PORTE_DIR_R |= 0x02;         // PE1: Output
  // GPIO_PORTE_DIR_R &= ~0x07;        // PE2,1,0 input  -> CAMBIAR
	GPIO_PORTE_DIR_R &= ~0x01;        // PE0: Input
  // GPIO_PORTE_AFSEL_R &= ~0x17;      // not alternative Functions for PE4,PE2,PE1,PE0
	GPIO_PORTE_AFSEL_R &= ~0x03;      // not alternative Functions for PE0 and PE1
  // GPIO_PORTE_AMSEL_R &= ~0x17;      // no analog for PE4,PE2,PE1,PE0
	GPIO_PORTE_AMSEL_R &= ~0x03;      // no analog Functions for PE0 and PE1
  // GPIO_PORTE_PCTL_R &= ~0x000F0FFF; // bits for PE4,PE2,PE1,PE0
	GPIO_PORTE_PCTL_R &= ~0x0000000FF; // bits for PE1,PE0
  // GPIO_PORTE_DEN_R |= 0x17;         // enable PE4,PE2,PE1,PE0
	GPIO_PORTE_DEN_R |= 0x03;         // enable PE1,PE0
}	
/* NO USO LA SUBRUTINA DE DELAY PQ COMO EN TODOS LOS EJEMPLOS ANTERIORES NO ME FUNCIONA
// Subroutine to delay in units of 1 msec
// Inputs: Number of ms to delay
// Outputs: None
void delayms(unsigned long ms){
  unsigned long count;
  while(ms > 0 ) { // repeat while there are still ms to delay
    count = 16000; // number of counts to delay 1ms at 80MHz
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    ms--;
  }
}
*/
