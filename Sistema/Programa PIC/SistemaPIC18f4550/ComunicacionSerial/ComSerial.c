#include <18F4550.h>
#device adc=10       //Resolucion 10BITS
#use delay(clock=10000000)
#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7,bits=8,parity=N)
#fuses HS,NOPROTECT,NOWDT,NOLVP,CPUDIV1,nomclr
#define Tiempo 200

#use FAST_IO(A) 
#use FAST_IO(B)
#use FAST_IO(D)

#byte T0CON=0xFD5
#byte wreg=0xFE8
#byte TOSU=0xFFF
#byte TOSH=0xFFE
#byte TOSL=0xFFD

int np=0;
int colap[6];
int m,m1;
int8 i,j;

int8 k1,k2,k3,k4,k5,k6,k7;
int8 l1,l2,l3,l4,l5,l6,l7;
int cont;
int1 contar=0;

char proceso;

int const lenbuff=10;
int valor2=0;
int32 val1,val2,val3,val4,val5;
//int16 quantum=65048;

// Lectura ADC
int value;
int16 value1;


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
int prioridad;
} vpcb[10];

// ======== VARIABLES DEL CALENDARIO ========== \\
// estado_calendario = 0 -->   No ejecutar 
// estado_calendario = 1 -->   Ejecutar 
// estado_calendario = 2 -->   Vacio
// estado_calendario = 3 -->   Cargando

int8 calendario[1000];
int16 reloj = 0;
int estado_calendario = 0;
int prioridad_act = 0;
//=========================
int16 mcm=0;
//=============================


void activar_timer()
{
   // set_timer0(65478);    //    para 3ms
   // set_timer0(65438);    //    para 5ms
   // set_timer0(65380);    //    para 8ms
   // set_timer0(65341);    //    para 10ms
   // set_timer0(65282);    //    para 13ms
   // set_timer0(65243);    //    para 15ms
   // set_timer0(65185);    //    para 18ms
   // set_timer0(65146);    //    para 20ms
    set_timer0(65048);    //    para 25ms
   // set_timer0(64950);    //    para 30ms
   //set_timer0(quantum);
   enable_interrupts(INT_TIMER0);
}

void desactivar_timer()
{
   disable_interrupts(INT_TIMER0);
}

void addcbuff(char c);
void mostrar_procesos();
void descargar_proceso(int idp);
void Config_TMR0();


#INT_TIMER0
void inter_quantum()
{  int i=0;
   desactivar_timer();
   output_toggle(PIN_D6);
   // Guardando variables
   vpcb[colap[0]].wreg=wreg;
   vpcb[colap[0]].da=(int32)TOSL;
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSH<<8);
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSU<<16);
   
   // corriendo proceso
   if(np>1)
   {   
      prioridad_act = 0;
      for (i=1; i<np; i++ ){
         if (vpcb[colap[i]].prioridad > prioridad_act){
            m1 = i;
            prioridad_act = vpcb[colap[i]].prioridad;
         }
      }
      
      m=colap[0];
      colap[0] = colap[m1];
      for(i=m1;i<np-1;i++)
      {
         colap[i]=colap[i+1];
      }
      colap[np-1]=m;
   }
   
   /*if(contar == 1){
      cont++;
   }*/
   
   if(estado_calendario == 1){
      reloj++;
      if(reloj >= mcm){
         reloj = 0;
      }
   }
   
   //==== PC=S0_Init() =======
   TOSL=00;
   TOSH=40;
   TOSU=00;
} 

void inte_Init()  // interrupcion de puerto serial
{
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_RDA);
}


#INT_RDA
void serial_isr()
{
   //printf("Recibido");     // se envia por Serial una palabra para verificar interrupcion
   rcvchar = 0x00;
   rcvchar = getc();    // se recibe el caracter enviado por la computadora
   addcbuff(rcvchar);   // guarda el caracter en un buffer
}

