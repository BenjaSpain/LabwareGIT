// 0.Documentation Section 
// Lab7_HeartBlock, main.c

// Runs on LM4F120 or TM4C123 LaunchPad
// Input from PF4(SW1- Negative Logic) is AS (atrial sensor), 
// Output to PF3, Green LED, is Ready,
// Output to PF1, Red LED, is VT (ventricular trigger) 
// Make PF4 input, PF3,PF1 output
// Initialize Ready to high and VT to low
// Repeat this sequence of operation over and over
// 1) Wait for AS to fall (touch SW1 switch)
// 2) Clear Ready low
// 3) Wait 10ms (debounces the switch)
// 4) Wait for AS to rise (release SW1)
// 5) Wait 250ms (simulates the time between atrial and ventricular contraction)
// 6) set VT high, which will pulse the ventricles 
// 7) Wait 250ms
// 8) clear VT low 
// 9) set Ready high

// Date: January 15, 2016

// RESULT OF LAB 7 (Note of Benjamin 2023-09-18):
//		Lab Grade result is 67 in spite of 100 -> It is because the 'Delay' does not work
//		'Delay' is not working in any example for me... For now I leave the Lab working all the functions but the 'Delay'

// 1. Pre-processor Directives Section
#include "TExaS.h"

// Constant declarations to access port registers using 
// symbolic names instead of addresses
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
// 2. Declarations Section
//   Global Variables
unsigned long AS_SW1; // input from PF4
unsigned long READY_LED_GREEN; // Output PF3
unsigned long VT_LED_REED; // Output PF1

//   Function Prototypes
void PortF_Init(void);
void Delay1ms(unsigned long msec);
void EnableInterrupts(void);  // Enable interrupts
void WaitForASLow(void);
void WaitForASHigh(void);
void SetVT(void);
void ClearVT(void);
void SetReady(void);
void ClearReady(void);

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  TExaS_Init(SW_PIN_PF40, LED_PIN_PF31,ScopeOn);  // activate grader and set system clock to 80 MHz
  PortF_Init();                            // Init port PF4 PF3 PF1    
  EnableInterrupts();                      // enable interrupts for the grader  
  while(1){          // Follows the nine steps list above
    // a) Ready signal goes high
		SetReady();
    // b) wait for switch to be pressed
		WaitForASLow(); //OK
    // c) Ready signal goes low
		ClearReady();		//OK
    // d) wait 10ms
//		Delay1ms(10);
		Delay1ms(10); // ESTA FUNCION NO TIENE EFECTO. Me pasa en todos los ejemplos. LOS TEMPORIZADORES NO FUNCIONAN (TArdan microsegundos en vez de segundos)
		// e) wait for switch to be released
		WaitForASHigh();
    // f) wait 250ms
		Delay1ms(250); // ESTA FUNCION NO TIENE EFECTO. Me pasa en todos los ejemplos. LOS TEMPORIZADORES NO FUNCIONAN (TArdan microsegundos en vez de segundos)
    // g) VT signal goes high
		SetVT();
    // h) wait 250ms
    // i) VT signal goes low
		ClearVT();
  }
}
// Subroutine to initialize port F pins for input and output
// PF4 is input SW1 and PF3-1 is output LEDs
// Inputs: None
// Outputs: None
// Notes: ...
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;      // 1) F clock
  delay = SYSCTL_RCGC2_R;            // delay to allow clock to stabilize     
  GPIO_PORTF_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R &= ~0x10;         // 4.1) PF4 input,
  GPIO_PORTF_DIR_R |= 0x0E;          // 4.2) PF3,2,1 output  
  GPIO_PORTF_AFSEL_R &= 0x00;        // 5) no alternate function
  GPIO_PORTF_PUR_R |= 0x10;          // 6) enable pullup resistor on PF4       
  GPIO_PORTF_DEN_R |= 0x1E;          // 7) enable digital pins PF4-PF1
}
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E


// Subroutine reads AS input and waits for signal to be low
// If AS is already low, it returns right away
// If AS is currently high, it will wait until it to go low
// Inputs:  None
// Outputs: None
void WaitForASLow(void){ //OK
	do{
		AS_SW1 = GPIO_PORTF_DATA_R&0x10; // PF4 into SW1
	}while(AS_SW1 == 0x10);
}

// Subroutine reads AS input and waits for signal to be high
// If AS is already high, it returns right away
// If AS is currently low, it will wait until it to go high
// Inputs:  None
// Outputs: None
void WaitForASHigh(void){ //ESCRIBIENDO ESTA FUNCION
	do{
		AS_SW1 = GPIO_PORTF_DATA_R&0x10; // PF4 into SW1
	}while(AS_SW1 == 0x00);
}

// Subroutine sets VT high
// Inputs:  None
// Outputs: None
// Notes:   friendly means it does not affect other bits in the port
void SetVT(void){
	GPIO_PORTF_DATA_R |= 0x02;
}

// Subroutine clears VT low
// Inputs:  None
// Outputs: None
// Notes:   friendly means it does not affect other bits in the port
void ClearVT(void){
	GPIO_PORTF_DATA_R &= ~0x02;
}

// Subroutine sets Ready high
// Inputs:  None
// Outputs: None
// Notes:   friendly means it does not affect other bits in the port
void SetReady(void){ //OK
	GPIO_PORTF_DATA_R |= 0x08;
}


// Subroutine clears Ready low
// Inputs:  None
// Outputs: None
// Notes:   friendly means it does not affect other bits in the port
void ClearReady(void){  //OK
	GPIO_PORTF_DATA_R &= ~0x08;
}

// Subroutine to delay in units of milliseconds
// Inputs:  Number of milliseconds to delay
// Outputs: None
// Notes:   assumes 80 MHz clock
void Delay1ms(unsigned long msecs){
	unsigned long i;
	
  while(msecs > 0 ) { // repeat while there are still halfsecs to delay
		//Valor de i lo ajustamos a la Dev Kit según indicaciones del vídeo del Lab 7 en real board (Lab 7 , Part f)
    i = (16000*25/30); // 400000*0.5/0.13 that it takes 0.13 sec to count down to zero
    while (i > 0) { 
      i = i - 1;
    } // This while loop takes approximately 3 cycles
    msecs = msecs - 1;
  }
}

