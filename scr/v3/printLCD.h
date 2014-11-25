

/*********************************************************************
  Printf personalizado para la escritura en pantalla LCD.

  Los datos de entrada se deben colocar despues de " deben empezar con ,
  
  writeLCD( "TEMPERATURA %fC %d",variable float, variable int);
  writeLCD( "%D %2/%2/%2 %H:%M:%S ",dayOfMonth, month, year);
  
  COMANDO                   ENTRADA    SALIDA
  %f dato tipo float        float      4 posiciones un decimal justificado derecha.
  %d dato decimal           int/byte   4 posiciones justificado derecha.
  %4 dato decimal           int/byte   4 posiciones justificado derecha.
  %3 dato decimal           int/byte   3 posiciones justificado derecha.
  %2 dato decimal           int/byte   2 posiciones justificado derecha.  
  %c caracter              ,'o' " "    Caracter
  %s cadena texto flash.    Puntero F  Cadena de texto en flash.
  
  &&& Datos sistema.  Sin datos entrada.
  %% Imprime simbolo %                 %
  %D Imprime Dia Semana     Sistema    3 texto LUN,MAR,MIE,
  %H Imprime hora           Sistema.   2 posiciones.
  %M Imprime minuto         Sistema    Cero a la izquierda 2 posiciones
  %S Imprime segundos       Sistema    Cero a la izquierda 2 posiciones
  
  Problemas:
  El simbolo º produce un ascii extraño.
********************************************************************/  

#include <avr/pgmspace.h>

prog_char string_0[] PROGMEM = "LUN";   // "String 0" etc are strings to store - change to suit.
prog_char string_1[] PROGMEM = "MAR";
prog_char string_2[] PROGMEM = "MIE";
prog_char string_3[] PROGMEM = "JUE";
prog_char string_4[] PROGMEM = "VIE";
prog_char string_5[] PROGMEM = "SAB";
prog_char string_6[] PROGMEM = "DOM";

// Then set up a table to refer to your strings.

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{   
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6 };


//{{{ owrite(): Imprime  cadena.
/*
 \param  puntero a cadena a imprimir.
 \out    Salida en pantalla.
*/
int owrite(const char *str)
{
    Serial.print(str);    
}


//{{{ pdec(): print decena,
/*Subfuncion transforma a ascii los valorea a partir de decenas.
 \param val entero valor.
 \param result cadena donde se va a escribir.
 \param len longitud de la cadena
 \return verdadero si ha sucedido.
*/

char *pdec(int val, char *result, signed char len)
{
    int d;
    while (len >= 0) 
    {       
      if (val > 0)
        result[len] =val%10+48;
      else 
        result[len] =' '; 
        
      val /= 10;
      len--;    
   }
   
   return result;  
}


// {{{ itoaR(): integer to string right justified.
/* Transforam integer en cadena justificado derecha 
 \param val Integer value
 \param result String where the result will be written
 \param len Lenght of the string
 \return true if the operation was succeeded
*/

char *itoaR(int val, char *result, signed char len)
{
    int d;

    result[len--] = 0;		   //NULL TERMINATION.
    result[len--] = (val%10)+48;   //Guarda Unidades.
    val /= 10;                     //Eliminamos unidades.
    pdec(val, result, len);
    
    return result;  
}
//{{ sftoaR(): float to string right justified whit signed.
/* Transforma float en cadena justificado derecha añade signo negativo. 
 \param val entero valor.
 \param result cadena donde se va a escribir.
 \param len longitud de la cadena
 \return verdadero si ha sucedido.
*/

char *sftoaR(float val, char *result, signed char len)
{
    float v;
    int i;
    boolean neg = false;
    
    v= val * 10;			// Ejemplo entrada 32.5 v=325
    i = v;                              // Pasamos float a un entero 325
    
    if(v < 0){                          // Si es negativo 
      neg=true;
      i = - i;
    }   				
	
    result[len--] = 0;            	//null termination.
    result[len--] = ((i%10)+48); 	//Decimal a array.
    result[len--] = '.';		//Coma decimal.
    i /= 10;                            //Borramos unidades.
    pdec(i, result, len);
    if(neg)
      result[0]='-';
    
    return result;
}

//{{ ftoaR(): float to string right justified.
/* Transforma float en cadena justificado derecha   
 \param val entero valor.
 \param result cadena donde se va a escribir.
 \param len longitud de la cadena
 \return verdadero si ha sucedido.
*/

char *ftoaR(float val, char *result, signed char len)
{
    float v;
    int i;
    
    v= val * 10;			// Ejemplo entrada 32.5 v=325						
    i = v;					// Pasamos float a un entero 325

    result[len--] = 0;            	//null termination.
    result[len--] = ((i%10)+48); 	//Decimal a array.
    result[len--] = '.';		//Coma decimal.
    i /= 10;                            //Borramos unidades.
    pdec(i, result, len);
    
    return result;
}


// {{{ writeLCD(): personalized printf for LCD
void writeLCD(const unsigned char line,const char *fmt, ...) 
{
	va_list ap, ap2;
	short d;
	char c, *s;
	double f;
	char i[10];
        if (line < 5){
          lcd.setCursor ( 0, line );
        }
       

	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '%') {
			*fmt++;
			switch (*fmt){
			case 's':
				s = va_arg(ap, char*);
				owrite (s);
				break;
			case 'H':                      
				itoaR(hour, i, 2);
				owrite (i);
				break;
			case 'M':
				itoaR(minute +100, i, 2);
				owrite (i);
				break;
			case 'S':
				itoaR(second +100, i, 2);
				owrite (i);
				break;
			case 'D':
				strcpy_P(i, (char*)pgm_read_word(&(string_table[dayOfWeek])-1));			   
				owrite (i);
				break;
			case 'd':
			case '4':
				d = va_arg(ap, int);
				itoaR(d, i, 4);
				owrite(i);
				break;
			case '3':
				d = va_arg(ap, int);
				itoaR(d, i, 3);
				owrite(i);
				break;
			case '2':
			        d = va_arg(ap, int);
				itoaR(d, i, 2);
				owrite(i);
				break;   
			case 'c':
				c = va_arg(ap, int);
				i[0]=char(c);
				i[1]=0;
				owrite(i);
			case 'f':
				f = va_arg(ap, double);
				ftoaR((float)f, i, 5);
				owrite(i);
				break;
			case '%':                             	
				owrite("%");
				break;
			}
			*fmt++;

		} else {
			i[0]=char(*fmt);
			i[1]=0;
			owrite(i);
			*fmt++;
		}
	}
	va_end(ap);
}