void addcbuff(char c)   
{
   switch(c)
   {
      case '*':                  // en caso de recibir *, levanta el flag para saber que se termino de enviar un comando.
         cbuff[xbuff++] = c;
         flagcmd = 1;
      break;
      default:
         cbuff[xbuff++] = c;
   }
}

void inicbuff(void)           // inicializa el buffer, limpiandolo
{
   int i;
   for (i=0; i<lenbuff;i++)      
   {
      cbuff[i] = 0x00;
   }
   xbuff=0x00;
}


void procesa_cmd(void){             // procesa el ultimo comando guardado, un comando esta comprendido entre $ hasta * 
 
   int i, j=0;                      
   char salida[lenbuff];            // Argumento de comando (si lo tiene)
   disable_interrupts(int_rda);     
   flagcmd=0;                       // Desactivo flag de comando pendiente.
   for (i=0;i<xbuff;i++)
   {
      if (cbuff[i] == '$'){         // verifica que tenga '$'
         while((cbuff[i] != '*') & (i<xbuff))   // hasta que encuentre *
         {
            salida[j++]=cbuff[i++];
         }
         comando = salida[1];
         valor2 = salida[2] -'0';
         val1 = salida[3]-'0';
         val2 = salida[4]-'0';
         val3 = salida[5]-'0';
         
      }
   }
   inicbuff();                            // Borro buffer.
   for(i=0;i<lenbuff;i++)                 // Bucle que pone a 0 todos los
   {
      salida[i]=0x00;                     // caracteres en el argumento
   }
   enable_interrupts(int_rda);            
}


//*****************************************************************************
//PROGRAMA 1: Parpadeo Led D0

#ORG 0x1000, 0x1044
void Parpadear_D0()
{
   while(TRUE)
   {
      output_toggle(PIN_D0);
      for (k1=1;k1<150;k1++){for (l1=1;l1<200;l1++){}}
      output_toggle(PIN_D0);
      for (k1=1;k1<150;k1++){for (l1=1;l1<200;l1++){}}
   }
}

//*****************************************************************************
//PROGRAMA 2: Parpadeo Led D1

#ORG 0x1046, 0x1090       
void Parpadear_D1()
{
   while(TRUE)
   {
      output_toggle(PIN_D1);
      for (k2=1;k2<50;k2++){for (l2=1;l2<200;l2++){}}
      output_toggle(PIN_D1);
      for (k2=1;k2<50;k2++){for (l2=1;l2<200;l2++){}}
   }
}

//*****************************************************************************
//PROGRAMA 3: Parpadeo Led D2

#ORG 0x1092, 0x10D6
void Parpadear_D2()
{
   while(TRUE)
   {
      output_toggle(PIN_D2);
      for (k3=1;k3<100;k3++){for (l3=1;l3<200;l3++){}}
      output_toggle(PIN_D2);
      for (k3=1;k3<100;k3++){for (l3=1;l3<200;l3++){}}
   }
}

//*****************************************************************************
//PROGRAMA 4: Parpadeo Led D3 - FINITO

#ORG 0x10D8, 0x11D8
void Parpadear_D3()
{
   //cont = 0;
   //contar = 1;
   while(TRUE)
   {
      output_toggle(PIN_D3);
      for (k4=1;k4<200;k4++){for (l4=1;l4<100;l4++){}}
      output_toggle(PIN_D3);
      for (k4=1;k4<200;k4++){for (l4=1;l4<100;l4++){}}
      comando = 'D';
      valor2 = 4;
      //printf("T=%i*", cont);
      //contar = 0;
   }
}

//*****************************************************************************
//PROGRAMA 5: Parpadeo Led D4 - FINITO

#ORG 0x11DA, 0x12DA
void Parpadear_D4()
{
   //cont = 0;
   //contar = 1;
   while(TRUE)
   {
      output_toggle(PIN_D4);
      for (k5=1;k5<150;++k5){for (l5=1;l5<100;++l5){}}
      output_toggle(PIN_D4);
      for (k5=1;k5<150;++k5){for (l5=1;l5<100;++l5){}}
      comando = 'D';
      valor2 = 5;
      //printf("T=%i*", cont);
      //contar = 0;
   }
}


