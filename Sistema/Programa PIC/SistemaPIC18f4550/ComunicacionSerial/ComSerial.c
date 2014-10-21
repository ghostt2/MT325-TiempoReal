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

int const lenbuff=10;

int   xbuff=0x00;
char  cbuff[lenbuff];
char  rcvchar=0x00;
char  comando;
int1  flagcmd=0;
int32 valor=0;
int32 valor2=0;
int32 val1,val2,val3,val4,val5;



void  addcbuff(char c);
void inte_Init()  // interrupcion de puerto serial
{
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_RDA);
}


#INT_RDA
void serial_isr()
{
   printf("Recibido");     // se envia por Serial una palabra para verificar interrupcion
   rcvchar = 0x00;
   rcvchar = getc();    // se recibe el caracter enviado por la computadora
   addcbuff(rcvchar);
}

void addcbuff(char c)
{
   switch(c)
   {
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
      if (cbuff[i] == '$'){
         while((cbuff[i] != '*') & (i<xbuff))
         {
            salida[j++]=cbuff[i++];
         }
         comando = salida[1];
         valor2 = salida[2] -'0' + 1;
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
      }
   }
   inicbuff();                            // Borro buffer.
   for(i=0;i<lenbuff;i++)                 // Bucle que pone a 0 todos los
   {
      salida[i]=0x00;                     // caracteres en el argumento
   }
   enable_interrupts(int_rda);
}

void cargar_proceso(int valor2){
   printf("cargado");
   output_bit(PIN_A0,0);
   output_bit(PIN_A1,1);
}

void descargar_proceso(int valor2){
   printf("descargado");
   output_bit(PIN_A0,1);
   output_bit(PIN_A1,0);
}

void main(void)
{
   inte_Init();         // se activa la interrupcion por comunicacion serial
   
   set_tris_a(0x00);
   output_a(0x00);
   
   while(true){
      if(flagcmd == 1){                // se verifica si se ha recibido un nuevo caracter
         procesa_cmd();
      }
      switch(comando)
      {
         case 'C': if (valor2<8){ 
                     cargar_proceso(valor2);
                     break;
                   }
         case 'D': if (valor2<10) {
                     descargar_proceso(valor2);
                     break;
                   }
      }
      comando='-';
      valor2=100;
      valor=0;
   }
}

