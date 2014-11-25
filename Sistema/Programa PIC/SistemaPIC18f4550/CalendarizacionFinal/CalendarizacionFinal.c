#include <18F4550.h>
#device   adc=10

#use    delay(clock=20000000)
#use    rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7)
#fuses  HS,NOPROTECT,NOWDT,NOLVP,PUT           
//#include <lcd.c>
#define Tiempo 200

#byte PORTA =0x80   // 0b10000000   
#byte PORTB =0x81   // 0b10000001 
#byte PORTC =0x82   // 0b10000010
#byte PORTD =0x83   // 0b10000011
#byte PORTE =0x84   // 0b10000100
#byte TRISA =0x92   // 0b10100010
#byte TRISB =0x93   //
#byte TRISC =0x94   // 
#byte TRISD =0x95   //
#byte TRISE =0x96   //

#byte TOSU=0xFFF
#byte TOSH=0xFFE
#byte TOSL=0xFFD
#byte STKPTR=0xFFC  
#byte PCLATU=0xFFB  
#byte PCLATH=0xFFA   
#byte PCL=0xFF9    
#byte TMR0H=0xFD7
#byte TMR0L=0xFD6
#byte TMR1H=0xFD9
#byte TMR1L=0xFD4
#byte T0CON=0xFD5
#byte T1CON=0xFD6
#byte bsr=0xFE0
#byte wreg=0xFE8
#byte status=0xFD8
#byte RCON=0xFD0
#byte INTCON=0xFF2

#byte ADCON0=0x1F
#bit  ANALOGICO= ADCON0.3
#bit  TMR0IE=INTCON.5
#bit  TMR0IF=INTCON.2
#bit  TMR1IE=INTCON.5
#bit  TMR1IF=INTCON.2
#bit  IPEN=RCON.7

// ==================== VARIABLES DEL BUFFER ==============================
int8 const size_buffer=32;
int    x_buffer=0x00; 
char   c_buffer[size_buffer]; 
char   name[4];
float quantum=0.025;     

// ===== VARIABLES DEL Sistema Operativo ======= \\

int8 colap[20];
int8 np=0; 
// int i, ii,ii1;
int8 i, ii, ii1, ii2, ii3, ii4, ii5;
int8 busc;
int32  result=0; 
int8 h1, h2, h3, h4, h5;
int8 f1, f2, f3, f4, f5;

// ====  VARIABLES PARA ENVIAR AL ADMINISTRADOR DE PROCESOS
int8   idenv;         // ID del proceso
char   nomenv[5];     // NOMBRE del proceso
int8   estenv;        // ESTADO del proceso
int32  direnv;        // DIR_INI del proceso
int32  diraenv;       // DIR_ACT del ptoceso
int32  quantenv;      // NUMERO QUANTUM del proceso
 
int8 opcion;

// Variables del convertidor ADC
float value;
int16 value1;
//=============================

int32 dirini ;
int8 killp;

int8 k1,k2,k3,k4,k5,k6;
int8 l1,l2,l3,l4,l5,l6;

int8 m;

struct pcb{
   int32 dir_ini;     // Direccion inicial  TOSU TOSH TOSL
   // int32 dir_fin;     // Direccion final    TOSU TOSH TOSL
   int32 dir_cor;     // Direccion actual   TOSU TOSH TOSL
   int8 estado;        // Descargado=0; CargadoSinIniciar=1; Cargado=2;
   int8 wreg;          // Almacena el registro de trabajo WREG
   int8 status;        // Almacena el estado del procesador STATUS
   int8 bsr;           // Almacena los datos del estado del proceso BSR
   int8 id;            // ID identificador o numero del proceso  
   int8 prioridad;    // prioridad del procesos en ejecucion 
   int32 tiem_eje;    // Tiempo de ejcucion del proceso
   char nombre[5];    // Nombre del proceso 
}  procesos[13]; 

// ======== VARIABLES DEL CALENDARIO ========== \\
// estado_calendario = 0 -->   No ejecutar 
// estado_calendario = 1 -->   Ejecutar 
// estado_calendario = 2 -->   Vacio  

int8 calendario[1300];
int reloj = 0;
int long_calendario;
int estado_calendario = 0;
//=========================
int8 band_calendario=0;
int8 band_cal_ejec=0;
int16 ind_cal=0;
int16 mcm=0;
int bandera=0;
//=============================


