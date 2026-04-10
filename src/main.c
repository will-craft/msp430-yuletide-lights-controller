// MSP430 Yuletide Lights Controller


#include <msp430G2553.h>
#include <intrinsics.h>

// Function declarations
void init_system(void);                         //Initialize system

// LCD Display function declarations from lcd_Port1.c by Prof. Sergio Moribe (UTFPR)
extern void IniDisp(void);
extern void EscInst(unsigned char inst);
extern void EscDado(unsigned char dado);
extern void GotoXY(unsigned char Linha,unsigned char Coluna);
extern void MString(unsigned char *str);

//Global system variables  
unsigned int hour;              //hour variable
unsigned int minute;            //minute variable
unsigned int second;            //second variable
unsigned int turnOffHour;       //turn-off hour variable
unsigned int turnOffMin;        //turn-off minute variable
unsigned int turnOnHour;        //turn-on hour variable
unsigned int turnOnMin;         //turn-on minute variable
unsigned char lineBuf[17];      //17-position array - line buffer
unsigned int updateDisplay;     //display update status variable
unsigned int switchMode;        //switch programming mode
unsigned int prevSwitch;        //previous switch state
                                //7 = Set current time
                                //6 = Set turn-on time
                                //5 = Set turn-off time
                                //4 = Effect 1 - one direction only
                                //3 = Effect 2 - both directions
                                //2 = Effect 3 - center to edges
                                //1 = Effect 4 - edges to center
                                //0 = Effect 5 - all effects in sequence
unsigned int uartCtrl;          //UART controller
unsigned int timeSetMode;       //time programming controller
unsigned int effect;            //general effect controller
unsigned int ef2Ctrl;           //effect 2 controller
unsigned int ef3Ctrl;           //effect 3 controller
unsigned int ef4Ctrl;           //effect 4 controller
unsigned int ef5Ctrl;           //effect 5 controller
unsigned char effectState;      //general effect on/off state
unsigned char rxBuf[6];         //RX buffer
unsigned int i, j;              //iteration variables
unsigned int rxCmd;             //RX commands
unsigned int txCmd;             //TX commands

