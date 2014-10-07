#include <18F4550.h>

#use delay(clock=10000000)
#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7,bits=8,parity=N)
#fuses HS,NOPROTECT,NOWDT,NOLVP,CPUDIV1,nomclr
#include "lcd.c"
#define Tiempo 200

#byte T0CON=0xFD5
#byte wreg=0xFE8
#byte TOSU=0xFFF
#byte TOSH=0xFFE
#byte TOSL=0xFFD

/*

#byte PORTA =0x80
#byte PORTB =0x81
#byte PORTC =0x82
#byte PORTD =0x83
#byte PORTE =0x84
#byte TRISA =0x92
#byte TRISB =0x93
#byte TRISC =0x94
#byte TRISD =0x95
#byte TRISE =0x96


#byte TOSU=0xFFF
#byte TOSH=0xFFE
#byte TOSL=0xFFD
#byte STKPTR=0xFFC
#byte PCLATU=0xFFB
#byte PCLATH=0xFFA
#byte PCL=0xFF9
#byte TMR0H=0xFD7
#byte TMR0L=0xFD6
#byte T0CON=0xFD5
#byte bsr=0xFE0
#byte wreg=0xFE8
#byte status=0xFD8
#byte RCON=0xFD0
#byte INTCON=0xFF2

#byte ADCON0=0x1F
#bit ANALOGICO= ADCON0.3
#bit TMR0IE=INTCON.5
#bit TMR0IF=INTCON.2
#bit IPEN=RCON.7
*/
int np=0;
int colap[6];
int m;
int8 i,j,j1,j2,j3,j4;
int8 dis1,dis2;

int c1,c2;
int k1,k2,k3,k4,k5,k6;
int l1,l2,l3,l4,l5,l6;

char proceso;
int SHOW=0;
int d1=10;
int d2=0;

int const lenbuff=10;
int32 valor=0;
int32 valor2=0;
int32 val1,val2,val3,val4,val5;
int16 quantum=65048;


int   xbuff=0x00;
char  cbuff[lenbuff];
char  rcvchar=0x00;
char  comando;
int1  flagcmd=0;


struct pcb{
int32 di;
int32 df; 
int32 da;
int estado; 
int wreg; 
int status;
int bsr;
int id;
} vpcb[7];

void activar_timer()
{
   set_timer0(quantum);
   enable_interrupts(INT_TIMER0);
}

void desactivar_timer()
{
   disable_interrupts(INT_TIMER0);
}

int32 DISPLAY(INT NUMERO){
   IF(NUMERO==0){SHOW=0xBF;};
   IF(NUMERO==1){SHOW=0x86;};
   IF(NUMERO==2){SHOW=0xDB;};
   IF(NUMERO==3){SHOW=0xCF;};
   IF(NUMERO==4){SHOW=0xE6;};
   IF(NUMERO==5){SHOW=0xED;};
   IF(NUMERO==6){SHOW=0xFD;};
   IF(NUMERO==7){SHOW=0x87;};
   IF(NUMERO==8){SHOW=0xFF;};
   IF(NUMERO==9){SHOW=0xE7;};
   return(SHOW);
}

#INT_TIMER0
void inter_quantum()
{  int i=0;
   desactivar_timer();
   vpcb[colap[0]].wreg=wreg;
   vpcb[colap[0]].da=(int32)TOSL;
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSH<<8);
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSU<<16);
   if(np>1)
   {  m=colap[0];
      for(i=0;i<np-1;i++)
      {colap[i]=colap[i+1];
      }
      colap[np-1]=m;
   }
   
   //pc=inicio_S0;
   
   TOSL=00;
   TOSH=40;
   TOSU=00;
}  

void  addcbuff(char c);
void mostrar_procesos();
void descargar_proceso(int idp);
void ir_quantum();
void Config_TMR0();

#INT_RDA
void serial_isr()
{
   printf("Recibido");
   rcvchar = 0x00;
   if(kbhit())
   { 
      rcvchar = getc();
      addcbuff(rcvchar);
   }
}

void addcbuff(char c)
{
   switch(c)
   {
      //case 0x2A:
      case '*':
         cbuff[xbuff++] = c;
         flagcmd = 1;
      break;
      default:
         cbuff[xbuff++] = c;
   }
}

void inicbuff(void)
{
   int i;
   for (i=0; i<lenbuff;i++)
   {
      cbuff[i] = 0x00;
   }
   xbuff=0x00;
}

