
/**
 \ Arduino_Sistema_Domotico.ino
 \ http://domotica-arduino.es/foro/
 ****************************************************************************
 ///  BASADO EN LA VERSIÓN 2.
 *  Este archivo es una modificación para poder se utilizado con Arduinos
 *  con microcontrolador Atmega328. (Arduino Uno , Arduino Ethernet, etc...
 *
 *  Marteriales necesarios.
 *  Modulo I2C RTC http://domotica-arduino.es/comprar-online/es/11-comprar-reloj-arduino
 *  Informacion conexiones
 *  http://zygzax.com/2013/04/22/reloj-con-a...y-rtc-i2c/
 *  http://www.tuelectronica.es/tutoriales/a...duino.html
 *  EN EL CASO DE ATMEGA328 Arduino Uno el modulo RTC tiene que incluir una memoria 24C32.
 *
 *  Ethernet Shield  http://domotica-arduino.es/comprar-online/es/home/10-arduino-ethernet-shield.html 
 * 
 *  No esta probada en Arduino Leonardo.
 ****************************************************************************/
 
 /*
   Carga definiciones del proyecto por defecto, no borrar.
 */
 
 #include   "EXC_def.h"
 #include <SPI.h>
 #include <EEPROM.h>
 
 
 
/**************************************************************************
  #Herramientas de depuracion.
***************************************************************************/

//#define ENABLE_WATCH_DOG;
 #define DEBUG_MODE


/**************************************************************************
  #Librerias shield ETHERNET. UNO Y MEGA.
***************************************************************************/

#define ETHERNET_SHIELD
#include <Ethernet.h>
#include <EthernetUdp.h>  

/**************************************************************************
  #Librerias shield WIFI. UNO Y MEGA.
***************************************************************************/


//#define WIFI_SHIELD;
//#include <WiFi.h>
//#include <WiFiUdp.h>


//  Descomentar para incluir uso de SD, con historicos.
//  #define Historical_SD 
//  #include <SD.h>


/**************************************************************************
  #     OBLIGATORIO UNO, NANO ATMEGA 328 
  #     EN CASO MEGA USA MEMORIA EXTERNA.  
  #Librerias estandar modulo Reloj.......  
  #define moduleDS3231 o #define moduleDS1307
***************************************************************************/
  #define moduleDS1307 
  #include <Wire.h>


//CONFIGURACION EQUIPOS INTALADOS, TERMOSTATOS, ENCHUFES RADIOFRECUENCIA 433MHZ, INFARROJOS.
//EQUIPMENT CONFIGURATION , THERMOSTAT, RADIO 433MHZ, INFARROJOS.



//#define LED_IR 
//#define IR_RECIVE 
//#define THERMOSTAT_DS18B20_NUMBER 1
//#define RECEIVER_433
//#define TRANSMITER_433
//#define ELECTRIC_OUTLET_433
//#define RETROAVISOS             //Habilita funcionamiento por retroavisos


/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/

#ifdef ENABLE_WATCH_DOG
  #include <avr/wdt.h>
#endif 

#if defined (RECEIVER_433)  || defined (TRANSMITER_433) || defined ELECTRIC_OUTLET_433
  #include <RCSwitch.h>
  RCSwitch mySwitch = RCSwitch();
  #include "Mhz433.h"   
#endif 


#ifdef LED_IR  
  #include <IRremote.h>
  IRsend irsend;
#endif 

#ifdef IR_RECIVE
 #ifndef LED_IR
    #include <IRremote.h>
  #endif
  IRrecv irrecv(19);            //Pin 19 Entrada receptor Infrarojo Arduino con Atmega 328 utilizar.....
  decode_results results;
#endif 