//Main Program
void main( void )
{
  init_system();            //Initialize system
  IniDisp();                //Initialize LCD 2x16 display
  __bis_SR_register(GIE);   //Enable interrupts
  
  while (1)
  {
    switchMode = P2IN & 7;              //Variable receives input value
    if(switchMode != prevSwitch){       //When input changes
      prevSwitch = switchMode;          //Register the change
      uartCtrl = 1;                     //Activate UART controller
      switch(switchMode){
        case 0:         //Effect 5
          effect = 5;                   //Register effect in variable
          P2OUT = 0x0F;                 //Initial output value
          ef2Ctrl = 0;                  //Reset effect 2 controller
          ef3Ctrl = 0;                  //Reset effect 3 controller
          ef4Ctrl = 0;                  //Reset effect 4 controller
          ef5Ctrl = 0;                  //Reset effect 5 controller
          break;
        case 1:         //Effect 4
          effect = 4;                   //Register effect in variable
          P2OUT = 0x8F;                 //Initial output value
          ef4Ctrl = 0;                  //Reset effect 4 controller
          break;
        case 2:         //Effect 3
          effect = 3;                   //Register effect in variable
          P2OUT = 0x27;                 //Initial output value
          ef3Ctrl = 0;                  //Reset effect 3 controller
          break;
        case 3:         //Effect 2
          effect = 2;                   //Register effect in variable
          P2OUT = 0x0F;                 //Initial output value
          ef2Ctrl = 0;                  //Reset effect 2 controller
          break;
        case 4:         //Effect 1
          effect = 1;                   //Register effect in variable
          P2OUT = 0x0F;                 //Initial output value
          break;
        case 5:         //Set turn-off time
          P2OUT = 0x07;                 //Turn off LEDs
          break;
        case 6:         //Set turn-on time
          P2OUT = 0x07;                 //Turn off LEDs
          break;
        case 7:         //Set current time
          P2OUT = 0x07;                 //Turn off LEDs
          break;
      }
    }
    
    if(updateDisplay == 1){
      //FIRST DISPLAY LINE
      GotoXY(0,0);                //Position cursor
      EscDado((hour/10) + '0');   //Show hour tens digit
      EscDado((hour%10) + '0');   //Show hour units digit
      EscDado(':');               //Show :
      EscDado((minute/10) + '0'); //Show minute tens digit
      EscDado((minute%10) + '0'); //Show minute units digit
      EscDado(':');               //Show :
      EscDado((second/10) + '0'); //Show second tens digit
      EscDado((second%10) + '0'); //Show second units digit
      
      switch(switchMode){
        case 0:         //Effect 5
          MString(" EF. 5 ");
          break;
        case 1:         //Effect 4
          MString(" EF. 4 ");
          break;
        case 2:         //Effect 3
          MString(" EF. 3 ");
          break;
        case 3:         //Effect 2
          MString(" EF. 2 ");
          break;
        case 4:         //Effect 1
          MString(" EF. 1 ");
          break;
        case 5:         //Set turn-off time
          MString(" S.OFF ");
          break;
        case 6:         //Set turn-on time
          MString(" S.ON  ");
          break;
        case 7:         //Set current time
          MString(" S.TIME ");
          break;
      }
      EscDado(effectState);
      
      //SECOND DISPLAY LINE
      GotoXY(1,0);                        //Position cursor
      EscDado('O');
      EscDado(' ');
      EscDado((turnOnHour/10) + '0');     //Show turn-on hour tens digit
      EscDado((turnOnHour%10) + '0');     //Show turn-on hour units digit
      EscDado(':');
      EscDado((turnOnMin/10) + '0');      //Show turn-on minute tens digit
      EscDado((turnOnMin%10) + '0');      //Show turn-on minute units digit
      MString("  ");
      EscDado('F');
      EscDado(' ');
      EscDado((turnOffHour/10) + '0');    //Show turn-off hour tens digit
      EscDado((turnOffHour%10) + '0');    //Show turn-off hour units digit
      EscDado(':');
      EscDado((turnOffMin/10) + '0');     //Show turn-off minute tens digit
      EscDado((turnOffMin%10) + '0');     //Show turn-off minute units digit
      
      
      updateDisplay = 0;
    }
    
    //------------SERIAL COMMANDS------------
        if (rxCmd == 1){
          rxCmd = 0;
          if (rxBuf[0] == 'H'){			//Hour command
            if (rxBuf[3] == 0x0A){
              switch (timeSetMode){
              case 1:
                  hour = rxBuf[2] - '0';
                  break;
              case 2:
                  turnOnHour  = rxBuf[2] - '0';
                  break;
              case 3:
                  turnOffHour = rxBuf[2] - '0';
                  break;    
                }
            }
            
            else {
              switch(timeSetMode){
                  case 1:
                    hour = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                  break;
                  case 2:
                    turnOnHour = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                   break;
                   case 3:
                    turnOffHour = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                   break;
              }
            
            }
          }
          
          if (rxBuf[0] == 'M'){			//Minute command
                if (rxBuf[3] == 0x0A){
              switch (timeSetMode){
              case 1:
                  minute = rxBuf[2] - '0';
                  break;
              case 2:
                  turnOnMin  = rxBuf[2] - '0';
                  break;
              case 3:
                  turnOffMin = rxBuf[2] - '0';
                  break;    
                }             
          }
          
          else {
              switch(timeSetMode){
                  case 1:
                    minute = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                  break;
                  case 2:
                    turnOnMin = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                   break;
                   case 3:
                    turnOffMin = ((rxBuf[2] - '0')*10) + rxBuf[3] - '0';
                   break;
              }
          }   
        }
        
        if (rxBuf[0] == 'B')			//Button toggle command
        {
          if(effectState == 'O'){
            effectState = 'F';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = 'O';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = ' ';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = '0';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = 0x0A;
          }
          else{
            effectState = 'O';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = 'O';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = ' ';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = '1';
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = 0x0A;
          }
        }
        
       
    unsigned char msg1[10] = {'S',' ','E','F','F','E','C','T',':',' '} ;    
     if (txCmd == 0){    
     if(timeSetMode == 0){     
    while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
       
   
      UCA0TXBUF = msg1[j]; 
      
          if (j == 9){
            txCmd = 1;
            j = 0; 
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = effect + '0';
            
            while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
            UCA0TXBUF = 0x0A; 
          }else
            j++;
      
     }
      unsigned char msg2[18] = {'S',' ','S','E','T',' ','C','U','R','R','E','N','T',' ','T','I','M','E'};
      unsigned char msg3[18] = {'S',' ','S','E','T',' ','T','U','R','N','-','O','N',' ','T','I','M','E'};
      unsigned char msg4[19] = {'S',' ','S','E','T',' ','T','U','R','N','-','O','F','F',' ','T','I','M','E'};
     
     if(timeSetMode == 1){
      
       
        while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
        UCA0TXBUF = msg2[j];
            if (j == 17){
              txCmd = 1;
              j = 0;
              while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
              UCA0TXBUF = 0x0A;
              effectState = 'F';
          }          

        else
          j++;
      
     }
     
      if(timeSetMode == 2){
        while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
        UCA0TXBUF = msg3[j];
            if (j == 17){
              txCmd = 1;
              j = 0;
              while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
              UCA0TXBUF = 0x0A;
              effectState = 'F';
          }        
        else
          j++;
     }  
        
        if(timeSetMode == 3){
       
        while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
        UCA0TXBUF = msg4[j];
            if (j == 18){
              txCmd = 1;
              j = 0;
              while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
              UCA0TXBUF = 0x0A;
              effectState = 'F';
          }   
        else j++;
     }
  }  
  
  
  
       if (uartCtrl == 1){
            
    if(effectState == 'F'){
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = 'A';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = ' ';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = '0';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = 0x0A;  
          txCmd = 1;
      
    }
    else{
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = 'A';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = ' ';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = '1';
          while(!(IFG2 & UCA0TXIFG)); //WAIT FOR TX BUFFER
          UCA0TXBUF = 0x0A; 
          txCmd = 1;
    }
    
      uartCtrl = 0;
    }
  }
  }
}

