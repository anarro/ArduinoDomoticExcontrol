#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>
#include <Wire.h>
#include <avr/wdt.h>
#define UDP_TX_PACKET_MAX_SIZE 340 //increase UDP size
#define DS_RTC 0x68  //Direccion Reloj


#define THERMOSTAT_DS18B20_NUMBER 1
#define ENABLED_IR_LED true
#define ENABLED_IR_RECIVE true

#if (THERMOSTAT_DS18B20_NUMBER>0)
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #define ONE_WIRE_BUS 3           //Pin one wire donde estan conectadas las sondas
#endif 


#if (ENABLED_IR_LED)
  #include <IRremote.h>
  IRsend irsend;
#endif 

#if (ENABLED_IR_RECIVE)
  #include <IRremote.h>
  IRrecv irrecv(19);//El 19 corresponde con el pin de arduino, cambiar para utilizar otro
  decode_results results;
#endif 

#if (THERMOSTAT_DS18B20_NUMBER>0)
  DeviceAddress Ds18B20Addres1 = { 0x28, 0x2B, 0x7D, 0x54, 0x05, 0x00, 0x00, 0x32};
  DeviceAddress Ds18B20Addres2 = { 0x28, 0x8C, 0x7A, 0x5D, 0x04, 0x00, 0x00, 0x82};
  DeviceAddress Ds18B20Addres3 = { 0x28, 0x4E, 0x66, 0x58, 0x05, 0x00, 0x00, 0x04};
  
  #define TEMPERATURE_PRECISION 9
  OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  DallasTemperature sensorTemp(&oneWire);// Pass our oneWire reference to Dallas Temperature. 
  unsigned int ThermostatAC[THERMOSTAT_DS18B20_NUMBER];
  unsigned int ThermostatAH[THERMOSTAT_DS18B20_NUMBER];
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

//Numero de Entradas con conmutador
//Number of swicth Inputs
 byte PinSwicthInput[] = {39, 41};

//Define pines de Entradas
//Inputs pin
byte PinInput[] = {23, 25,27,29,31,33,35,37};

//Define pines de Salidas
//Outputs pin
byte PinOutput[]={22, 24,26,28,30,32,34,36,38,40,42,43,44,45,46,47};

//Numero de Persianas
//Number of blind
const byte NumeroPersianas = 2; 

//CONFIGURACION DE RED
//Direccion IP ES MUY PROBABLE QUE TENGAS QUE AJUSTARLO A TU RED LOCAL
//Cambia la ip por una dentro de tu rango, tambien debes actualizarla en tu aplicacion android
//Tambien puedes cambiar el puerto, por defecto 5000

// NETWORK CONFIGURATION
// IP Address, ADJUST TO YOUR LOCAL NETWORK
//If you change the IP address will have to adjust in android application
//If you change the Local Port address will have to adjust in android application

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 200);
unsigned int localPort = 5000;      // puerto local para eschucha de paquete

const String Mail ="";
const char* Key ="12345678";          //8 Characters, no more, no less....8 Caracteres, no mas, no menos
const boolean SecureConnection=false;  // ENABLED SECURE CONNECTION = TRUE.... CONEXION SEGURA = TRUE

// Set or reset watchdog
//Activa o desactiva perro guardian
const boolean EnabledWatchdog=true;

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
#define Number_SwicthInput ( sizeof(PinSwicthInput))
#define Number_Input ( sizeof(PinInput))
#define Number_Output ( sizeof(PinOutput))

byte EspRfrIp = 0;
// buffers para recepcion y envio de datos
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
EthernetClient client;
// Instanacia Ethernet UDP Para enviar y recibir paqueteP
EthernetUDP Udp;
//Varibles Reloj
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year,minutoMemory,TipoDia;
boolean HoraRetrasa;
unsigned long LastMsg;

//Variables Gestion Entradas Salidas
unsigned long LastTimeSwicthInput[Number_SwicthInput];  //Ultima vez que la entrada cambio el estado
int SwicthState[Number_SwicthInput];         // current state of the button
unsigned long LastTimeInput[Number_Input];  //Ultima vez que la entrada estaba en reposo
byte InState[Number_Input];  //Estado Entrada

//Registros Salidas Circuitos
byte ElectricalCircuitValue[30];
float Temperatura[]={22.2,22.4,22.6};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Configuracion Persianas
/////////////////////////////////////////////////////////////////////////////////////////////////////////
byte PosicionPersiana[NumeroPersianas];  //Controla la posicion de la persiana % Subida
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


/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
//ZONA DE CONFIGURACIONES 
//SETTINGS ZONE