#ifdef THERMOSTAT_DS18B20_NUMBER 
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #define TEMPERATURE_PRECISION 9  // Precision de lectura 
  #define ONE_WIRE_BUS 3           //Pin  donde estan conectadas las sondas
  DeviceAddress Ds18B20Addres[THERMOSTAT_DS18B20_NUMBER] ={
                  {0x28,0xAB,0x2A,0x46,0x04,0x00,0x00,0x88}    // Dirección de su sonda DS18B20 nº 1. 
               //,{0x28,0xAB,0x2A,0x46,0x04,0x00,0x00,0x89}    // Dirección de su sonda DS18B20 nº 2. Descomentar para usar.
                                                               // Copiar linea anterior si instala mas sondas.
                  };
  OneWire oneWire(ONE_WIRE_BUS);               // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  DallasTemperature sensorTemp(&oneWire);      // Pass our oneWire reference to Dallas Temperature.   
  boolean ThermostatCool[THERMOSTAT_DS18B20_NUMBER];    // Array de 
  boolean ThermostatHeat[THERMOSTAT_DS18B20_NUMBER];
  float Temperature[THERMOSTAT_DS18B20_NUMBER]={20};    // Array de memoria donde se almacena los valores leidos.
#endif 


#include <LiquidCrystal_I2C.h>
#define BACKLIGHT_PIN     13
LiquidCrystal_I2C lcd(0x38);  // Set the LCD I2C address


//Activa o desactiva cambio hora automatico invierno verano
//Set or Restet Daylight saving time o DST
//El modo dts esta configurado para europa
//Dts mode is set to europe
//Para otros paises configure funcion CargaHora()
//For other countries configure  function  CargaHora()
const boolean Enable_DaylightSavingTime  = true; 

/**************************************************************************
    CONFIGURACION DE PINES Y CIRCUITOS.
    
***************************************************************************/
// POLARIZAION PINES ENTRADA
  #define INTERNAL_RESISTOR; 

// MODO ACTIVACION RELES.
  const boolean On= true;
  const boolean Off= false;

/***************************************************************************
 #En Arduino UNO pueden ser utilizados los siguientes pines.
  0, 1  Si no utilizas el puerto serial.
  2, 3, 4, 5, 6, 7, 8, 9.
  Tambien pueden ser utilizados los pines.
  ANO a AN3 como entradas o salidas, se puede usar el literal
   14 a 17  indistintamente.
  No usar AN4 y AN5 pertenecen al modulo RTC.

****************************************************************************/  

//Entradas con conmutador
//Swicth Inputs
const byte PinSwicthInput[]={5};

//Define pines de Entradas
//Inputs pin
const byte PinInput[]={2,3,4};

//Define pines de Salidas
//Outputs pin
const byte PinOutput[]={6,7,8,14,15};

//Numero de Persianas
//Number of blind
const byte NumeroPersianas = 1;

// Circuitos 
const byte Circuit_Type[] ={Ado_3Etapas, Ado_Digital , Persiana};



/***********************************************************************************************************************/
/***********************************************************************************************************************/
//VARIABLES PARA CONTROL ESTADO CIRUCITOS
//EL RETORNO DE ESTADO SE RECOGE POR ENTRADAS DIGITALES.
/***********************************************************************************************************************/
/***********************************************************************************************************************/  

#ifdef RETROAVISOS 
  byte PinEstadoCircuito[]={27};
  #define RETROAVISO_NUMBER ( sizeof(PinEstadoCircuito))
  const byte Retroavisos_Number=RETROAVISO_NUMBER;
  unsigned long LastTimeEstadoRetroaviso[Retroavisos_Number];  
#endif





//CONFIGURACION DE RED
//Direccion IP ES MUY PROBABLE QUE TENGAS QUE AJUSTARLO A TU RED LOCAL
//Cambia la ip por una dentro de tu rango, tambien debes actualizarla en tu aplicacion android
//Tambien puedes cambiar el puerto, por defecto 5000

// NETWORK CONFIGURATION
// IP Address, ADJUST TO YOUR LOCAL NETWORK
//If you change the IP address will have to adjust in android application
//If you change the Local Port address will have to adjust in android application
//WIRELESS CONFIGURATION
#ifdef WIFI_SHIELD 
  enum Net_Security {OPEN,WEP,WPA};
  Net_Security Net_Type = WEP;
  char ssid[] = "YOUR-SSID"; //  your network SSID (name) 
  char pass[] = "YOUR-PASSWORD";    // your network password (use for WPA, or use as key for WEP)
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x26, 0x20};
  IPAddress ip(192,168,2,49);
  int keyIndex = 0;            // your network key Index number (needed only for WEP)  
  unsigned int TimConexion;
  int status = WL_IDLE_STATUS;     // the Wifi radio's status
  WiFiClient client;
  WiFiUDP Udp;
#endif