void procesa_cmd(void){
 
   int i, j=0;
   char salida[lenbuff];                   // Argumento de comando (si lo tiene)
   disable_interrupts(int_rda);
   flagcmd=0;                       // Desactivo flag de comando pendiente.
   for (i=0;i<xbuff;i++)
   {
      if (cbuff[i] == '$')
      //while((cbuff[i] != 0x4A) & (i<xbuff))
      while((cbuff[i] != '*') & (i<xbuff))
      {
         salida[j++]=cbuff[i++];
      }
      comando = salida[1];
      valor2 = salida[3] -'0' + 1;
      //printf ("\r\n Valor:  %Lu\n", valor);
      val1 = salida[2] - '0';
      val1 = val1;
      val2 = salida[3] - '0';
      val2 = val1*10 + val2;
      val3 = salida[4] - '0';
      val3 = val2*10 +val3;
      val4 = salida[5] - '0';
      val4 = val3*10+val4;
      val5 = salida[6]-'0';
      valor = val4*10 + val5;
      //valor = valor*10;
      //printf ("\r\n Valor:  %Lu\n", valor);
      //printf ("\r\n Valor2:  %Lu\n", valor2);
      //printf ("\r\n Comando:  %c\n", comando);
      //i++;
      break;
   }
   inicbuff();                            // Borro buffer.
   for(i=0;i<lenbuff;i++)                 // Bucle que pone a 0 todos los
   {
      salida[i]=0x00;                     // caracteres en el argumento
   }
   enable_interrupts(int_rda);
}



//PROGRAMA 1: Luces Secuenciales - Bucle Infinito

#ORG 0x1000, 0x1100
void ROML_LedA1()
{
   set_tris_a(0xFF);
   while(TRUE)
   {
      for (j1=0;j1<3;++j1)
      {
         IF(j1==0)
         {
            OUTPUT_BIT(PIN_A0,0);
            OUTPUT_BIT(PIN_A1,0);
         }
         IF(j1==1){OUTPUT_BIT(PIN_A0,1);}
         IF(j1==2){OUTPUT_BIT(PIN_A1,1);}
         for (k1=1;k1<200;++k1){for (l1=1;l1<200;++l1){}}
      }
   }
}

//PROGRAMA 2: Luces Secuenciales 2 - Bucle Infinito

#ORG 0x1102, 0x1200
void ROML_LedA2()
{
   set_tris_a(0xFF);
   while(TRUE)
   {
      for (j2=0;j2<3;++j2)
      {
         IF(j2==0)
         {
            OUTPUT_BIT(PIN_A2,0);
            OUTPUT_BIT(PIN_A3,0);
         }
         IF(j2==1)
         {
            OUTPUT_BIT(PIN_A2,1);
         }
         IF(j2==2)
         {
            OUTPUT_BIT(PIN_A2,0);
            OUTPUT_BIT(PIN_A3,1);
         }         
         for (k2=1;k2<200;++k2){for (l2=1;l2<200;++l2){}}
      }
   }
}
               

//PROGRAMA 3: Display 1 (De 9 a 0) - Bucle Infinito

#ORG 0x1202, 0x1300
void ROML_LedA3(){
while(TRUE){
   for (d1=10;d1>0;d1--)
   {
         dis1=DISPLAY(d1-1);
         for (k3=1;k3<200;++k3)
         {
            for (l3=1;l3<200;++l3)
            {
               output_b(dis1);
               output_bit(PIN_D0,1);
               output_bit(PIN_D1,0);
            }
         }
         if(d1==0){d1=10;}   
   }
}
}

//PROGRAMA 4: Display 2 (De 0 a 9) - Finito

#ORG 0x1302, 0x1400
void voidTask_LedB1(){
while(true){
   for (d2=0;d2<10;d2++)
   {
         dis2=DISPLAY(d2);
         for (k4=1;k4<200;++k4)
         {
            for (l4=1;l4<200;++l4)
            {
               output_b(dis2);
               output_bit(PIN_D0,0);
               output_bit(PIN_D1,1);
            }
         }
         if(d2==10){d2=0;}   
   }
comando='D';
valor2=4;
}
}

//PROGRAMA 5: Luces Secuenciales C - Finito

#ORG 0x1402, 0x1500
void voidTask_LedC1(){
while(TRUE){
   while(TRUE)
   {
      for (c1=0;c1<5;c1++)
      {
         for (j3=0;j3<3;++j3)
         {
            IF(j3==0)
            {
               OUTPUT_BIT(PIN_A4,0);
               OUTPUT_BIT(PIN_A5,0);
            }
            IF(j3==1)
            {
               OUTPUT_BIT(PIN_A4,1);
            }
            IF(j3==2)
            {
               OUTPUT_BIT(PIN_A4,0);
               OUTPUT_BIT(PIN_A5,1);
            }         
            for (k5=1;k5<200;++k5){for (l5=1;l5<200;++l5){}}
         }
      }
      comando='D';
      valor2=5;
   }
}
}


//PROGRAMA 6: Luces Secuenciales C2 - Finito

#ORG 0x1502, 0x1600
void voidTask_LedC2(){
while(TRUE){
   while(TRUE)
   {
      for (c2=0;c2<5;c2++)
      {
         for (j4=0;j4<3;++j4)
         {
            IF(j4==0)
            {
               OUTPUT_BIT(PIN_D2,0);
               OUTPUT_BIT(PIN_D3,0);
            }
            IF(j4==1)
            {
               OUTPUT_BIT(PIN_D2,1);
            }
            IF(j4==2)
            {
               OUTPUT_BIT(PIN_D2,0);
               OUTPUT_BIT(PIN_D3,1);
            }         
            for (k6=1;k6<200;++k6){for (l6=1;l6<200;++l6){}}
         }
      }
      comando='D';
      valor2=6;
   }
}
}


