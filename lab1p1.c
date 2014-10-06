//*************************************************************************//
// ECE 372A Lab1
// Team 08
// Albert Martinez
// Greg Burleson
// Michael Reed
//*************************************************************************//
#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"


_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
          BKBUG_ON & COE_ON & ICS_PGx1 &
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )



_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //
// Defines to simply UART's baud rate generator (BRG) regiser
// given the osicllator freqeuncy and PLLMODE.

#define XTFREQ          7372800                   // On-board Crystal frequency
#define PLLMODE         4                         // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)

#define BAUDRATE         115200       
#define BRGVAL          ((FCY/BAUDRATE)/16)-1 
 
/******************************************************************************************/

volatile int state; //running=2, stopped=1, reset=0
volatile unsigned char cnt;
volatile unsigned long int count;
volatile unsigned char mcnt;

/********************************************DebounceDelay**********************************/
void Debounce_Delay(void) 
	 {
	  T1CONbits.TON = 1; // start timer 
	  while(IFS0bits.T1IF == 0);  // wait (5ms)
	  T1CONbits.TON = 0; // turn off timer1
	  TMR1 = 0; // Reset Timer1
	  IFS0bits.T1IF = 0; // reset flag
     }

/***************************************StopWatch***Functions********************************/
void sw_reset()
       {
         if(state == 2); // timer currently running, ignore reset
		  
          
              
               
        else{
  	            LATAbits.LATA0 = 0; // turn on red lcd
                LATAbits.LATA1 = 1; // turn off green lcd 
				T3CONbits.TON = 0; // timer3 off
				count = 0;
				cnt = 0;
				mcnt = 0;
				TMR3=0;
                state = 0; // reset
				LCDClear(); // clear the LCD
				LCDMoveCursor(0,0); // cursor moved back to top
				LCDPrintString(" Ready ");
				LCDMoveCursor(1,0);
				// Display current time in format MM:SS:FF 
				LCDPrintChar('0');
				LCDPrintChar('0');
				LCDPrintChar(':');
				LCDPrintChar('0');
				LCDPrintChar('0');
				LCDPrintChar(':');
				LCDPrintChar('0');
				LCDPrintChar('0');
               	
        }
}

void sw_stop(){
		T3CONbits.TON = 0; // Stop Timer 3
		LATAbits.LATA0 = 0; // turn on red lcd
        LATAbits.LATA1 = 1; // turn off green lcd 
		state = 1; //stopped
		LCDMoveCursor(0,0);
		LCDPrintString(" Stopped ");
}


void sw_start(){
		T3CONbits.TON = 1; // Start Timer 3
		LATAbits.LATA0 = 1; // turn off red lcd
        LATAbits.LATA1 = 0; // turn on green lcd 
		state = 2; //running
		LCDMoveCursor(0,0); // cursor moved back to top
		LCDPrintString(" Running ");
}
		


