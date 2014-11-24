// From: http://www.a-control.de/arduino-fehler/?lang=en
#if 1
__asm volatile ("nop");
#endif
#include   "Excontrol_def.h"
//Enable watch dog
//habilita perro guardian
//#define ENABLE_WATCH_DOG
#define INTERNAL_RESISTOR
//#define WIFI_SHIELD
//#define ARDUINO_MEGA
#ifdef ARDUINO_MEGA
  //#define SD_CARD
#endif


#ifdef ENABLE_WATCH_DOG
  #include <avr/wdt.h>
#endif 
#include <SPI.h>  
#ifdef WIFI_SHIELD
//#include <WiFi.h>
//#include <WiFiUdp.h>
#else
#include <Ethernet.h>
#include <EthernetUdp.h>
 // #include <utility/w5100.h>     // Advance Ethernet functions
#endif
#ifdef ARDUINO_MEGA
//#include <SD.h>
//#include <EEPROM.h>
   
#else

  //Clock addres
  //DS3231= 57
  //ds1307=50
#define IC24C32_I2C_ADDRESS 0x50
#endif 

#include <Wire.h>
#define UDP_TX_PACKET_MAX_SIZE 100 //increase UDP size
#define DS_RTC 0x68  //Direccion Reloj
//****************************************************
//CONFIGURACION EQUIPOS INTALADOS, TERMOSTATOS, ENCHUFES RADIOFRECUENCIA 433MHZ, INFARROJOS.
//EQUIPMENT CONFIGURATION , THERMOSTAT, RADIO 433MHZ, INFARROJOS.


//#define DEBUG_MODE
//#define LED_IR 
//#define IR_RECIVE 
//#define THERMOSTAT_DS18B20_NUMBER
//#define RECEIVER_433
//#define TRANSMITER_433
//#define RETROAVISOS //Habilita funcionamiento por retroavisos


/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/

//
#ifdef RECEIVER_433
  #include <RCSwitch.h>
#endif

#ifdef TRANSMITER_433 
 #ifndef RECEIVER_433
    #include <RCSwitch.h>
  #endif
#endif 
#ifdef LED_IR  
  #include <IRremote.h>
  IRsend irsend;
#endif 

#ifdef IR_RECIVE
 #ifndef LED_IR
    #include <IRremote.h>
  #endif

  IRrecv irrecv(19);//El 19 corresponde con el pin de arduino, cambiar para utilizar otro
  decode_results results;
#endif 

#ifdef THERMOSTAT_DS18B20_NUMBER 
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #define TEMPERATURE_PRECISION 9
  #define ONE_WIRE_BUS 3           //Pin one wire donde estan conectadas las sondas
  DeviceAddress Ds18B20Addres[THERMOSTAT_DS18B20_NUMBER] ={{0x28,0xAB,0x2A,0x46,0x04,0x00,0x00,0x88}};
  OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  DallasTemperature sensorTemp(&oneWire);// Pass our oneWire reference to Dallas Temperature.   
  boolean ThermostatCool[THERMOSTAT_DS18B20_NUMBER];
  boolean ThermostatHeat[THERMOSTAT_DS18B20_NUMBER];
  float Temperature[]={20};
  const float Histeresis =0.5;
#endif 

#ifdef RECEIVER_433
  RCSwitch mySwitch = RCSwitch();
#endif
#if defined (TRANSMITER_433)  && !defined (RECEIVER_433)
  RCSwitch mySwitch = RCSwitch();
  
#endif 




/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//ZONA DE CONFIGURACIONES 
//SETTINGS ZONE
//Define numero de entradas salidas 
//Configuracion Red
//Activa o desactiva el perro guardian
/******************************************************************************************************************************/

//SETTINGS ZONE
//Defines number of inputs and outputs
//Network configuration
//Set or Restet Daylight saving time o DST
//Set or Reset watchdog
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//Activa o desactiva cambio hora automatico invierno verano
//Set or Restet Daylight saving time o DST
//El modo dts esta configurado para europa
//Dts mode is set to europe
//Para otros paises configure funcion CargaHora()
//For other countries configure  function  CargaHora()
const boolean Enable_DaylightSavingTime  = true; 

//Entradas con conmutador
//Swicth Inputs
byte PinSwicthInput[] = {5};

//Define pines de Entradas
//Inputs pin
byte PinInput[]={2, 3, 4};

//Define pines de Salidas
//Outputs pin
byte PinOutput[]={6,7,8,14,15};

//Numero de Persianas
//Number of blind
const byte NumeroPersianas =1;



const boolean  On = HIGH;
const boolean  Off = LOW;
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//VARIABLES PARA CONTROL ESTADO CIRUCITOS
//EL RETORNO DE ESTADO SE RECOGE POR ENTRADAS DIGITALES.
/***********************************************************************************************************************/
/***********************************************************************************************************************/  

