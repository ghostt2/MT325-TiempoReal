#include <16f877a.h>
#device ICD=TRUE
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP 
#use delay(clock=10000000) 
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8) 
#use FAST_IO(A) 
#use FAST_IO(B)
#use FAST_IO(C) 


// **************************************
// Definicion de variables
char q;
// **************************************

// **************************************
// Declaracion de Funciones
void Cargar_configuracion();
void Configurar_interrupciones();
void version();
void encender_all ();
void apagar_all ();
//***************************************

// **************************************
// FUNCION PRINCIPAL
void main()
{
   Cargar_configuracion();
for(;;){
      q=getch();
      switch(q){
         case 'v': version();
         break;
         case 'e': encender_all();
         break;
         case 'a': apagar_all();
         break;
         default: printf("Error de comando");
         break;
         }
      delay_ms(100);
   }
}
// **************************************


// ****************************************
// Definicion de las Funciones

void Cargar_configuracion(){
   SET_TRIS_B(0x00);    //configura PORTB entrada / 0=salida / 1=entrada
   OUTPUT_B(0x00);   //inicializando PORTD en 0x00
   //SET_TRIS_D(0x00);    //configura PORTD salida
   }

void Configurar_interrupciones(){
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_2);   //Configuración timer0
   //set_timer0 (0x1B);                    //Carga del timer0
   //enable_interrupts(INT_TIMER0);       //Habilita interrupción timer0

   //enable_interrupts(INT_RB);       // Habilitando interrupcion RB /RB4-RB7
   //bit_clear(OPTION_REG,7);         // Habilita el Pull-UP de RB->OPTION_REG,RBPU
   //RBPU=0;                            // Habilita el PullUP de RB 
   //enable_interrupts(GLOBAL);       // Habilita las interrupciones
}

void version(){
   printf("Version de Software: v1");
   }
   
void encender_all (){
   OUTPUT_B(0xFF);
   printf("Encendidos Todos");
}

void apagar_all (){
   OUTPUT_B(0x00);
   printf("Apagados Todos");
}
//*****************************************