//*****************************************************************************
//PROGRAMA 6: Parpadeo Led B0 - FINITO

#ORG 0x12DC, 0x13DC
void Parpadear_B0()
{
   //cont = 0;
   //contar = 1;
   while(TRUE)
   {
      output_toggle(PIN_B0);
      for (k6=1;k6<100;k6++){for (l6=1;l6<100;l6++){}}
      output_toggle(PIN_B0);
      for (k6=1;k6<100;k6++){for (l6=1;l6<100;l6++){}}
      comando = 'D';
      valor2 = 6;
      //printf("T=%i*", cont);
      //contar = 0;
   }
}

//*****************************************************************************
//PROGRAMA 7: Parpadeo Led B1 - FINITO
#ORG 0x13DE, 0x14DE
void Parpadear_B1()
{
   //cont = 0;
   //contar = 1;
   setup_adc_ports(AN0_TO_AN3|vss_vdd);
   setup_adc(adc_clock_internal);
   while(TRUE)
   {
      /*output_toggle(PIN_B1);
      for (k7=1;k7<50;k7++){for (l7=1;l7<100;++l7){}}
      output_toggle(PIN_B1);
      for (k7=1;k7<50;k7++){for (l7=1;l7<100;++l7){}}*/
      set_adc_channel(0);
      delay_us(50);
      value1=Read_ADC();
      //value1 = 500;
      value1= (value1*250)/1023;
      value = value1;
      printf("A+0+%i*",value);
      delay_us(150);
      comando = 'D';
      valor2 = 7;
      for (k7=1;k7<250;k7++){for (l7=1;l7<250;++l7){}}
      //printf("T=%i*", cont);
      //contar = 0;
   }
}


//*****************************************************************************
// PROGRAMA 8: Mostrar Procesos

#ORG 0x2502, 0x2850
void task_manager(){
   while(TRUE){
      desactivar_timer();
      int m;
         int32 mproceso, mdirini, mdirfin, mdiract;
         printf("|   Proceso  |  Dir.Ini  |  Dir.Fin  |  Dir.Act  |*");
         for(m=0;m<np;m++)
         {
            mproceso = colap[m];
            mdirini = vpcb[mproceso].di;
            mdirfin = vpcb[mproceso].df;
            mdiract = vpcb[mproceso].da;
            mproceso++;
            printf("|      %Lu     |   %Lu    |   %Lu    |   %Lu    |*",mproceso,mdirini,mdirfin,mdiract);
         }
         //printf("*");
      comando='D';
      valor2=8;
      activar_timer();
   }
   
}

//*****************************************************************************
//PROGRAMA 9: Lectura ADC
#ORG 0x14E0, 0x1605
void leer_ADC()
{  //setup_adc_ports(0|vss_vdd);
   //setup_adc(adc_clock_internal);
   while(TRUE)
   {
      //set_adc_channel(0);
      //value1=Read_ADC();
      //delay_us(40);
      //value = 5*(value1);
      //value = value/1023.0;
      value = 50;
      printf("A+0+%i*",value);
      //delay_ms(50);
      comando = 'D';
      valor2 = 9;
   }
}



void limpiar_calendario (int mcm){
   for (int i=0; i<mcm; i++){
      Calendario[i] = 0;
   }
}

void cargar_proceso(int idp, int prioridad)
{
    colap[np]=idp-1;
    vpcb[idp-1].estado=1;
    vpcb[idp-1].prioridad =prioridad;
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
}

void Port_Init()
{
   set_tris_b(0x00);
   set_tris_d(0x00);
   set_tris_a(0x00);
   output_b(0x00);
   output_d(0x00);
   output_a(0x00);
}

void Config_TMR0()
{
   T0CON=0b10000111;

}