#ifdef RETROAVISOS 
byte PinEstadoCircuito[]={};//Indicar el pin de las entradas que son retroavisos.
  const byte Retroavisos_Number=sizeof(PinEstadoCircuito);
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
  Net_Security Net_Type = WPA;
  char ssid[] = ""; //  your network SSID (name) 
  char pass[] = ""; //  // your network password (use for WPA, or use as key for WEP) 
  int keyIndex = 0;// your network key Index number (needed only for WEP)
  
  unsigned int TimConexion;
  int status = WL_IDLE_STATUS;     // the Wifi radio's status
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
  WiFiClient client;
  WiFiUDP Udp;
#else
  // buffers para recepcion y envio de datos
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
  EthernetClient client;
  // Instanacia Ethernet UDP Para enviar y recibir paqueteP
  EthernetUDP Udp;
   
#endif


byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x26, 0x20};
IPAddress ip(192,168,1,200);
unsigned int localPort = 5000;
const String Mail ="";
const char* Key ="";
const boolean SecureConnection=false;


//CONFIGURACION DE HISOTORICOS
#ifdef SD_CARD
  boolean SdOk=true;
  File SdFile;
  const byte  MinutesHistorico=15;
  byte MinutesLstHist=0;
#endif
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//FIN DE ZONAS DE CONFIGURACIONES
//END SETTINGS ZONE
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
// Circuitos 



struct Circuit {byte Type;boolean Out1_Value;boolean Out2_Value;byte Device_Number;byte Value;byte OldValue;byte CopyRef;};
byte TipeCir[] ={Persiana, Ado_3Etapas, Ado_Digital};

#ifdef ARDUINO_MEGA
  #define SS_ETHERNET 53 //53 for mega, for other set pin to 10
  #define SS_SD 4
  #define SS_UNO 10
#endif


const byte Number_SwicthInput=sizeof(PinSwicthInput);
const byte Number_Input=sizeof(PinInput);
const byte Number_Output=sizeof(PinOutput);
const byte Number_Circuit=sizeof(TipeCir);
Circuit circuits[Number_Circuit];


byte EspRfrIp = 0;

//Varibles Reloj
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year,minutoMemory,TipoDia;
boolean HoraRetrasa;
unsigned long Tim30Sg,TimSg,TimNow;

//Variables Gestion Entradas Salidas
unsigned long LastTimeSwicthInput[Number_SwicthInput];  //Ultima vez que la entrada cambio el estado
int SwicthState[Number_SwicthInput];         // current state of the button
unsigned long LastTimeInput[Number_Input];  //Ultima vez que la entrada estaba en reposo
byte InState[Number_Input];  //Estado Entrada