void iniciar_quantum()
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
   enable_interrupts(INT_TIMER0);
}

void fin_quantum(){
   disable_interrupts(INT_TIMER0);
}

Int1 cont=0;
Int1 flag_1 = 0;

#INT_TIMER0  //interrupcion la funcion a ejecutarse por el timer 0
void inter_quantum()
{  
   fin_quantum();
   if(cont == 0){output_bit(PIN_B7,1); cont =1;}
   else{output_bit(PIN_B7,0);cont = 0;}
   procesos[colap[0]].wreg=wreg;
   procesos[colap[0]].dir_cor = (int32)TOSL;
   procesos[colap[0]].dir_cor = procesos[colap[0]].dir_cor|((int32)TOSH<<8);
   procesos[colap[0]].dir_cor = procesos[colap[0]].dir_cor|((int32)TOSU<<16);
   
   if(np>1)
   { 
   
  //     if(procesos[colap[0]].prioridad == 1){
           colap[np] = colap[0];
        for(i=0;i<np;i++)
        {
           colap[i]=colap[i+1];
        }
     // }
   }
  
//==== PC=S0_Init() =======
   TOSL=0x00;
   TOSH=0x30;
   TOSU=0x00;
}  




//================== FUNCIONES METEMATICA DE POTNCIA ===========================
int32 pot_num(int16 base, int exp){
   int32 pot=1;
   int q=0;
   for (q=0;q<exp;q++) { pot=pot*base;}
   return pot;
}
//==============================================================================

void cmd_process(void){
            char   cmd[size_buffer];         // Comando
            strcpy(cmd,c_buffer);    
            // printf(cmd);
   if(x_buffer == 9 && band_calendario==0)
   {
       for(i = 0;i<6;i++){ name[i] = cmd[i+5]; }
 
       for( i=0; i<5; i++){
         result = result + ((cmd[i])-48)*pot_num(10,(5-i-1))+0.1;
       }
    
   } 
   else {
        
       for( i=0; i<x_buffer; i++){
            result = result + ((cmd[i])-48)*pot_num(10,(x_buffer-i-1))+0.1;
       }
         
   }  
   
  cmd ="";
  x_buffer=0x00;  // Borro buffer.
   
   if ( band_calendario == 1 )
   {
      calendario[ind_cal] = result;
      //  printf("%u ",result);
      ind_cal++;
      result=0;
   }
   
 }


void aux(char c)
{
   switch(c)
   {
  case 'C':
     band_calendario=1;
     mcm=0;
     break;
   case 'F':
     band_calendario=0;
     mcm = ind_cal;
    //  printf("%Lu",mcm);
     ind_cal=0;
     break;
   }
}


void add_to_buffer(char c){
   switch(c){
      case 'W':     // Enter -> Habilita Flag para procesar comando
          cmd_process();
      break;
      default:      // A�ade caracter recibido al Buffer
           c_buffer[x_buffer++]=c;
      break;
   }
}

// =================== INTERRUPCION SERIAL PARA RECIVIR LAS DIRECCIONES DE INICIO =======================
#INT_RDA    
void Data_Received(){
    char  data_r = 0x00; 
          data_r = getc();
          
   if ( data_r == 'C' || data_r == 'F')
       aux(data_r);
   else    
       add_to_buffer(data_r);
          
}



//***********************************************************************
//======================== Parpadearled B0 ====================================
#ORG 0x1000, 0x1044
void Parpadear_B0()   // 4096
{
   while(TRUE)
   {
      output_bit(PIN_B0,1);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      // for (k1=1;k1<50;++k1){for (l1=1;l1<50;++l1){}}
      output_bit(PIN_B0,0);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}    
      //for (k1=1;k1<50;++k1){for (l1=1;l1<50;++l1){}}
   }
}
 
//========================= Parpadear led B1 ===================================
#ORG 0x1046, 0x1090    // 4166
void Parpadear_B1()
{
   while(TRUE)
   {
         output_bit(PIN_B1,0);
         for (k2=1;k2<170;++k2){for (l2=1;l2<170;++l2){}}
         output_bit(PIN_B1,1);
         for (k2=1;k2<170;++k2){for (l2=1;l2<170;++l2){}}    
   }
}

