#include <16f877a.h>
#device ICD=TRUE
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP 
#use delay(clock=10000000) 
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8) 
#use FAST_IO(A) 
#use FAST_IO(B)
#use FAST_IO(C) 

#byte STATUS = 0x03
#byte PCLATH = 0x0A
#byte PCL = 0x02
#byte PIE1 = 0x8C

#bit RCIE = PIE1.5


// **************************************
// Definicion de variables y estructuras
char comando;
int valor;
int quantum;
int j1,j2;
int k1,k2;
int l1,l2;
int i;
int m;
int np = 0;
int temp;
int cola[8];


struct PCB{
   int id;
   int status;
   int pclath;
   int pcl;
   int16 dir_inicio;
   int16 dir_fin;
   int16 dir_actual;
   int acc;
   int estado;
   int estado_proceso;
   int prioridad;
} proceso[7];

// **************************************


// **************************************
// Declaracion de Funciones
void Cargar_configuracion();
void Configurar_interrupciones();
void version();
void activar_timer0 ();
void desactivar_timer0 ();
void cargar_proceso(int idp);
void descargar_proceso(int idp);
void Procesos_Init();
//***************************************

// Declaracion de las Interrupciones
#int_TIMER0
void quantum(void) {
   int i=0;
   desactivar_timer0 ();
   proceso[0].status = STATUS;
   proceso[0].pclath = PCLATH;
   proceso[0].pcl = PCL;
   proceso[cola[0]].dir_actual=(int16)PCL;
   proceso[cola[0]].dir_actual=proceso[cola[0]].dir_actual|((int16)PCLATH<<8);
   
   if(np>1)
   {  temp=cola[0];
      for(i=0;i<np-1;i++)
      {cola[i]=cola[i+1];
      }
      cola[np-1]=temp;
   }
   
}
// **************************************************

// **************************************************
// Procesos

// Sistema Operativo
#ORG 0x0100, 0x0200
void Inicio_SO()
{
   while (TRUE)
   {  
      RCIE = 1;
      printf("A");
      delay_ms(10);
      comando = getch();
      delay_ms(10);
      printf("V");
      delay_ms(10);
      valor = getch();
      valor = valor - 48;
      RCIE = 0;
      
      switch(comando)
      {
         case 'C':{if (valor<10) cargar_proceso(valor);break;}
         case 'D':{if (valor<10) descargar_proceso(valor);break;}
         case 'X':{for (m=0;m<8;m++) cargar_proceso(m);
                   break;}
      }
      comando='-';
      if(np>0)
       {
         if(proceso[cola[0]].estado==1)
            {activar_timer0();
             proceso[cola[0]].estado=2;
             goto_address(proceso[cola[0]].dir_inicio);
            }
         else
            {
             activar_timer0();
             goto_address(proceso[cola[0]].dir_actual);
            }
       }
   }
}

//PROGRAMA 1: Luces Secuenciales - Bucle Infinito
#ORG 0x205, 0x300
void ROML_LedB1()
{
   set_tris_b(0x00);
   while(TRUE)
   {
      for (j1=0;j1<3;j1++)
      {
         IF(j1==0)
         {
            OUTPUT_BIT(PIN_B0,0);
            OUTPUT_BIT(PIN_B1,0);
         }
         IF(j1==1){OUTPUT_BIT(PIN_B0,1);}
         IF(j1==2){OUTPUT_BIT(PIN_B1,1);}
         for (k1=1;k1<200;++k1){for (l1=1;l1<200;++l1){}}
      }
   }
}

//PROGRAMA 2 : Luces Secuenciales - Bucle Infinito
#ORG 0x305, 0x400
void ROML_LedB2()
{
   set_tris_b(0x00);
   while(TRUE)
   {
      for (j2=0;j2<3;j2++)
      {
         IF(j2==0)
         {
            OUTPUT_BIT(PIN_B2,0);
            OUTPUT_BIT(PIN_B3,0);
         }
         IF(j2==1){OUTPUT_BIT(PIN_B2,1);}
         IF(j2==2){OUTPUT_BIT(PIN_B3,1);}
         for (k2=1;k2<500;++k2){for (l2=1;l2<500;++l2){}}
      }
   }
}



// **************************************
// FUNCION PRINCIPAL
void main(void)
{
   Cargar_configuracion();
   Configurar_interrupciones();
   Procesos_Init();
   #asm
   call 0x0100
   #endasm

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
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_2);   //Configuración timer0

   //enable_interrupts(INT_RB);       // Habilitando interrupcion RB /RB4-RB7
   //bit_clear(OPTION_REG,7);         // Habilita el Pull-UP de RB->OPTION_REG,RBPU
   //RBPU=0;                            // Habilita el PullUP de RB 
   //enable_interrupts(GLOBAL);       // Habilita las interrupciones
}

void activar_timer0 (){
   set_timer0 (0x1B);                    //Carga del timer0
   enable_interrupts(INT_TIMER0);       //Habilita interrupción timer0
}
void desactivar_timer0 (){
   disable_interrupts(INT_TIMER0);       // Deshabilita interrupción timer0
}

void cargar_proceso(int idp)
{
    cola[np]=idp-1;
    proceso[idp-1].estado=1;
    printf("\n\r Proceso %i cargado. *",idp);
    //proceso=10;
    np++;
}

void descargar_proceso(int idp)
{
   int s,d,f;
   for(s=0;s<np;s++)
      {if(cola[s]==idp-1)
         {d=s;}
      }
   f=np-1;
   for(s=d;s<f;s++)
      {cola[s]=cola[s+1];
      }
   np--;   
   proceso[idp-1].estado=0;
   //proceso=0;
   printf("\n\r Proceso %i descargado. *",idp);
}

void Procesos_Init()
{
   proceso[0].dir_inicio=0x0205;
   proceso[0].dir_actual=0x0205;
   proceso[0].dir_fin=0x0300;
   proceso[1].dir_inicio=0x0305;
   proceso[1].dir_actual=0x0305;
   proceso[1].dir_fin=0x0400;
   proceso[0].estado=0;
   proceso[1].estado=0;
}

void version(){
   printf("Version de Software: v1");
   }

//*****************************************