int main(void)
{
	// ****************************************************************************** //

	// TODO: Configure AD1PCFG register for configuring input pins between analog input
	// and digital IO.
 	AD1PCFGbits.PCFG4 = 1;  //set IO5 to digital IO (Start Button)

	// TODO: Configure TRIS register bits for Right and Left LED outputs.
	TRISAbits.TRISA0 = 0; // red LED (IO1 as output)
    TRISAbits.TRISA1 = 0; // green LED (IO2 as output)

	// TODO: Configure LAT register bits to initialize Right LED to on.
    LATAbits.LATA0 = 0; //red LED on
    LATAbits.LATA1 = 1; //green LED off

	// TODO: Configure ODC register bits to use open drain configuration for Right
	// and Left LED output.
    ODCAbits.ODA0 = 1;
    ODCAbits.ODA1 = 1;

	// TODO: Configure TRIS register bits for swtich input.
	TRISBbits.TRISB2 = 1; //set IO5 as input (RB2) (stop/start)
    TRISBbits.TRISB5 = 1; //set RB5 as input (reset)

	// TODO: Configure CNPU register bits to enable internal pullup resistor for switch
	// input.
	CNPU1bits.CN6PUE = 1;
    CNPU2bits.CN27PUE = 1;

	// TODO: Setup Timer 1 to use internal clock (Fosc/2).
	T1CONbits.TCS = 0;

	// TODO: Setup Timer 1's prescaler to 1:256.
    T1CONbits.TCKPS1 = 1;
	T1CONbits.TCKPS0 = 1;
	

 	// TODO: Set Timer 1 to be initially off.
	T1CONbits.TON = 0;

	// **could have used T1CON = 0x0030** 

	// TODO: Clear Timer 1 value and reset interrupt flag
    TMR1 = 0;       
    IFS0bits.T1IF = 0; // reset interrupt flag for Timer1
    IEC0bits.T1IE = 1; // enable interrupt for Timer1

	// TODO: Set Timer 1's period value register to value for 5 ms.
    // Fosc     = XTFREQ * PLLMODE
    //             = 7372800 * 4
    //             = 29491200
    // 
    //    Fosc/2   = 29491200 / 2
    //             = 14745600
    //
    //    Timer 1 Freq = (Fosc/2) / Prescaler
    //                 = 14745600 / 256
    //                 = 57600
    //
    //    PR1 = 5 ms / (1 / (T1 Freq))
    //        = 5e-3 / (1 / 57600) 
    //        = 5e-3 * 57600
    //        = 288 
	PR1 = 288;
/*************************************Timer3 for Stopwatch**********************/
	
//Set Timer 3's prescaler to 1:256
T1CONbits.TCKPS1 = 1;
T1CONbits.TCKPS0 = 1;
IEC0bits.T3IE = 1; // enable interrupt for Timer1
PR3 = 576; // Timer 3's period value regsiter to value for 10 ms.
T3CONbits.TON = 0;	// Set Timer 3 to be initially off.
TMR3 = 0; // Clear Timer 1 value      
IFS0bits.T3IF = 0; // reset interrupt flag for Timer1
    
/***************************************CN Interrupt****************************/
 
CNEN1bits.CN6IE = 1;
CNEN2bits.CN27IE = 1; // CN interrupt for RB5 (reset)
IEC1bits.CNIE = 1; // Enable CN interrupt
IFS1bits.CNIF = 0; //clear flag

/****************************************Inititialize***************************/
LCDInitialize(); // Initialize LCD
LCDMoveCursor(0,0); // cursor moved back to top
LCDPrintString("Ready");
LCDMoveCursor(1,0);
// Display current time in format MM:SS:FF 
LCDPrintChar('0');
LCDPrintChar('0');
LCDPrintChar(':');
LCDPrintChar('0');
LCDPrintChar('0');
LCDPrintChar(':');
LCDPrintChar('0');
LCDPrintChar('0');

state=0;
cnt=0;
mcnt=0;
count=0;


while(1)
 {

 /***********************************StopWatch*************************************/
 long int sw_cnt;
 if(state == 2)
  {
     	    if(count%10 == 0)
			 {
              sw_cnt = (count/10)%10; 
              LCDMoveCursor(1,7); //update 1/100 sec
              LCDPrintChar(sw_cnt + '0');
             } 

			if(count%100 == 0)
             {
              sw_cnt = (count/100)%10; 
              LCDMoveCursor(1,6); // update 1/10 sec
              LCDPrintChar(sw_cnt + '0');	
             }

			if(count%1000 == 0)
             {
              sw_cnt = (count/1000)%10; 
              LCDMoveCursor(1,4); // update 1sec
              LCDPrintChar(sw_cnt + '0');	
             }

			if(count%10000 == 0)
             {
              LCDMoveCursor(1,3); //update 10sec
              LCDPrintChar(mcnt + '0');
             }

			if(count%60000 == 0)
             {
              sw_cnt = (count/60000)%10;  
              LCDMoveCursor(1,1);  // update 1min
              LCDPrintChar(sw_cnt + '0');
             }

			if(count%600000 == 0)
             {
              sw_cnt = (count/600000)%10;
              LCDMoveCursor(1,0);  // update 10min 
              LCDPrintChar(sw_cnt + '0'); 
	         } 
        
  }
}
return 0;
}

/*************************************timer3 Interrupt****************************************************/

void __attribute__((interrupt,auto_psv)) _T3Interrupt(void)
     {
      IFS0bits.T3IF = 0; //clear Timer 3 interrupt flag
      cnt = (cnt<9)?(cnt+1):0;	// Updates cnt to wraparound from 9 to 0 for this demo.
      count++;	// Increments the count

      if(count%1000 == 0)	
        mcnt = (mcnt<5)?(mcnt+1):0; // Updates mcnt to wraparound from 5 to 0
     }
	
/***********************************Change of Notification ************************************************/

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)
     {
      IFS1bits.CNIF =0;  // clear flag

      if (PORTBbits.RB2 == 0 ) 
        {
         if (state == 2) // check if stopwatch is running
          {
           sw_stop(); 
           Debounce_Delay(); 
           while(PORTBbits.RB2 == 0); // wait for button to be released
           Debounce_Delay();
         }
                 
      else  // stopwatch is currently stopped or was reset
        {
         sw_start();
         Debounce_Delay();
	     while(PORTBbits.RB2 == 0); // wait for button to be released
         Debounce_Delay();
        }
       }



      if(PORTBbits.RB5 == 0) // check if SW1 pressed
         sw_reset();
     }
	















