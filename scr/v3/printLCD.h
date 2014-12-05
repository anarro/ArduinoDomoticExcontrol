

/*********************************************************************
  Printf personalizado para la escritura en pantalla LCD.

  Los datos de entrada se deben colocar despues de " deben empezar con coma.
  
  writeLCD( "TEMPERATURA %fC %d",variable float, variable int);
  writeLCD( "%D %2/%2/%2 %H:%M:%S ",dayOfMonth, month, year);
  
  COMANDO                   ENTRADA    SALIDA
  %f dato tipo float        float      4 posiciones justificado derecha.
                                         Valores en decimal de -9.9 a 99.9
                                         Valor entero -999 a 9999
                                         Valores inferiores o superiores ----
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

const byte termometer[8] = //icon0 termometro
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};

const byte humidity[8] = //icono humedad
{
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110,
};

const byte centigrade [8] = 
{    
    0b11000,
    0b11000,
    0b00000,
    0b00011,
    0b00100,
    0b00100,
    0b00100,
    0b00011
};

const byte bulbOn [8] = 
{
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b01010,
    0b01110,
    0b01110,
    0b00100
};

const byte bulbOff [8] = 
{  
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b01010,
    0b01110,
    0b01110,
    0b00100
};

// lcd.print((char)223); //signo grados 


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

  
//{{{ loadCharacters():
/*Create a news customs characters.
 \param  None.
 \
 \out    Salida en pantalla.
*/
void loadCharsLCD()
{
  lcd.createChar(0, centigrade);
  lcd.createChar(1, bulbOn);
  lcd.createChar(2, bulbOff);  
  lcd.createChar(3, termometer);
  lcd.createChar(4, humidity);
}

#define lcdPrintCentigrade   lcd.print((uint8_t)0)
#define lcdPrintBulbOn       lcd.print((uint8_t)1)
#define lcdPrintBulbOff      lcd.print((uint8_t)2)
#define lcdPrintTermometer   lcd.print((uint8_t)3)
#define lcdPrintHumidity     lcd.print((uint8_t)4)


//{{{ owrite(): Imprime  cadena.
/*
 \param  puntero a cadena a imprimir.
 \out    Salida en pantalla.
*/
int owrite(const char *str)
{
    lcd.print(str);
    
  #ifdef LCDINSERIAL
    Serial.print(str);
  #endif    
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
    signed int i;
    boolean neg = false;
    result[len--] = 0;            	//null termination.
    

    if(val > 100){
      i= val;
      pdec(i, result, len);
    }
    else if(val < -10){      
      i = val;                           
      i=-i;
      neg=true;
      pdec(i, result, len);
    }
    else{      
      v= val * 10;			// Ejemplo entrada 32.5 v=325
      i = v;                            // Pasamos float a un entero 325
      
      if(i < 0){                        // Si es negativo 
        neg=true;
        i = - i;
      }
      
      result[len--] = ((i%10)+48); 	//Decimal a array.
      result[len--] = '.';		//Coma decimal.
      i /= 10;                          //Borramos unidades.
      pdec(i, result, len);
    }
    
    if(neg)
      result[0]='-';
    
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
                                if(f > 10000 || f < -999.9){
                                  owrite("----");
                                }
                                else{
                                  sftoaR((float)f, i, 4);
                                  owrite(i);
                                }
				break;
			case '%':                             	
				owrite("%");
				break;
			case 'b':
                                if (va_arg(ap, int))
                                  lcdPrintBulbOn;
                                 else
                                  lcdPrintBulbOff;	
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