//Initialize system
void init_system(void)
{
  WDTCTL = WDTPW+WDTHOLD;       //Disable watchdog
  DCOCTL = 0; 					//CALIBRATE DCO TO 1MHz
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  // Configure ports
  P1DIR = 0xFF;         				//SET P1 ALL AS OUTPUT
  P1OUT = 0x00;         				//ALL P1 OUTPUTS TO ZERO
  P2SEL &= ~(BIT7 + BIT6); 				//SELECT P2.6 AND P2.7 AS GPIO
  P2DIR	= BIT7+BIT6+BIT5+BIT4+BIT3;     //SET P2.3 TO P2.7 AS OUTPUT
  P2REN = BIT2+BIT1+BIT0;               //Enable pull-up/pull-down
  P2OUT	= BIT2+BIT1+BIT0;               //Pull-up on inputs P2.0 to P2.2
                                        //Outputs at 0
    
  //Configure TimerA
  TA0CTL = TASSEL_2+ID_3+MC_3+TAIE; //SMCLK/8, UP/DOWN MODE, ENABLE TAIFG INT
  TA0CCR0 = 62500;                  //COUNT APPROX. 1s
  TA1CTL = TASSEL_2+ID_2+MC_1+TAIE; //SMCLK/4, UP MODE, ENABLE TAIFG INT
  TA1CCR0 = 50000;                  //COUNT APPROX. 200ms
  
  //UART
  UCA0CTL1 |= UCSSEL_2; //SELECT SMCLK
  UCA0BR0 = 104;        //UART AT 9600BPS
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS_1;
  UCA0CTL1 &= ~UCSWRST; //START SERIAL
  IE2 |= UCA0RXIE;      //ENABLE RX INTERRUPT
    
  //Initialize variables
  hour = 0;                           //Initialize hour to 0
  minute = 0;                         //Initialize minute to 0
  second = 0;                         //Initialize second to 0
  updateDisplay = 1;                  //Initialize display update
  effectState = 'O';                  //Initialize effect state
  ef2Ctrl = 0;                        //Initialize effect 2 controller
  ef3Ctrl = 0;                        //Initialize effect 3 controller
  ef4Ctrl = 0;                        //Initialize effect 4 controller
  ef5Ctrl = 0;                        //Initialize effect 5 controller
}

//ISR HANDLES TIMER0_A INTERRUPTS - TAIV VECTOR
#pragma vector = TIMER0_A1_VECTOR
__interrupt void isr_timer0_A1(void)
{
 switch(TA0IV){
  case TA0IV_TAIFG:    //TAIFG = 0x0A
      second++;        //Increment second value
      if (second > 59){
          second = 0;
          minute++;
      }
      if (minute > 59){
          minute = 0;
          hour++;
      }
      if (hour > 23){
          hour = 0;
      }
      
      updateDisplay = 1;
      break;
   }  
}