//Variables Control Alarmas
byte Alarms[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Configuracion Persianas
/////////////////////////////////////////////////////////////////////////////////////////////////////////
byte PosicionPersiana[NumeroPersianas];  //Controla la posicion de la persiana % Subida
byte LocalizadorPersiana[NumeroPersianas];  //Controla posicion persiana en array de circuitos


unsigned long TiempoMovPersiana[NumeroPersianas];  //Tiempo de mov desde el ultimo refresco

//Memoria de tiempos y posicion respecto a tiempo
unsigned long TiempoPosPersianaUp[NumeroPersianas];  //Posicion persiana en subida
unsigned long TiempoPosPersianaDown[NumeroPersianas];  //Posicion persiana en Bajada
unsigned long TimUpPersiana[NumeroPersianas];  //Tiempo en MicrosSg subida persiana
unsigned long TimDowPersiana[NumeroPersianas];  //Tiempo en MicrosSg bajada persiana

//Variables para salidas y entradas
boolean OutUpPersiana[NumeroPersianas];  //Boleana para activar salida persiana
boolean OutDowPersiana[NumeroPersianas];  //Boleana para activar salida persiana
boolean InUpPersiana[NumeroPersianas];  //Boleana pulsador subida Persiana
boolean InDowPersiana[NumeroPersianas];  //Boleana pulsador bajada Persiana

boolean Condicionados[10];              //Guarda el estado de los condicionados
byte Consignas[10];                     //Guarda el valor de las consignas
boolean Connecting=false;



/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//ZONA DE CONFIGURACIONES 
//SETTINGS ZONE

/******************************************************************************************************************************/
void UserSetup() {

}

void UserLoop(){
/*************************************************************/
//AUTO GENERATED CODE
/*************************************************************/

/*************************************************************/
//END AUTO GENERATED CODE
/*************************************************************/


}
void LoopNewSecond(){
  //Este evento se produce cada segundo.
  //This event occurs every second.

}
void Loop30Sg(){
  //Este evento se produce cada 30sg.
  //This event occurs every 30SG.

}
void NewMinute(){
  //Este evento se produce cada cada minuto.
  //This event occurs every minute.
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
void SwicthStateChange(int NumberInput, int value){
//AUTO GENERATED CODE
/*************************************************************/
/*************************************************************/
//END GENERATED CODE
/*************************************************************/
//CIRCUITO NUMERO 0
  //Conmutador
  if (NumberInput==0){
    if (circuits[2].Value==1){
      circuits[2].Value=0;
    }
    else{
      circuits[2].Value=1;
    }
  }

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
  

  #ifdef DEBUG_MODE   
   Serial.print("Swict State Change, input ");
   Serial.print(NumberInput);
    if (value==LOW){Serial.println(" Off");}else{Serial.println(" On");}
  #endif
}
void ShortInput(int NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
//AUTO GENERATED CODE
/*************************************************************/

  //CIRCUITO NUMERO 0
  //Circuito 
  if (NumberInput==2){
    switch (circuits[1].Value){
    case 0:
         circuits[1].Value=3; break;
    case 1: 
         circuits[1].Value=3; break;
    case 2: 
         circuits[1].Value=1; break;
    case 3: 
         circuits[1].Value=2; break;}}

  //CIRCUITO NUMERO 1
  //Circuito 
  if (NumberInput==0){
    if ( OutUpPersiana[0] || OutDowPersiana[0] ){
      circuits[0].Value=PosicionPersiana[0];
    }
    else{
      circuits[0].Value=100;
    }
  }
  if (NumberInput==1){if (OutUpPersiana[0]||OutDowPersiana[0]){circuits[0].Value=PosicionPersiana[0];}else {circuits[0].Value=0;}}

/*************************************************************/
//END GENERATED CODE
/*************************************************************/
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.

  #ifdef DEBUG_MODE   
   Serial.print("Short Input End ");
   Serial.print(NumberInput);
   if (NumberInput<=99){	
	Serial.print(" pin ");
	Serial.println(PinInput[NumberInput]);  
	}  
  #endif
}
void LongInputEnd(int NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
//AUTO GENERATED CODE
/*************************************************************/

  //CIRCUITO NUMERO 1
  //Circuito 
  if (NumberInput==0){InUpPersiana[0]=0;InDowPersiana[0]=0;}

  if (NumberInput==1){InUpPersiana[0]=0;InDowPersiana[0]=0;}

/*************************************************************/
//END GENERATED CODE
/*************************************************************/
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.

  #ifdef DEBUG_MODE   
    Serial.print("Long Input End ");
    Serial.print(NumberInput);
    if (NumberInput<=99){Serial.print(" pin ");Serial.println(PinInput[NumberInput]);} 
  #endif
  
  /*****************************************************************/
  //FINAL DE PULSACION LARGA
  //LONG PRESS END, EVENT
  // This event occurs with end a long press.
  /*****************************************************************/
  
}
void LongInput(int NumberInput){
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
//AUTO GENERATED CODE
/*************************************************************/

 
  //CIRCUITO NUMERO 1
  //Circuito 
  if (NumberInput==0){InUpPersiana[0]=1;InDowPersiana[0]=0;}

  if (NumberInput==1){InUpPersiana[0]=0;InDowPersiana[0]=1;}

/*************************************************************/
//END GENERATED CODE
/*************************************************************/
/*************************************************************/
//Este evento se produce con una pulsación corta..
//This event occurs with a short press.
/*************************************************************/
  #ifdef DEBUG_MODE   
    Serial.print("Long Input  ");
    Serial.print(NumberInput);
    if (NumberInput<=99){Serial.print(" pin ");Serial.println(PinInput[NumberInput]);} 
  #endif 
}

void OutControl(){
/*************************************************************/
//Activamos los reles de control. Activate control relays.
//AUTO GENERATED CODE
/*************************************************************/

 
  //CIRCUITO NUMERO 0
	//Circuito 
	//Out 1
	digitalWrite(PinOutput[0],circuits[0].Out1_Value);
	//Out 2
	digitalWrite(PinOutput[1],circuits[0].Out2_Value);     

	//CIRCUITO NUMERO 1
	//Conmutador
	//Out 1
	digitalWrite(PinOutput[2],circuits[1].Out1_Value);

        //CIRCUITO NUMERO 2
	//Persiana
	//Out 1
	digitalWrite(PinOutput[3],circuits[2].Out1_Value);
        digitalWrite(PinOutput[4],circuits[2].Out2_Value);

/*************************************************************/
//END GENERATED CODE
/*************************************************************/


     #ifdef ELECTRIC_OUTLET_433
         if (circuits[c].Type==EnchufeRF){
           if (circuits[c].Value!=circuits[c].OldValue){
             
             if (circuits[c].Value==1){Electric_Outlet_Control(circuits[c].Device_Number+1,true);}else{Electric_Outlet_Control(circuits[c].Device_Number+1,false);}
             circuits[c].OldValue=circuits[c].Value;
           }
         }
     #endif 
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
  //Monitor sensores
  //monitor sensors
  //El parametro numero de sensor representa se corresponde con el sensor configurado en la app
  //The number represents sensor parameter corresponds to the sensor set in the app
char* ReadSensor(byte NumeroSensor){


  return "RESERVA"; //No borrar, dont delete this line
}
String GetAlarmsName(byte Number){
/*************************************************************/
//AUTO GENERATED CODE
/*************************************************************/

  if (Number==0){return "Reserva";}
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

/*************************************************************/
//AUTO GENERATED CODE
/*************************************************************/

  if (Number==0){return "Reserva";}
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