/******************************************************************************************************************************/
void setup() {

  Serial.begin(9600);
  SystemSetup();//Dont remove this line, no elimines esta linea
  //si estas usando receptor infrarrojos Reinicialo
  //if you are using infrared receiver restart it  
  //irrecv.enableIRIn(); // Re-enable receiver  
}
void loop(){

   SystemLoop();//Dont remove this line, no elimines esta linea
}
void Loop30Sg(){
//Este evento se produce cada 30sg.
//This event occurs every 30SG.
  
  //on if temperature control
  //Activar RefreshTemperature con termostatos
  //RefreshTemperature()

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
void SwicthStateChange(int NumberInput){
  //Serial.println(NumberInput);
    /*****************************************************************/
  //Este evento se produce cuando un conmutador cambia posicion
  // This event occurs with swicth change state.
  /*****************************************************************/
    //Ejemplo de uso
  // Example of use
  

  //Ado Circuito numero 3 (Conmutador )
   if (NumberInput==0){if (ElectricalCircuitValue[2]==1){ElectricalCircuitValue[2]=0;}else{ElectricalCircuitValue[2]=1;}}
   //Ado Circuito numero 4  (Interruptor)
   if (NumberInput==1){if (ElectricalCircuitValue[3]==1){ElectricalCircuitValue[3]=0;}else{ElectricalCircuitValue[3]=1;}}
}
void ShortInput(int NumberInput){
  /*****************************************************************/
  //Este evento se produce con una pulsación corta.
  // This event occurs with a short press.
  /*****************************************************************/
  //Ejemplo de uso
  // Example of use
  

  //Iluminacion 1
  if (NumberInput==0){
         switch (ElectricalCircuitValue[0]) {
          case 0:    
            ElectricalCircuitValue[0]=3; break;
          case 1:    
            ElectricalCircuitValue[0]=3; break;
          case 2:    
             ElectricalCircuitValue[0]=1; break;
          case 3:    
             ElectricalCircuitValue[0]=2; break; 
       }  
  }
  //Iluminacion 2
  if (NumberInput==1){
     switch (ElectricalCircuitValue[1]) {
          case 0:    
            ElectricalCircuitValue[1]=3; break;
          case 1:    
            ElectricalCircuitValue[1]=3; break;
          case 2:    
             ElectricalCircuitValue[1]=1; break;
          case 3:    
             ElectricalCircuitValue[1]=2; break;          
       } 
  }

   //Persianas
  //blind
  //Ejemplo de uso
  // Example of use  
   
   //Persiana 1 
   if (NumberInput==4){ElectricalCircuitValue[23]=100;} //Open blind 100%
   if (NumberInput==5){ElectricalCircuitValue[23]=0;}//Closed blind 
  //Persiana 2
   if (NumberInput==6){ElectricalCircuitValue[24]=100;}//Open blind 100%
   if (NumberInput==7){ElectricalCircuitValue[24]=0;}//Closed blind 

}
void LongInputEnd(int NumberInput){
  /*****************************************************************/
  //FINAL DE PULSACION LARGA
  //LONG PRESS END, EVENT
  // This event occurs with end a long press.
  /*****************************************************************/
  
  //Persianas
  //blind
  //Ejemplo de uso
  // Example of use

  
  if (NumberInput==4){InUpPersiana[0]=false;}//detiene subida de persiana en manual,Manual shutter up stops 
  if (NumberInput==5){InDowPersiana[0]=false;}//detiene bajada persiana manual, Manual shutter down stops0
  if (NumberInput==6){InUpPersiana[1]=false;}//detiene subida de persiana en manual,Manual shutter up stops 
  if (NumberInput==7){InDowPersiana[1]=false;}//detiene subida de persiana en manual,Manual shutter up stops
  
}
void LongInput(int NumberInput){
  /*****************************************************************/
  //EVENTO PRODUCINO AL INICIO DE UNA PULSACION LARGA
  // LONG PRESS START
  // This event occurs with start a long press.
  /*****************************************************************/
  //Ejemplo de uso
  // Example of use
  
  
    //Pulsador Ado. Circuito 1
   if (NumberInput==0){ElectricalCircuitValue[0]=0;}
   
   //Pulsador Ado. Circuito 2
   if (NumberInput==1){ElectricalCircuitValue[1]=0;}  
   

  //Persianas
  //blind
  //Ejemplo de uso
  // Example of use

  if (NumberInput==4){InUpPersiana[0]=true;InDowPersiana[0]=false;}//Sube persiana 0 mientras se pulsa, Upload shutter while pressing 
  if (NumberInput==5){InDowPersiana[0]=true;InUpPersiana[0]=false;}//Baja persiana 0 mientras se pulsa, Donwload shutter while pressing 
  if (NumberInput==6){InUpPersiana[1]=true;InDowPersiana[1]=false;}//Sube persiana  mientras se pulsa, Upload shutter while pressing 
  if (NumberInput==7){InDowPersiana[1]=true;InUpPersiana[1]=false;}//Baja persiana  mientras se pulsa, Donwload shutter while pressing 
  
}
void OutControl()
{

   /*************************************************************/
   //Gestion Salidas
   //Activamos los reles de control.
   
   // Outputs Management
   // Activate control relays.
   /*************************************************************/
  //Ejemplo de uso
  // Example of use
 
  
   // Circuito Numero 1
   //Iluminacion Iluminacion con 3 puntos de luz controlados mediante 2 Reles.
   switch (ElectricalCircuitValue[0]) {
    case 0:    // your hand is on the sensor
      digitalWrite(PinOutput[4], LOW);
      digitalWrite(PinOutput[5], LOW); 
      break;
    case 1:    // your hand is close to the sensor
      digitalWrite(PinOutput[4], HIGH);
      digitalWrite(PinOutput[5], LOW); 
      break;
    case 2:    // your hand is a few inches from the sensor
       digitalWrite(PinOutput[4], LOW);
      digitalWrite(PinOutput[5], HIGH); 
      break;
     case 3:    
       digitalWrite(PinOutput[4], HIGH);
      digitalWrite(PinOutput[5], HIGH); 
      break;
     default:    
      ElectricalCircuitValue[0]=0;
      break;
    } 
    
   // Circuito Numero 2
   //Iluminacion Iluminacion con 3 puntos de luz controlados mediante 2 Reles.
     switch (ElectricalCircuitValue[1]) {
    case 0:    
      digitalWrite(PinOutput[6], LOW);
      digitalWrite(PinOutput[7], LOW); 
      break;
    case 1:    
      digitalWrite(PinOutput[6], HIGH);
      digitalWrite(PinOutput[7], LOW); 
      break;
    case 2:    
       digitalWrite(PinOutput[6], LOW);
      digitalWrite(PinOutput[7], HIGH); 
      break;
     case 3:    
       digitalWrite(PinOutput[6], HIGH);
      digitalWrite(PinOutput[7], HIGH); 
      break;
     default:    
      ElectricalCircuitValue[1]=0;
      break;
  }
  
  // Circuito Ado Numero 3
   switch (ElectricalCircuitValue[2]) {
    case 0:    
      digitalWrite(PinOutput[8], HIGH);
      break;
    case 1:    
      digitalWrite(PinOutput[8], LOW); 
      break;
    default:
      ElectricalCircuitValue[2]=0;
   }
  
  // Circuito Ado Numero 4

   switch (ElectricalCircuitValue[3]) {
    case 0:    
      digitalWrite(PinOutput[9], HIGH);
      break;
    case 1:    
      digitalWrite(PinOutput[9], LOW); 
      break;
    default:
      ElectricalCircuitValue[3]=0;
   }
  
  
  // Circuito Numero 7
   //Enchufe
      //Enchufes
   switch (ElectricalCircuitValue[6]) {
    case 0:    
      digitalWrite(PinOutput[11], HIGH);
      break;
    case 1:    
      digitalWrite(PinOutput[11], LOW); 
      break;
    default:
      ElectricalCircuitValue[6]=0;
   }
   // Circuito Numero 8
   //Enchufe numero 1
   switch (ElectricalCircuitValue[7]) {
    case 0:    
      digitalWrite(PinOutput[12], HIGH);
      break;
    case 1:    
      digitalWrite(PinOutput[12], LOW); 
      break;
    default:
      ElectricalCircuitValue[7]=0;
   }
 

   // Circuito Numero 10
   //Persiana 1 1
   if ((OutDowPersiana[0]==true)||(OutUpPersiana[0]==true))
   {
     if ((OutDowPersiana[0]==true)&&(OutUpPersiana[0]==false)){digitalWrite(PinOutput[0], HIGH); digitalWrite(PinOutput[1], HIGH);  }
     if ((OutDowPersiana[0]==false)&&(OutUpPersiana[0]==true)){digitalWrite(PinOutput[0], HIGH); digitalWrite(PinOutput[1], LOW);  }     
   }
   else{
     
     digitalWrite(PinOutput[0], LOW);
     digitalWrite(PinOutput[1], LOW);
   }
   //Persiana 2
   if ((OutDowPersiana[1]==true)||(OutUpPersiana[1]==true))
   {
     if ((OutDowPersiana[1]==true)&&(OutUpPersiana[1]==false)){digitalWrite(PinOutput[2], HIGH); digitalWrite(PinOutput[3], HIGH);  }
     if ((OutDowPersiana[1]==false)&&(OutUpPersiana[1]==true)){digitalWrite(PinOutput[2], HIGH); digitalWrite(PinOutput[3], LOW);  }     
   }
   else{
     digitalWrite(PinOutput[2], LOW);
     digitalWrite(PinOutput[3], LOW);
   }
}

char* RunCommand(byte CommandNumber){
  //Este evento se produce cuando se ejecuta un comando desde el app
  //This event occurs when a command is executed from the app
 //Serial.print("Command Nª");
 //Serial.println(CommandNumber); 
 
 
 return "COMPLETED";
}
void CommonOrders(byte CommandNumber){
  //Este evento se produce cuando se ejecuta un comando desde el app
  //This event occurs when a command is executed from the app
 Serial.print("Command Nª");
 Serial.println(CommandNumber); 
}
char* ReadSensor(byte NumeroSensor)
{
  //Monitor sensores
  //monitor sensors
  //El parametro numero de sensor representa se corresponde con el sensor configurado en la app
  //The number represents sensor parameter corresponds to the sensor set in the app
  if (NumeroSensor==1){return "22.5";}
  if (NumeroSensor==2){return "60";}
  return "RESERVA"; 
}

/*****************************************************************************************/
/*****************************************************************************************
Funcion de Recepcion de infrarrojos por mando a distancia.
Puedes seleccionar escenas.
Puedes mover persianas
Adaptando el codigo a tu mando a distancia

Receiving function infrared remote control.
You can select scenes.
You can move blinds
Adapting the code to your remote

DONWLOAD LIBRARY...DESCARGA
https://github.com/shirriff/Arduino-IRremote
************************************************************************************************/
/************************************************************************************************/
#if (ENABLED_IR_RECIVE)
void RecepcionInfrarrojos(decode_results *results) {
  

 if (results->decode_type == NEC) {
   //En este caso es nec pero puede ser JVC, PANASONIC...
   //Tienes que adaptarlo a tu mando a distancia
   switch (results->value){
    case  0x40BFA05F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(1);
      break;
      
    case  0x40BF609F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(2);
      break;
    
    case  0x40BFE01F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(3);
      break;
    case  0x40BF906F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(4);
      break;  
      
    case  0x40BF50AF://cambia el codigo por el que envio tu mando a distancia
      SelectScene(5);
      break;  
          
    case  0x40BFD02F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(6);
      break;  
     
    case  0x40BFB04F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(7);
      break;  
      
    case  0x40BF708F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(8);
      break;  
    
    case  0x40BFF00F://cambia el codigo por el que envio tu mando a distancia
      SelectScene(9);
      break;  
      
    case  0x40BF8877://cambia el codigo por el que envio tu mando a distancia
      SelectScene(10);
      break;  
   
   //Subir 20% Persiana 1
    case  0x40BFB847://cambia el codigo por el que envio tu mando a distancia 
      if (ElectricalCircuitValue[23]<=80){ElectricalCircuitValue[23]+=20;}else{ElectricalCircuitValue[23]=100;}
      break;  
    //bajar 20% Persiana 1
    case  0xC03F807F://cambia el codigo por el que envio tu mando a distancia
      if (ElectricalCircuitValue[23]>=20){ElectricalCircuitValue[23]-=20;}else{ElectricalCircuitValue[23]=0;}
      break;  
    //Subir 20% Persiana 2  
    case  0x40BF38C7://cambia el codigo por el que envio tu mando a distancia
      if (ElectricalCircuitValue[24]<=80){ElectricalCircuitValue[24]+=20;}else{ElectricalCircuitValue[24]=100;}
      break;  
    //bajar 20% Persiana 2
    case  0xC03F00FF://cambia el codigo por el que envio tu mando a distancia
      if (ElectricalCircuitValue[24]>=20){ElectricalCircuitValue[24]-=20;}else{ElectricalCircuitValue[24]=0;}
      break; 
     
   }  
  }  
}
#endif 
/***********************************************************************************
Funcion de Envio de infrarrojos por mando a distancia.
Puedes manejar numerosos dispositvos controlados por infrarrojo

Delivery function of infrared remote control.
You can manage many enabled devices controlled by infrared
DONWLOAD LIBRARY...DESCARGA
https://github.com/shirriff/Arduino-IRremote
************************************************************************************************/
#if (ENABLED_IR_LED)
void SendIr(byte CommandNumber){
    //Turnf off lg tv
    //Apagar tv lg
     switch (CommandNumber) {
    case 1:    
      irsend.sendNEC(0x20DF10EF, 32); 
      delay(40);      
   
     } 
   
  //si estas usando receptor infrarrojos Reinicialo
  //if you are using infrared receiver restart it  
 //irrecv.enableIRIn(); // Re-enable receiver
}
#endif 