//============================ Parpadear led B2 ================================
#ORG 0x1092, 0x10D6     // 4242
void Parpadear_B2()
{
   while(TRUE)
   { output_bit(PIN_B2,0);
         for (k3=1;k3<200;++k3){for (l3=1;l3<200;++l3){}}
         output_bit(PIN_B2,1);
         for (k3=1;k3<200;++k3){for (l3=1;l3<200;++l3){}}    
   }
}

//============================= Parpadear led B3 ===============================
#ORG 0x10D8, 0x111C     // 4312
void Parpadear_B3()
{
   while(TRUE)
   { output_bit(PIN_B3,0);
         for (k4=1;k4<200;++k4){for (l4=1;l4<200;++l4){}}
         output_bit(PIN_B3,1);
         for (k4=1;k4<200;++k4){for (l4=1;l4<200;++l4){}}    
   }
}

//===================== Parpadear led B4 "FINITO" ==============================
/*#ORG 0x111E, 0x11F4      // 4382 
void Parpadear_B4()
{
   for ( ii=0; ii<50; ii++)
   {
         output_bit(PIN_B4,1);
         for (k5=1;k5<200;++k5){for (l5=1;l5<200;++l5){}}
         output_bit(PIN_B4,0);
         for (k5=1;k5<200;++k5){for (l5=1;l5<200;++l5){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}

//===================== Parpadear led B5 "FINITO" ==============================
#ORG 0x11F6, 0x12CC      // 4598
void Parpadear_B5()
{
   for ( ii1=0; ii1<50; ii1++)
   {
         output_bit(PIN_B5,1);
         for (k6=1;k6<200;++k6){for (l6=1;l6<200;++l6){}}
         output_bit(PIN_B5,0);
         for (k6=1;k6<200;++k6){for (l6=1;l6<200;++l6){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}
*/
/*

//========================== Leer ADC ==========================================

#ORG 0x12CE, 0x13F2     // 4814
void leer_ADC()
{  setup_adc_ports(0|vss_vdd);
   setup_adc(adc_clock_internal);
   while(1)
   {
      set_adc_channel(0);
      delay_us(40);
      value1=Read_ADC();
      value = 5*(value1);
      value = value/1023.0;
      printf(lcd_putc,"\n Voltage=   %f",value);
      delay_ms(500);
   }
}

//================== Contador 7 segmentos==MEMORIA LIBRE =======================

#ORG 0x13F4, 0x1620     // 4896
void Contador()
{  
   while(1)
   {
      printf("no implementado");
   }
}

*/

//==============================================================================   
//======================== PROCESOS DEL CALENDARIO =============================
//==============================================================================
#ORG 0x5000, 0x50FF
void proceso_1()
{
   for ( ii1=0; ii1<2; ii1++)
   {
      output_bit(PIN_A1,1);
      for (h1=1;h1<50;++h1){for (f1=1;f1<50;++f1){}}
      output_bit(PIN_A1,0);
      for (h1=1;h1<50;++h1){for (f1=1;f1<50;++f1){}}  
         
   }
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}