//ISR HANDLES TIMER1_A INTERRUPTS - TAIV VECTOR
#pragma vector = TIMER1_A1_VECTOR
__interrupt void isr_timer1_A1(void)
{
  //--------------LED EFFECT PROGRAMMING--------------
  switch(TA1IV){
    case TA1IV_TAIFG:    //TAIFG = 0x0A
      if(effectState == 'O'){
        switch(switchMode){
            case 0:         //Effect 5
              if(ef5Ctrl == 0){			//Part 1
                P2OUT <<= 1;            	//Shift left
                P2OUT &= ~(BIT3);       	//Clear BIT3
                P2OUT |= 7;             	//Restore pull-up
                if(P2OUT == 7){         	//When sequence ends
                  P2OUT = 0x0F;         	//Set output for next sequence
                  ef5Ctrl = 1;          	//Start next part
                }
              }
              else if(ef5Ctrl == 1){   //Part 2
                if(ef2Ctrl == 0){      		//Step 1
                  P2OUT <<= 1;          		//Shift left
                  P2OUT &= ~(BIT3);     		//Clear BIT3
                  P2OUT |= 7;           		//Restore pull-up
                  if(P2OUT == 0x87){    		//When sequence ends
                    ef2Ctrl = 1;        		//Start next step
                  }
                }
                else {               		//Step 2
                  P2OUT >>= 1;          		//Shift right
                  P2OUT |= 7;           		//Restore pull-up
                  if(P2OUT == 0x0F){    		//When sequence ends
                    ef2Ctrl = 0;        		//Reset effect 2 controller
                    ef5Ctrl = 2;        		//Start next part
                  }
                }
              }
              else if(ef5Ctrl == 2){	//Part 3
                  if(ef3Ctrl == 0){     	//Step 1
                  P2OUT = 0x27;         		//Set output value
                  ef3Ctrl = 1;          		//Start next step
                }
                else if(ef3Ctrl == 1){		//Step 2
                  P2OUT = 0x57;         		//Set output value
                  ef3Ctrl = 2;          		//Start next step
                }
                else{                   	//Step 3
                  P2OUT = 0x8F;         		//Set output value
                  ef3Ctrl = 0;          		//Reset effect 3 controller
                  ef5Ctrl = 3;          		//Start next part
                }
              }
              else{                 	//Part 4
                if(ef4Ctrl == 0){       	//Step 1
                P2OUT = 0x8F;           		//Set output value
                ef4Ctrl = 1;           			//Start next step
                }
                else if(ef4Ctrl == 1){		//Step 2
                  P2OUT = 0x57;         		//Set output value
                  ef4Ctrl = 2;          		//Start next step
                }
                else{                   	//Step 3
                  P2OUT = 0x27;         		//Set output value
                  ef4Ctrl = 0;          		//Reset effect 4 controller
                  ef5Ctrl = 0;          		//Reset effect 5 controller
                }
              }
              break;
            case 1:         //Effect 4
              if(ef3Ctrl == 0){ 			//Step 1
                P2OUT = 0x8F;           		//Set output value
                ef3Ctrl = 1;            		//Start next step
              }
              else if(ef3Ctrl == 1){      	//Step 2
                P2OUT = 0x57;           		//Set output value
                ef3Ctrl = 2;           			//Start next step
              }
              else{                     	//Step 3
                P2OUT = 0x27;           		//Set output value
                ef3Ctrl = 0;            		//Reset effect 4 controller
              }
              break;
            case 2:         //Effect 3
              if(ef3Ctrl == 0){           	//Step 1
                P2OUT = 0x27;           		//Set output value
                ef3Ctrl = 1;            		//Start next step
              }
              else if(ef3Ctrl == 1){      	//Step 2
                P2OUT = 0x57;           		//Set output value
                ef3Ctrl = 2;            		//Start next step
              }
              else{                     	//Step 3
                P2OUT = 0x8F;           		//Set output value
                ef3Ctrl = 0;            		//Reset effect 3 controller
              }
              break;
            case 3:         //Effect 2
              if(ef2Ctrl == 0){  			//Step 1
                P2OUT <<= 1;            		//Shift left
                P2OUT &= ~(BIT3);       		//Clear BIT3
                P2OUT |= 7;             		//Restore pull-up
                if(P2OUT == 0x87){      		//When sequence ends
                  ef2Ctrl = 1;          		//Start next step
                }
              }
              else {                		//Step 2
                P2OUT >>= 1;            		//Shift right
                P2OUT |= 7;             		//Restore pull-up
                if(P2OUT == 0x0F){      		//When sequence ends
                  ef2Ctrl = 0;          		//Reset effect 2 controller
                }
              }
              break;
            case 4:         //Effect 1
              P2OUT <<= 1;         			//Shift left
              P2OUT &= ~(BIT3);         	//Clear BIT3
              P2OUT |= 7;               	//Restore pull-up
              if(P2OUT == 7){           	//When sequence ends
                P2OUT = 0x0F;           	//Return to initial position
              }
              break;
        }
      }
  break;
  }
}

//ISR HANDLES UART INTERRUPTS
#pragma vector=USCIAB0RX_VECTOR
__interrupt void ISR_RX(void)
{
      rxBuf[i] = UCA0RXBUF;             //Assign RX buffer to variable
      
      if (rxBuf[i] == 0x0A){            //When buffer end is reached
          rxCmd = 1;                    //Activate RX controller
          i = 0;                        //Reset iteration variable
      }
      else i++;                         //Otherwise increment variable
}