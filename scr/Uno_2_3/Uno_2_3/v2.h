





//definir pines control SD
#define SS_SD 4
#define SS_ETHERNET 53 //53 for mega, for other set pin to 10

//Aumentar tamaño buffer UDP
#define UDP_TX_PACKET_MAX_SIZE 100 //increase UDP size

//Direccion de trabajo para modulos RTC.
#define DS_RTC 0x68  //Direccion Reloj

#ifdef moduleDS1307
  #define IC24C32_I2C_ADDRESS 0x50
#endif

#ifdef moduleDS3231
  #define IC24C32_I2C_ADDRESS 0x57
#endif





//Arrays constructor

const byte Number_Circuit = ( sizeof(Circuit_Type) ); 
const byte Number_SwicthInput= (sizeof(PinSwicthInput) ); //NSINPUT;
const byte Number_Input=( sizeof(PinInput) );
const byte Number_Output=( sizeof(PinOutput) );


struct Circuit {
  byte  Type;
  boolean Out1_Value;
  boolean Out2_Value;
  int Device_Number;
  byte Value;
  byte OldValue;
  byte CopyRef;
}circuits[Number_Circuit];

//MEMORIA GLOBAL

byte EspRfrIp = 0;


boolean Condicionados[N_CONDITIONED];            //Guarda el estado de los condicionados
byte Consignas[N_SETPOINTS];                     //Guarda el valor de las consignas
//Varibles Reloj
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year,minutoMemory,TipoDia;
//boolean HoraRetrasa;
unsigned long Tim30Sg,TimSg,TimNow;

//Variables Gestion Entradas Salidas
unsigned long LastTimeSwicthInput[Number_SwicthInput];  //Ultima vez que la entrada cambio el estado
int SwicthState[Number_SwicthInput];         // current state of the button
unsigned long LastTimeInput[Number_Input];  //Ultima vez que la entrada estaba en reposo
byte InState[Number_Input];  //Estado Entrada


//Variables Control Alarmas
boolean Alarms[N_ALARMS]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,};

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

#ifdef Historical_SD
  #include "historical_SD.h"
#endif

