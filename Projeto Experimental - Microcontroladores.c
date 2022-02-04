/*
 * File:   newmain.c
 * Author: Daniel, Lucas e Yasmin
 *
 * Created on 10 de Outubro de 2021, 15:00
 */

/*==========================================================================================
	Predefinições e diretivas
==========================================================================================*/
#define _XTAL_FREQ 8000000  //Frequência do cristal

// Configurando pinos do display
#define RS RD1  
#define EN RD0
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define ad_resolution 1023 //Resolução -> quantidade de bits   
#define vdd           5    //Tensão de 5V
#define factor        100  //Fator de correção do sensor LM35


// Configurações padrão
#pragma config FOSC = HS 
#pragma config WDTE = OFF 
#pragma config PWRTE = OFF 
#pragma config BOREN = ON 
#pragma config LVP = OFF 
#pragma config CPD = OFF 
#pragma config WRT = OFF  
#pragma config CP = OFF 

#include <xc.h>             
#include "lcd.h";   // Biblioteca do display


/*==========================================================================================
	Variáveis globais
==========================================================================================*/

unsigned int M[] = {0,50,100,150,200,250,300,350,400,450};
unsigned int temp;
unsigned char temperatura,centena,dezena,unidade,dec1,dec2,nivel;

/*==========================================================================================
    Função ADC 
==========================================================================================*/

unsigned int ReadADC(){
    
  GO_DONE = 1;                          //Inicia a conversão                   
  while(GO_DONE);                       //Reseta automaticamente ao fim da conversão      
  return ((ADRESH<<8) | ADRESL);        //Retorna resultados da conversão
}


/*==========================================================================================
    Função que muda o valor 
==========================================================================================*/
void ChangeDC(){
    
    temp = M[nivel];              
    CCP1CONbits.CCP1Y = temp;       //bit menos significativo
    temp = temp >> 1;
    CCP1CONbits.CCP1Y = temp;       //2º menos significativo
    temp = temp >> 1;             
    CCPR1L = temp;                 //8 bits mais significativos
    
} 

/*==========================================================================================
    Função de interrupção
==========================================================================================*/

void __interrupt() Interrupt(){  
    if(INTF == 1){
        INTF = 0;
        while(PORTBbits.RB0){
        PORTDbits.RD2 = 1;
        __delay_ms(20);  
        }
    return;
    }
}            

/*==========================================================================================
    Main
==========================================================================================*/
int main()
{        
    float store;
    
    TRISCbits.TRISC2 = 0;
    TRISD = 0;                    
    PR2 = 124;      //Baseado na aula                 
    TMR2 = 0;       //               
    T2CON = 7;      //              
    ADCON0 = 0x45;  //   
    ADCON1 = 0xC0;  //         
    CCP1CON = 0x0F; //         

    
    //Habilitar Interrupção:
    TRISBbits.TRISB0 = 1;          
    OPTION_REGbits.INTEDG = 1;      
    INTE = 1;                       
    GIE  = 1;                      
    
    
    //Configurando o ventilador e porta como desligado
    PORTDbits.RD3 = 0;
    PORTDbits.RD2 = 0;
    
    
    //Ligando o display
    Lcd_Init();
    
    
    while(1){
        store = (ReadADC());    
        temperatura = (store*vdd*factor)/ad_resolution;  
        centena = (temperatura-((temperatura-(temperatura%10))/10)%10)/100;
        dezena = ((temperatura-(temperatura%10))/10)%10;
        unidade = temperatura%10;
        //dec1 = ((temperatura%1000)%100)/10;
        //dec2 = ((temperatura%1000)%100)%10;
        dec1 = 0;
        dec2 = 0;
        
        Lcd_Clear(); 
        Lcd_Set_Cursor(1,3);
        Lcd_Write_String("TEMPERATURA");
        Lcd_Set_Cursor(2,5);
        Lcd_Write_Char(centena+48);   
        Lcd_Write_Char(dezena+48);
        Lcd_Write_Char(unidade+48);
        Lcd_Write_String(".");
        Lcd_Write_Char(dec1+48);
        Lcd_Write_Char(dec2+48);
        Lcd_Write_String(" ");
        Lcd_Write_String("C");
        __delay_ms(800);
        
  
        if (centena == 0){
            PORTDbits.RD3 = 0; //exaustor desligado
            nivel = 9;   
            ChangeDC();  
        }
        else if(centena == 1){            
            if (dezena >= 0 && dezena < 3){
            PORTDbits.RD3 = 0; 
            nivel = 9;       
            ChangeDC();          
            } 
            else if (dezena >= 3 && dezena < 9){
            PORTDbits.RD3 = 0; 
            nivel = 7;       
            ChangeDC();  
            }
            else {
            PORTDbits.RD3 = 0;
            nivel = 5;       
            ChangeDC();     
            }                    
        }   
        else if (centena >= 2){
                if (dezena >= 0 && dezena < 4){
                    PORTDbits.RD3 = 0;
                    nivel = 5;        
                    ChangeDC();    
                }
                else if (dezena == 4){
                    if (unidade == 0){
                        PORTDbits.RD3 = 0;
                        nivel = 2;        
                        ChangeDC(); 
                    } else {
                        PORTDbits.RD3 = 1;
                        nivel = 0;        
                        ChangeDC(); 
                        } 
                } else{
                    PORTDbits.RD3 = 1;
                    nivel = 0;        
                    ChangeDC();
                }
        }
        else{
        break;
        }
    }
    return 0;
}