// PROGRAMA 7: Mostrar Procesos

#ORG 0x2502, 0x2850
void task_manager(){
while(TRUE){
desactivar_timer();
int m;
   int32 mproceso, mdirini, mdirfin, mdiract;
   printf("\r\n|   Proceso  |  Dir.Ini  |  Dir.Fin  |  Dir.Act  |");
   for(m=0;m<np;m++)
   {
      mproceso = colap[m]+1;
      mdirini = vpcb[mproceso-1].di;
      mdirfin = vpcb[mproceso-1].df;
      mdiract = vpcb[mproceso-1].da;
      printf("\r\n|      %Lu     |   %Lu    |   %Lu    |   %Lu    |",mproceso,mdirini,mdirfin,mdiract);
   }
   printf("*");
activar_timer();
int16 mm;
for (mm=0;mm<250;mm++)
{
   int mmm;
   for (mmm=0;mmm<250;mmm++)
   {
      mm=mm+1;
      mm=mm-1;
   }
}

}

}

void Port_Init()
{
   set_tris_b(0x00);
   set_tris_d(0x00);
   set_tris_a(0x00);
}

void inte_Init()
{
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_RDA);
}

void Config_TMR0()
{
   T0CON=0b10000111;

}

void Procesos_Init()
{
   vpcb[0].di=0x1000;
   vpcb[0].da=0x1000;
   vpcb[0].df=0x1100;
   vpcb[1].di=0x1102;
   vpcb[1].da=0x1102;
   vpcb[1].df=0x1200;
   vpcb[2].di=0x1202;
   vpcb[2].da=0x1202;
   vpcb[2].df=0x1300;
   vpcb[3].di=0x1302;
   vpcb[3].da=0x1302;
   vpcb[3].df=0x1400;
   vpcb[4].di=0x1402;
   vpcb[4].da=0x1402;
   vpcb[4].df=0x1500;
   vpcb[5].di=0x1502;
   vpcb[5].da=0x1502;
   vpcb[5].df=0x1600;
   vpcb[6].di=0x2502;
   vpcb[6].da=0x2502;
   vpcb[6].df=0x2850;
   vpcb[0].estado=0;
   vpcb[1].estado=0;
   vpcb[2].estado=0;
   vpcb[3].estado=0;
   vpcb[4].estado=0;
   vpcb[5].estado=0;
   vpcb[6].estado=0;
   
}


void cargar_proceso(int idp)
{
    colap[np]=idp-1;
    vpcb[idp-1].estado=1;
    printf("\n\r Proceso %i cargado. *",idp);
    proceso=10;
    np++;
}

void descargar_proceso(int idp)
{
   int s,d,f;
   for(s=0;s<np;s++)
      {if(colap[s]==idp-1)
         {d=s;}
      }
   f=np-1;
   for(s=d;s<f;s++)
      {colap[s]=colap[s+1];
      }
   np--;   
   vpcb[idp-1].estado=0;
   proceso=0;
   printf("\n\r Proceso %i descargado. *",idp);
}

void ir_quantum()
{
   {  int i=0;
   desactivar_timer();
   vpcb[colap[0]].wreg=wreg;
   vpcb[colap[0]].da=(int32)TOSL;
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSH<<8);
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSU<<16);
   if(np>1)
   {  m=colap[0];
      for(i=0;i<np-1;i++)
      {colap[i]=colap[i+1];
      }
      colap[np-1]=m;
   }
   
   //pc=inicio_S0;
   
   TOSL=00;
   TOSH=40;
   TOSU=00;
}  
}



void MCU_Init()
{
   printf("Conectado.............*");
   Port_Init();
   Procesos_Init();
   inte_Init();
   Config_TMR0();
   lcd_init();
   setup_spi(FALSE);
   setup_psp(PSP_DISABLED);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
   port_b_pullups(FALSE);
}


#ORG 0x4000, 0x4FFE
void Inicio_SO()
{
   while (TRUE)
   {  //putc(0);        
      if(flagcmd) procesa_cmd();
      switch(comando)
      {
         case 'C':{if (valor2<10) cargar_proceso(valor2);break;}
         case 'D':{if (valor2<10) descargar_proceso(valor2);break;}
         case 'A':{printf("\r Valor de Acumulador:  %d*", wreg);}
         case 'X':{for (m=0;m<8;m++) cargar_proceso(m);
                   break;}
         
         case 'T':{quantum=valor;
                   printf ("\r Valor del Timer:  %Lu*", valor);
                  }
      }
      comando='-';
      valor2=100;
      valor=0;
       if(np>0)
       {
         if(vpcb[colap[0]].estado==1)
            {activar_timer();
             vpcb[colap[0]].estado=2;
             goto_address(vpcb[colap[0]].di);
            }
         else
            {wreg=vpcb[colap[0]].wreg;
             activar_timer();
             goto_address(vpcb[colap[0]].da);
            }
       }
   }
}


void main(void)
{
   output_a(0x00);
   output_b(0x00);
   //output_c(0x00);
   output_d(0x00);
   printf("Esperando conexi�n..*");
   
   MCU_Init();
   #asm
   call 0x4000
   #endasm
}