#ifdef ETHERNET_SHIELD
  // buffers para recepcion y envio de datos
//  char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
  EthernetClient client;
  // Instanacia Ethernet UDP Para enviar y recibir paqueteP
  EthernetUDP Udp;
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x26, 0x20};
  IPAddress ip(192,168,2,49);
  unsigned int localPort = 5000;
  const String Mail ="";
  const char* Key ="";  
  const boolean SecureConnection=false;  
#endif


#include "Common_functions.h"

/******************************************************************************************************************************/
void UserSetup() { 

}

void UserLoop(){
  float test = -323.5;
  static byte oldSecond;
  char buf[10];
 
   if(!(millis() % 6000)){
//     writeLCD(1, "HABITA %f%% S=%3\n",134.5,35);
//     writeLCD(1,"%D  %H:%M:%S  \n");
   }
   else if (!(millis() %3000)){
//     writeLCD(1,"%s \n","HOLA");
//     writeLCD(1, "%D %2/%2/%2 %H:%M\n",dayOfMonth, month, year);
   
   }
   else if (!(millis() %1000)){
   //  writeLCD(1, "%s \n","LINEA");
//     writeLCD(1, "%D %2/%2/%2 %H:%M\n",dayOfMonth, month, year);
     Serial.print("float=");
     Serial.println(test);
    // Serial.println(sftoaR(test, buf ,6));
     Serial.println(dayOfWeek);
   }

}

//Este evento se produce cada segundo.
//This event occurs every second.
void LoopNewSecond(){
  

 //  writeLCD( "%c\n",'c');
 //  Serial.println(ftoaR(123.35,buffer,4));
}

//Este evento se produce cada 30sg.
//This event occurs every 30SG.
void Loop30Sg(){


}

void NewMinute(){
  //Este evento se produce cada cada minuto.
  //This event occurs every minute.
  #ifdef DEBUG_MODE       
    Serial.println("New Minute");   
  #endif
 if (Condicionados[0]){SetAlarm(0);ResetAlarm(0);}
 
}
/******************************************************************************************************************************/
 /***********************************************************************************************************************/
/***********************************************************************************************************************/
//EVENTOS CONTROL ENTRADAS SALIDAS
//INPUT OUTPUT CONTROL EVENTS

//CUATRO EVENTOS PARA ENTRADAS DIGITALES
//CONMUTADOR CAMBIA VALOR
//PULSACION CORTA
//PULSACION LARGA, MAYOR DE 0.5 SEGUNDOS
//FINAL PULSACION LARGA

// FOUR EVENTS FOR DIGITAL INPUTS
//VALUE CHANGE SWITCH
// PRESS SHORT
// PRESS LONG, OVER 0.5 SECONDS
// LONG PRESS END.

/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
void SwicthStateChange(byte NumberInput){
//AUTO GENERATED CODE
/*************************************************************/
  //CIRCUITO NUMERO 0
  //Conmutador
  if (NumberInput==0){
    if (circuits[1].Value==1){
      circuits[1].Value=0;
    }
    else{
      circuits[1].Value=1;
    }
  }

/*************************************************************/
//END GENERATED CODE
/*************************************************************/

    /*****************************************************************/
  //Este evento se produce cuando un conmutador cambia posicion
  // This event occurs with swicth change state.
  //dos parametros
  //two parameters
  //Number input--numero de entrada
  //Value, if input is HIGH value = HIGH, If value=low then Value=Low
  //Value, si la entrada esta en nivel alto el valor es HIGH si es bajo el valor es LOW
  //Tambien puede acceder desde cualquier punto del codigo al valor de la entrada con SwicthState[]
  //You can know input state anywhere using SwictState[]
  
  /*****************************************************************/


}
void ShortInput(byte NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
//AUTO GENERATED CODE
/*************************************************************/

	//CIRCUITO NUMERO 0
	//Circuito 
	if (NumberInput==0){
        
		switch (circuits[0].Value){
		case 0:
		     circuits[0].Value=1;break;
		case 1: 
		     circuits[0].Value=2; break;
		case 2: 
		     circuits[0].Value=3; break;
		case 3: 
		     circuits[0].Value=1; break;
                }
              }

	//CIRCUITO NUMERO 1
	//Persiana
    if (NumberInput==1){
      circuits[2].Value=50;
    }
     if (NumberInput==2){
      circuits[2].Value=0;
    }
/*************************************************************/
//END GENERATED CODE
/*************************************************************/
  #ifdef DEBUG_MODE   
   Serial.print("Short Input End ");
   Serial.print(NumberInput);
   Serial.print(" pin ");
   Serial.println(PinInput[NumberInput]);    
  #endif
}
void LongInputEnd(byte NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
//AUTO GENERATED CODE
/*************************************************************/

/*************************************************************/
//END GENERATED CODE
/*************************************************************/
  #ifdef DEBUG_MODE   
    Serial.print("Long Input End ");
    Serial.print(NumberInput);
    Serial.print(" pin ");
    Serial.println(PinInput[NumberInput]); 
  #endif
  
  /*****************************************************************/
  //FINAL DE PULSACION LARGA
  //LONG PRESS END, EVENT
  // This event occurs with end a long press.
  /*****************************************************************/
  
}
void LongInput(byte NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
/*************************************************************/
  if (NumberInput==0)
    circuits[0].Value=0; 
    
  #ifdef DEBUG_MODE   
    Serial.print("Long Input Start ");
    Serial.print(NumberInput);
    Serial.print(" pin ");
    Serial.println(PinInput[NumberInput]);     
  #endif
 
  /*****************************************************************/
  //EVENTO PRODUCINO AL INICIO DE UNA PULSACION LARGA
  // LONG PRESS START
  // This event occurs with start a long press.
  /*****************************************************************/    
}