#ORG 0x5100, 0x51EE
void proceso_2()
{
   for ( ii2=0; ii2<3; ii2++)
   {
         output_bit(PIN_A2,1);
         for (h2=1;h2<50;++h2){for (f2=1;f2<50;++f2){}}
         output_bit(PIN_A2,0);
         for (h2=1;h2<50;++h2){for (f2=1;f2<50;++f2){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}


#ORG 0x5200, 0x52EE
void proceso_3()
{
   for ( ii3=0; ii3<15; ii3++)
   {
         output_bit(PIN_A3,1);
         for (h3=1;h3<50;++h3){for (f3=1;f3<70;++f3){}}
         output_bit(PIN_A3,0);
         for (h3=1;h3<50;++h3){for (f3=1;f3<70;++f3){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}

#ORG 0x5300, 0x53EE
void proceso_4()
{
   for ( ii4=0; ii4<20; ii4++)
   {
         output_bit(PIN_A4,1);
         for (h4=1;h4<50;++h4){for (f4=1;f4<80;++f4){}}
         output_bit(PIN_A4,0);
         for (h4=1;h4<50;++h4){for (f4=1;f4<80;++f4){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}

#ORG 0x5400, 0x54EE
void proceso_5()
{
   for ( ii5=0; ii5<23; ii5++)
   {
         output_bit(PIN_A5,1);
         for (h5=1;h5<50;++h5){for (f5=1;f5<50;++f5){}}
         output_bit(PIN_A5,0);
         for (h5=1;h5<50;++h5){for (f5=1;f5<50;++f5){}}    
   }
   
   fin_quantum();
   procesos[colap[0]].estado =0;
   for (i = 0;i<np;i++){
   colap[i]=colap[i+1];}
   np--;
   goto_address(0x3000);
}

//==============================================================================
//==============================================================================
void Port_Init()
{  
   output_bit(PIN_A0,0);
   output_bit(PIN_A1,0);
   output_bit(PIN_A2,0);
   output_bit(PIN_A3,0);
   output_bit(PIN_B2,0);
   output_bit(PIN_B3,0);
   set_tris_b(0x00);      //  Salidas de los LEDS
   set_tris_d(0x00);      //  Salidas del LCD
   set_tris_a(0x00);      //  todas salidas LEDS

}

void interrupt_Init()
{
   enable_interrupts(GLOBAL);      // habilitacion de las interrupciones globales
   enable_interrupts(INT_RDA);     // habilita interrupcion del serial rs232
}

void TMR0_Config()
{
    T0CON=0b10000111; // habilita el enable del timer 0, con un preescalado de 1/256
   // T0CON=0b11000111; // preescalado de 1/128
}

void Procesos_Init()
{  
   for(i=0;i<20;i++)
   {
      procesos[i].estado=0;
      procesos[i].nombre[4] = "";
   }  //todos los procesos al inicio estan descargados     
}



//======================= SISTEMA OPERATIVO S.O. =================================
#ORG 0x3000, 0x4800
void SO_Init(){
   fin_quantum();
   while (1){  
           opcion = result%10; 
      switch(opcion){
  
            case 1: {
                   dirini = (result-1)/10;   
              for ( i=0; i<np; i++){ 
                    if ( procesos[colap[i]].dir_ini == dirini){ flag_1 = 1; break;}
              }    
              if(flag_1 == 0){
                   np++;
                   busc = np;
                  for ( i=1; i<np; i++){
                       if ( procesos[i].estado == 0){ busc = i; break;}
                   }
                  printf(dirini); 
                  colap[np-1] = busc ;
                  procesos[colap[np-1]].dir_ini= dirini;
                  procesos[colap[np-1]].estado = 1;      // 1=proceso cargado
                  procesos[colap[np-1]].id = busc ;
                  procesos[colap[np-1]].tiem_eje=0;
                  procesos[colap[np-1]].prioridad = 1;  
                  strcpy(procesos[colap[np-1]].nombre,name);   // copia el nombre y lo gyuarda 
                  //printf(lcd_putc, "\f CARGADO: %i ", np);
             }
               result=0;
               flag_1 = 0;
               busc = 0 ;
               break;
            }
            case 2: {
                  printf("\n ");
                  killp=(result-2)/10;
               for( i=0; i<np; i++ ){
                    if(procesos[colap[i]].id == killp){ busc = i; break;}
               }
                  procesos[colap[busc]].estado=0;
                  //printf(lcd_putc, "\f desc. %u ", killp);
                  printf("\n proceso descargado   : %u  ", killp);
                  printf("\n ");
               for( i=busc; i<np; i++ ){
                  colap[i]=colap[i+1];
               }
                   np--;
                   result=0;
                   busc = 0;
                   break;
               }
             case 3: {
                    
                  //   printf("\n 'SISTEMAS EN TIEMPO REAL MT-325 UNI-FIM'  ");
                    printf("\n 'S.O. Version 3.2' \n"); 
                    //lcd_gotoxy(1,1);
                    //lcd_putc("S.O. Version 3.2");
                    result=0;
                    break;
             
             }
             case 4: {
             
                  for ( i=0; i<np; i++ ){
                     idenv =  procesos[colap[i]].id;
                     strcpy(nomenv,procesos[colap[i]].nombre); 
                     estenv = procesos[colap[i]].estado;  
                     direnv = procesos[colap[i]].dir_ini;
                     diraenv = procesos[colap[i]].dir_cor;
                     quantenv = procesos[colap[i]].tiem_eje;
                     printf("%u + %s + %u + %Lu + %Lu + %Lu",idenv,nomenv,estenv,direnv,diraenv,quantenv);
                     printf("\r\n");
                     delay_ms(18);
                     }
                     result=0;
               break;
              }
               case 5: {
               // Se cargan los procesos de la base de datos
               for ( i=9; i<14; i++){
                  switch (i){
                     case 9:
                        procesos[i].dir_ini=0x5000;
                        break;
                     case 10:
                        procesos[i].dir_ini=0x5100;
                        break;
                     case 11:
                        procesos[i].dir_ini=0x5200;
                        break;
                     case 12:
                        procesos[i].dir_ini=0x5300;
                        break;
                     case 13:
                        procesos[i].dir_ini=0x5400;
                        break;
                  }
                  procesos[i].estado=1;       // 1 = proceso cargado
                  procesos[i].id=i;
                  procesos[i].tiem_eje=0;
                  procesos[i].prioridad =5;
               }
               
               result =0;
               estado_calendario = 1; 
               printf("Procesos cargados Exitosamente");
                   break;
               }
               case 6: {
                  // Ejecutar calendario
                   band_cal_ejec=1;   // activa la bandera de ejecucion del calendario 
                   estado_calendario = 2;
                   reloj = 0;
                   ind_cal = 0;
                   bandera = 0;  
                   result = 0;
                 break;
               }              
               case 7: {
                  // Detener ejecucion del calendario                 
                   estado_calendario = 0;          
                   band_cal_ejec = 0; 
                   result =0;
                   break;
               }
               case 8: {
                  // Eliminar calendario
                  // Liberar los PCBs 
                   int c1;
                   for(c1=0;c1<15;c1++){
                      if(procesos[i].prioridad == 5){
                      procesos[i].estado = 0;
                      }
                   }
                     estado_calendario = 2;
                     result =0; 
                 break;
               }
               case 9: {
                   // Muestra el vector calendario  
                   int mos,p;
                   for( p = 0; p<mcm;p++){
                      mos = calendario[p];
                      printf("%u",mos);
                   }
                     result = 0;   
                  // printf("Vector Calendarizado");
                   break;
               }
               default:break;
        }
        
       
    if (estado_calendario == 2 && mcm != 0)
    {
            if ( reloj == 0 )  // "reloj"
            {
               for ( i=0; i< np; i++ )
               {
                  colap[np-i] = colap[np-i-1];
               }
               
               colap[0] = calendario[reloj] + 8;
               
               if ( bandera == 1)
               {
                  if ( calendario[mcm-1] == 0)
                     np++;
               }
               
               else
                  np++;
               procesos[colap[0]].estado=1;
            
            }
            else
            {
               if ( calendario[reloj-1] == calendario[reloj] )
               {
                  if ( calendario[reloj] != 0 )
                  {
                     for ( i=0; i< np; i++ )
                     {
                        colap[np-i] = colap[np-i-1];
                     }
                     colap[0] = calendario[reloj] + 8;
                  }
               }
               else
               {
                  //np--;
                  if ( calendario[reloj] != 0)
                  {
                     np++;
                     
                     for ( i=0; i< np; i++ )
                     {
                        colap[np-i] = colap[np-i-1];
                     }
                     colap[0] = calendario[ind_cal] + 8;
                     procesos[colap[0]].estado=1;
                  }
               }
            }
            reloj++;
            if ( ind_cal == mcm )
            {
               ind_cal = 0;
               bandera = 1;
            }
         }
           
              
        
         if(np!=0){   
            if(procesos[colap[0]].estado==1)
            {
               procesos[colap[0]].estado=2;
               procesos[colap[0]].tiem_eje++;
               iniciar_quantum();
               goto_address(procesos[colap[0]].dir_ini);
            }
         
            if(procesos[colap[0]].estado==2)
            {
               wreg=procesos[colap[0]].wreg;
               procesos[colap[0]].tiem_eje++;
               iniciar_quantum();
               goto_address(procesos[colap[0]].dir_cor);
            }
         }
   }
}  

//================ PROGRAMA PRINCIPAL DE INICIO ================================

void main(void)
{  
   Port_Init();
   Procesos_Init();
   Interrupt_Init();
   TMR0_Config();
   //lcd_init();
   setup_adc_ports(AN0);
   setup_adc(ADC_CLOCK_DIV_32 );
   //lcd_putc("Sistema Iniciado");
   //lcd_gotoxy(1,2);
   //lcd_putc("=== MT - 325 ===");
   SO_Init();
}
//==============================================================================