void Procesos_Init()
{
   //Programa 1
   vpcb[0].di=0x1000;
   vpcb[0].da=0x1000;   
   vpcb[0].df=0x1044;
   //Programa 2
   vpcb[1].di=0x1046;
   vpcb[1].da=0x1046;
   vpcb[1].df=0x1090;
   //Programa 3
   vpcb[2].di=0x1092;
   vpcb[2].da=0x1092;
   vpcb[2].df=0x10D6;
   //Programa 4
   vpcb[3].di=0x10D8;
   vpcb[3].da=0x10D8;
   vpcb[3].df=0x11D8;
   //Programa 5
   vpcb[4].di=0x11DA;
   vpcb[4].da=0x11DA;
   vpcb[4].df=0x12DA;
   //Programa 6
   vpcb[5].di=0x12DC;
   vpcb[5].da=0x12DC;
   vpcb[5].df=0x13DC;
   //Programa 7
   vpcb[6].di=0x13DE;
   vpcb[6].da=0x13DE;
   vpcb[6].df=0x14DE;
   //Programa 8
   vpcb[7].di=0x2502;
   vpcb[7].da=0x2502;
   vpcb[7].df=0x2850;
   //Programa 9
   vpcb[8].di=0x14E0;
   vpcb[8].da=0x14E0;
   vpcb[8].df=0x1605;
   
   vpcb[0].estado=0;
   vpcb[1].estado=0;
   vpcb[2].estado=0;
   vpcb[3].estado=0;
   vpcb[4].estado=0;
   vpcb[5].estado=0;
   vpcb[6].estado=0;
   vpcb[7].estado=0;
   vpcb[8].estado=0;
   
}

void MCU_Init()
{
   printf("Conectado.............*");
   Port_Init();
   Procesos_Init();
   inte_Init();
   Config_TMR0();
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
   {       
      if(flagcmd){
         procesa_cmd();
      }
      switch(comando)
      {
         case 'C': {    // Cargar proceso especifico
                     if (valor2<9) cargar_proceso(valor2 , 1);
                     printf("C+%i*", valor2);
                     delay_us(100);
                     break;
                   }
         case 'D': {    // Descargar proceso especifico
                     if (valor2<9) descargar_proceso(valor2);
                     printf("D+%i*", valor2);
                     delay_us(100);
                     break;
                   }
         case 'U': {    // Cargar procesos 1-6
                     for (m=1;m<4;m++) {cargar_proceso(m , 1);}
                     break;
                   }
         case 'X': {    // Descargar procesos 1-6
                     for (m=1;m<4;m++) {descargar_proceso(m);}
                     break;
                   }
         case 'M': {    // iniciar/detener carga del calendario
                     if (valor2 == 0) {
                        estado_calendario = 3;
                        mcm = val3+(val2*10)+(val1*100);
                        limpiar_calendario (mcm);
                        printf("M+0*");
                     } else if ( valor2 == 1 ){
                        estado_calendario = 0;
                        printf("M+1*");
                     }
                     break;
                   }
         case 'P': {    // Guardar un proceso en el calendario
                     int pos;
                     pos = val3+(val2*10)+(val1*100);
                     Calendario[pos] = valor2;
                     printf("P+%i*",valor2);
                     break;
                   }
         case 'T': {    // Ejecutar calendario
                     estado_calendario = 1;
                     reloj = 0;
                     printf("T+1*");
                     break;
                   }
         case 'R': {    // Detener Calendario
                     estado_calendario = 0;
                     printf("R+1*");
                     break;
                   }
         case 'V': {    // Version
                     printf("Version del Sistema: V3.0*");
                     break;
                   }
      }
      comando='-';
      valor2=100;
      if (estado_calendario == 1){
         if (Calendario[reloj] != 0){
            cargar_proceso(Calendario[reloj] , 2);
         }
      }
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
   printf("Esperando conexi�n..*");
   
   MCU_Init();
   #asm
   call 0x4000
   #endasm
}