void OutControl(){
/*************************************************************/
//Activamos los reles de control. Activate control relays.
//AUTO GENERATED CODE
/*************************************************************/

	//CIRCUITO NUMERO 0
	//Circuito 
	//Out 1
	digitalWrite(PinOutput[3],circuits[0].Out1_Value);
	//Out 2
	digitalWrite(PinOutput[4],circuits[0].Out2_Value);     

	//CIRCUITO NUMERO 1
	//Conmutador
	//Out 1
	digitalWrite(PinOutput[2],circuits[1].Out1_Value);

        //CIRCUITO NUMERO 2
	//Persiana
	//Out 1
	digitalWrite(PinOutput[0],circuits[2].Out1_Value);
        digitalWrite(PinOutput[1],circuits[2].Out2_Value);
      
     
/*************************************************************/
//END GENERATED CODE
/*************************************************************/
}


char* RunCommand(byte CommandNumber){
  //Este evento se produce cuando se ejecuta un comando desde el app
  //This event occurs when a command is executed from the app
   #ifdef DEBUG_MODE   
    Serial.print("Command Nª");Serial.println(CommandNumber);     
  #endif
  
  //Enabled this line for send infrared 
   #ifdef LED_IR
     SendIr(CommandNumber);
   #endif
  
 return "COMPLETED";
}
void CommonOrders(byte CommandNumber){
  //Este evento se produce cuando se ejecuta un comando desde el app
  //This event occurs when a command is executed from the app
  
  #ifdef DEBUG_MODE   
    Serial.println(CommandNumber);Serial.println(CommandNumber);             
  #endif
}


char* ReadSensor(byte NumeroSensor)
{
  //Monitor sensores
  //monitor sensors
  //El parametro numero de sensor representa se corresponde con el sensor configurado en la app
  //The number represents sensor parameter corresponds to the sensor set in the app
  if (NumeroSensor==100){
    char val[10];    
   //dtostrf(Temperatura[0],4,2,val);
   //Serial.print(val);
    return val;
  }

  return "RESERVA"; //No borrar, dont delete this line
}

String GetAlarmsName(byte Number)
{
/*************************************************************/
//AUTO GENERATED CODE
/*************************************************************/

	if (Number==0){return "CARAMBA CARAMBITA";}
	if (Number==1){return "Reserva";}
	if (Number==2){return "Reserva";}
	if (Number==3){return "Reserva";}
	if (Number==4){return "Reserva";}
	if (Number==5){return "Reserva";}
	if (Number==6){return "Reserva";}
	if (Number==7){return "Reserva";}
	if (Number==8){return "Reserva";}
	if (Number==9){return "Reserva";}
/*************************************************************/
//END AUTO GENERATED CODE
/*************************************************************/

    return "RESERVA";
}
