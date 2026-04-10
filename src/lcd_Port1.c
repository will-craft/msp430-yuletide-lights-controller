//****************************************************************************//
// UTFPR - Prof. Sergio Moribe                                                //
// MSP430X2XX                                                                 //
// Biblioteca Funçoes de controle EM 4 bITS de Display LCD 16X2               //
// Obs.: R/W = 0 Somente escrita no display (não le busy flag)                //
// RS=P1.3, E_LCD=P1.0, DB7=P1.7, DB6=P1.6, DB5=P1.5, DB4=P1.4                //
//****************************************************************************//

//#include <msp430F2131.h>
#include <msp430G2553.h>
#include <intrinsics.h>

//***************************************************************************
// Defines do Display LCD
//***************************************************************************
#define RS BIT3     //REGISTER SELECT DO LCD
#define LCD BIT0    //ENABLE DO LCD

//***************************************************************************
//PROTOTIPO DAS FUNÇÕES
//***************************************************************************
void IniDisp(void);
void strobe();
void EscInst(unsigned char inst);
void EscDado(unsigned char dado);
void GotoXY(unsigned char Linha,unsigned char Coluna);
void MString(unsigned char *str);
unsigned char CHexAsc(unsigned char hex);

//***************************************************************************
//FUNÇÕES DE TRATAMENTO DO DISPLAY DE CRISTAL LIQUIDO LCD 16X2
//***************************************************************************
//***************************************************************************
//FUNÇÃO DE INICIALIZACAO DO DISPLAY LCD
void IniDisp(void)
{

  __delay_cycles(20000);        //Espera 20 ms
  P1OUT &= BIT2+BIT1;   //Zera tudo menos BIT2 e BIT1
  P1OUT |= 0x30;        //RS=0,DB7,DB6,DB5,DB4 = 0011
  strobe();
  __delay_cycles(10000); //Espera 10 ms
  P1OUT &= BIT2+BIT1;   //Zera tudo menos BIT2 e BIT1
  P1OUT |= 0x30;        //RS=0,DB7,DB6,DB5,DB4 = 0011
  strobe();
  __delay_cycles(1000); //Espera 1mS
  P1OUT &= BIT2+BIT1;   //Zera tudo menos BIT2 e BIT1
  P1OUT |= 0x30;        //RS=0,DB7,DB6,DB5,DB4 = 0011
  strobe();
  __delay_cycles(1000); //Espera 1mS
  P1OUT &= BIT2+BIT1;   //Zera tudo menos BIT2 e BIT1
  P1OUT |= 0x20;        //RS=0,DB7,DB6,DB5,DB4 = 0010 => 4 bit mode
  strobe();
  __delay_cycles(1000); //Espera 100uS 
  EscInst(0x06);        //INSTRUCAO DE MODO DE OPERACAO
  EscInst(0x28);        //4-bits, 2 linhas
  EscInst(0x0C);        //INSTRUCAO DE Cursor Off
  EscInst(0x01);        //INSTRUCAO DE LIMPEZA DO DISPLAY
  __delay_cycles(100000);
}

//***************************************************************************
//FUNÇÃO PULSO DE STROBE NO LCD
void strobe()
{
  P1OUT |= LCD;         //HAB. LCD=1
  __delay_cycles(1);
  P1OUT &= ~LCD;        //FAZ LCD=0
}

//***************************************************************************
//FUNÇÃO QUE ESCREVE INSTRUÇÃO PARA O DISPLAY E ESPERA DESOCUPAR
//NOME; ESCINST
//ENTRADA: INSTRUCAO A SER ESCRITA NO MODULO DISPLAY
void EscInst(unsigned char inst)
{
  P1OUT &= BIT2+BIT1;   //RS=0 Zera tudo menos BIT2 e BIT1
  P1OUT |= (inst & 0xF0); //Separa parte alta
  strobe();
  P1OUT &= BIT2+BIT1;   //RS=0 Zera tudo menos BIT2 e BIT1
  P1OUT |= ((inst<<4) & 0xF0); //Separa parte baixa e hab. LCD
  strobe();
  __delay_cycles(500);  //espera 500us
}

//***************************************************************************
//FUNÇÃO QUE ESCREVE DADO PARA O DISPLAY
//NOME: ESCDADO
//ENTRADA: DADO EM ASCII A SER ESCRITO NO MODULO DISPLAY
void EscDado(unsigned char dado)
{
  P1OUT &= BIT2+BIT1;   //RS=0 Zera tudo menos BIT2 e BIT1
  P1OUT |= (dado & 0xF0) | RS; //Separa parte alta e RS=1 E LCD=1
  strobe();
  P1OUT &= BIT2+BIT1;   //RS=0 Zera tudo menos BIT2 e BIT1
  P1OUT |= (dado<<4 & 0xF0) | RS; //Separa parte baixa e RS=1 E LCD=1
  strobe();
  __delay_cycles(500);  //espera 500us  
}

//***************************************************************************
//FUNÇÃO QUE POSICIONA O CURSOR
//NOME: GOTOXY
//ENTRADA: LINHA (0 A 1)
//         COLUNA (0 A 15)
void GotoXY(unsigned char Linha,unsigned char Coluna)
{
  if (Linha == 0)
    Coluna |= 0x80;     //Calcula posição na linha 0
  else
    Coluna |= 0xC0;     //Calcula posição na linha 1
  EscInst(Coluna);      //INSTRUCAO
}

//*****************************************************************************
//FUNÇÃO QUE MOSTRA UMA STRING NO DISPLAY A PARTIR DA POSICAO DO CURSOR
//NOME: MSTRING
//ENTRADA: ENDERECO INICIAL DA STRING NA MEMORIA FINALIZADA POR 00H
void MString(unsigned char *str)
{
  while (*str)
  {
    EscDado(*str);
    str++;
  }
}

//***************************************************************************
//ROTINA QUE CONVERTE BYTE DE 00h A 0Fh EM ASCII
//NOME: CHEXASC
//ENTRADA: VALOR HEXADECIMAL DE 00h A 0Fh
//SAIDA: VALOR ASCII CORRESPONDENTE
unsigned char CHexAsc(unsigned char hex)
{
  if (hex < 10)           //Se número
    return hex+0x30;      //converte número em ASCII
  else
    return hex+0x37;      //converte letra em ASCII
}
