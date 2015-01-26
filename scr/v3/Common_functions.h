/*
  Carga de modulos.
*/


#include "EXC_ram.h"
#include "EXC_eeprom.h"
#include "EXC_time.h"
#include "printLCD.h"



#include <Wire.h>



//Funciones externas que son configurables por el usuario.
extern void UserSetup();
extern void UserLoop();
extern void OutControl();
extern void LoopNewSecond();
extern void Loop30Sg();
extern void NewMinute();
extern void SwicthStateChange(byte NumberInput);
extern void LongInputEnd(byte NumberInput);
extern void LongInput(byte NumberInput);
extern void ShortInput(byte NumberInput);
extern String GetAlarmsName(byte Number);
extern void CommonOrders(byte CommandNumber);
extern char* ReadSensor(byte NumeroSensor);
extern char* RunCommand(byte CommandNumber);


// PROTOTIPOS
// Circuitos
void InputState();
void CheckSwicth();
void SelectScene(byte);

// Persianas.
void ReiniciarTiempoPersianas();
void SubirPersiana(byte NPersiana);
void BajarPersiana(byte);
void CargaPosicionPersiana(byte NPersiana);

//
void RecepcionPaqueteUDP();
//boolean CreateCabHTTP(String URL, String Key2);
boolean ComproRespuestaHTTP();
void connectAndRfr();
void EnvioEstadoActual();


void InitDS18B20();
void RefreshTemperature();

void ActualizaMinuto();
void CargaHora();
void saveCircuitValue();


#ifdef Historical_SD
  #include "historical_SD.h"
#endif

#ifdef IR_RECIVE   
  #include "InfraRedFunction.h"
#endif 

#if defined (ETHERNET_SHIELD) || defined (WIFI_SHIELD)
 #include "EXC_udp.h"
 #include "EXC_http.h"
#endif

//#ifdef RECEIVER_433
//  #include "Mhz433.h"
//#endif

//#ifdef THERMOSTAT_DS18B20_NUMBER
//  #include "Mhz433.h"
//#endif


#ifdef RETROAVISOS 
void ComprobarRetroavisos(){   
  int reading;
  
  for (int i=0;i<Number_Circuit;i++){ 
    if (circuits[i].Type==Ado_Retroaviso){
      reading = digitalRead(PinEstadoCircuito[circuits[i].Device_Number]);
      unsigned long InputMillis = millis();
      if (reading==circuits[i].Out2_Value){
        LastTimeEstadoRetroaviso[i]=InputMillis;}
      else{
        if(LastTimeEstadoRetroaviso[i] > InputMillis){
          LastTimeEstadoRetroaviso[i]=InputMillis;
        }
        if ((InputMillis-LastTimeEstadoRetroaviso[i])>=60){
          circuits[i].Out2_Value=reading;        
          if (reading==HIGH){
            circuits[i].Value=1;
          }
          else{
            circuits[i].Value=0;
          }
        }    
      }
    }
  }
}
#endif

void CheckSwicth(){
  int reading;
  for (byte i=0; i<Number_SwicthInput;i++){
    reading = digitalRead(PinSwicthInput[i]);
    unsigned long InputMillis = millis();
    if (reading==SwicthState[i]){
      LastTimeSwicthInput[i]=InputMillis;
    }
    else{
      if(LastTimeSwicthInput[i]>InputMillis){
        LastTimeSwicthInput[i]=InputMillis;}
      if ((InputMillis-LastTimeSwicthInput[i])>=60){
        SwicthState[i]=reading;
        SwicthStateChange(i);
        #ifdef DEBUG_MODE   
         Serial.print(F("Swicth change "));
         Serial.print(i);
         Serial.print(F(" to "));
         Serial.print(reading);
         Serial.print(F(" pin "));
         Serial.println(PinSwicthInput[i]);
        #endif

      }    
     }
  }
}

// PENDIENTE MEJORAR......
void GestionCircuitos(){
      
  //  ,,,,,,,Persiana,ConsignaTemp,,
  for (int c=0;c<Number_Circuit;c++){
    
    if (circuits[c].Type!=Reserva){
      if ((circuits[c].Type==Ado_Digital)||(circuits[c].Type==Puerta)||(circuits[c].Type==Enchufe)||(circuits[c].Type==EnchufeRF)||(circuits[c].Type==Riego)||(circuits[c].Type==Riego_Temporizado)||(circuits[c].Type==Frio)||(circuits[c].Type==Calor)||(circuits[c].Type==Valvula)||(circuits[c].Type==Radiante)||(circuits[c].Type==Ventilador)||(circuits[c].Type==Piloto)) {
        if (circuits[c].Value>=1){
          circuits[c].Out1_Value=On;
          circuits[c].Out2_Value=Off;
        }
        else{
          circuits[c].Out1_Value=Off;
          circuits[c].Out2_Value=Off;
        }
      }
      
      
      if (circuits[c].Type==Ado_3Etapas){
        switch (circuits[c].Value) 
        {
          case 0:    
            circuits[c].Out1_Value=Off;
            circuits[c].Out2_Value=Off;
          break;
          case 1:    
            circuits[c].Out1_Value=On;
            circuits[c].Out2_Value=Off;
          break;
          case 2:    
           circuits[c].Out1_Value=Off;
           circuits[c].Out2_Value=On; 
          break;
          case 3:    
             circuits[c].Out1_Value=On;
             circuits[c].Out2_Value=On; 
          break;
          default:    
            circuits[c].Value=0;
            circuits[c].Out1_Value=Off;
            circuits[c].Out2_Value=Off;
          break;
        } 
        }
      }
      if ((circuits[c].Type==Persiana)||(circuits[c].Type==Toldo)){//PersianaS
       circuits[c].Out1_Value=Off;
       circuits[c].Out2_Value=Off;
       if ((OutDowPersiana[circuits[c].Device_Number]==true)||(OutUpPersiana[circuits[c].Device_Number]==true))
       {
         if ((OutDowPersiana[circuits[c].Device_Number]==true)&&(OutUpPersiana[circuits[c].Device_Number]==false)){circuits[c].Out1_Value=On; circuits[c].Out2_Value=On;}
         if ((OutDowPersiana[circuits[c].Device_Number]==false)&&(OutUpPersiana[circuits[c].Device_Number]==true)){circuits[c].Out1_Value=On; circuits[c].Out2_Value=Off;}     
       }
      }
      //Gestion Enchufes Radio Frecuencia
     #ifdef ELECTRIC_OUTLET_433
       if (circuits[c].Type==EnchufeRF){
           if (circuits[c].Value!=circuits[c].OldValue){
             if (circuits[c].Value==1)
               Electric_Outlet_Control(circuits[c].Device_Number+1,true);
             else
               Electric_Outlet_Control(circuits[c].Device_Number+1,false);
           }
       }
     #endif 
  
  
     //Gestion retroaviso
     #ifdef RETROAVISOS   
        if (circuits[c].Type==Ado_Retroaviso){
          if (circuits[c].OldValue!=circuits[c].Value){
            if(circuits[c].Out1_Value==Off){
              circuits[c].Out1_Value=On;
            }
            else{
              circuits[c].Out1_Value=Off;
            }
          }       
        }
      #endif    
   }
}


//
void InputState(){  
  for (byte i=0; i<Number_Input;i++){
     unsigned long InputMillis = millis()-LastTimeInput[i];
     if ((InState[i]>=4)||(-1 >= InState[i])){
       InState[i]=0;
     }
     
     if (digitalRead(PinInput[i]) == LOW ) {
       if ((InState[i]==0)&&(InputMillis>=60)){
         LastTimeInput[i]=millis();
         InState[i]=1;
       }
       if ((InState[i]==1)&&(InputMillis>=440)){
         LongInput(i);
         InState[i]=2;
       }
       if (InState[i]==2){
         LastTimeInput[i]=millis();
       }
       if (InState[i]==3){
         LastTimeInput[i]=millis();
         InState[i]=1;
       }
     }
     else{
       if (InState[i]==0){
         LastTimeInput[i]=millis();
       }
       if (InState[i]==1){
         LastTimeInput[i]=millis();
         InState[i]=3;
       }
       if ((InState[i]==2) &&(InputMillis>=60)){
         LastTimeInput[i]=millis();
         InState[i]=0;
         LongInputEnd(i);
       }
       if ((InState[i]==3)&&(InputMillis>=60)){
         LastTimeInput[i]=millis();
         InState[i]=0;
         ShortInput(i);
       }
     }
  }
}

// Guarda el nuevo valor del circuito.
void saveCircuitValue(){
    for (int c=0;c<Number_Circuit;c++){
      circuits[c].OldValue=circuits[c].Value;
    }
}
/*************************************************************/
   //Gestion Persianas
/*************************************************************/ 

void SubirPersiana(byte NPersiana)
{
  if (OutDowPersiana[NPersiana]==true){
    BajarPersiana(NPersiana);
    OutDowPersiana[NPersiana]=false;
    OutControl();
    delay(200);
  }
  unsigned long TiempoActual = micros();
  if (OutUpPersiana[NPersiana]==false){OutUpPersiana[NPersiana]=true;}
  else{
    unsigned long DiferenciaTiempo;
    if (TiempoActual<TiempoMovPersiana[NPersiana]){DiferenciaTiempo=TiempoActual;}else{DiferenciaTiempo = TiempoActual-TiempoMovPersiana[NPersiana];}
    if ((TiempoPosPersianaUp[NPersiana] + DiferenciaTiempo)<TimUpPersiana[NPersiana]){TiempoPosPersianaUp[NPersiana]=TiempoPosPersianaUp[NPersiana] + DiferenciaTiempo;}
    else{TiempoPosPersianaUp[NPersiana]=TimUpPersiana[NPersiana];}
    byte porcentajeSubida = TiempoPosPersianaUp[NPersiana] / (TimUpPersiana[NPersiana]/100);
    byte porcentajeBajada=100-porcentajeSubida;
    PosicionPersiana[NPersiana]=porcentajeSubida;
    TiempoPosPersianaDown[NPersiana]=porcentajeBajada*(TimDowPersiana[NPersiana]/100);
  }  
  TiempoMovPersiana[NPersiana]=TiempoActual;
}
   
void BajarPersiana(byte NPersiana)
{
  if (OutUpPersiana[NPersiana]==true){
	SubirPersiana(NPersiana);
	OutUpPersiana[NPersiana]=false;
	OutControl();
	delay(200);
	}
  unsigned long TiempoActual = micros();
  if (OutDowPersiana[NPersiana]==false){
	OutDowPersiana[NPersiana]=true;
  }
  else{
    unsigned long DiferenciaTiempo;
    if (TiempoActual<TiempoMovPersiana[NPersiana]){
		DiferenciaTiempo=TiempoActual;
	}else{
		DiferenciaTiempo = TiempoActual-TiempoMovPersiana[NPersiana];
	}
    if ((TiempoPosPersianaDown[NPersiana] + DiferenciaTiempo)<TimDowPersiana[NPersiana]){
		TiempoPosPersianaDown[NPersiana]=TiempoPosPersianaDown[NPersiana] + DiferenciaTiempo;
	}else{
		TiempoPosPersianaDown[NPersiana]=TimDowPersiana[NPersiana];
	}
   
    byte porcentajeBajada = TiempoPosPersianaDown[NPersiana] / (TimDowPersiana[NPersiana]/100);
    byte porcentajeSubida=100-porcentajeBajada;
    PosicionPersiana[NPersiana]=porcentajeSubida;
    TiempoPosPersianaUp[NPersiana]=porcentajeSubida*(TimUpPersiana[NPersiana]/100);
  }  
  TiempoMovPersiana[NPersiana]=TiempoActual;
}

void GestionMovPersianas(byte NPersiana)
{
  if (InUpPersiana[NPersiana] || InDowPersiana[NPersiana])
  {  //Funcionamiento Manual
    if (InUpPersiana[NPersiana] && InDowPersiana[NPersiana]){
		OutDowPersiana[NPersiana]=false;OutUpPersiana[NPersiana]=false;
	}
    else{
      if (InUpPersiana[NPersiana]){SubirPersiana(NPersiana);} 
      if (InDowPersiana[NPersiana]){BajarPersiana(NPersiana);}
      circuits[LocalizadorPersiana[NPersiana]].Value=PosicionPersiana[NPersiana];    
    }    
  }
  else
  {  //Funcionamiento Automatico;
    if (circuits[LocalizadorPersiana[NPersiana]].Value==PosicionPersiana[NPersiana]){
      OutDowPersiana[NPersiana]=false;
      OutUpPersiana[NPersiana]=false;
    }
    else
    {      
       if (circuits[LocalizadorPersiana[NPersiana]].Value > PosicionPersiana[NPersiana]){
         SubirPersiana(NPersiana);
       }
       else{
         if (circuits[LocalizadorPersiana[NPersiana]].Value< PosicionPersiana[NPersiana]){
         BajarPersiana(NPersiana);}
       }    
    }  
  }
}

//
void ReiniciarTiempoPersianas()
{
  for ( byte c =0; c < NumeroPersianas; c++){
    TimUpPersiana[c]=(EepromRead(EM_UP_TIM_SHUTTER_OFFSET + c))*  1000000; 
    TimDowPersiana[c]=(EepromRead(EM_DO_TIM_SHUTTER_OFFSET + c))* 1000000;
  }
}

//
void ReiniciarPosicionPersiana(byte NumPersiana)
{
  TiempoPosPersianaUp[NumPersiana]=0;
  TiempoPosPersianaDown[NumPersiana]=TimDowPersiana[NumPersiana];
  circuits[LocalizadorPersiana[NumPersiana]].Value=100;
}

void CargaPosicionPersiana(byte NPersiana)
{
  PosicionPersiana[NPersiana]=circuits[LocalizadorPersiana[NPersiana]].Value;
  byte porcentajeBajada=100-PosicionPersiana[NPersiana];
  TiempoPosPersianaDown[NPersiana]=porcentajeBajada*(TimDowPersiana[NPersiana]/100);
  TiempoPosPersianaUp[NPersiana]=PosicionPersiana[NPersiana]*(TimUpPersiana[NPersiana]/100);
}


void timeChangeCircuit(int addressEE)
{
  byte ci=EepromRead(addressEE);
  byte val=EepromRead(addressEE+1);
  
  if (ci < 30)
  {
    circuits[ci].Value=val;
  }
  else if(ci < 40)
  {
    SelectScene(ci-29);            
  }  
  else if (ci < 50)
  {
    if (val==1){
      Condicionados[ci-40]=true;
    }
    else{
      Condicionados[ci-40]=false;
    }
  }
    
}


//Modificado 2.3
void SelectScene(byte Dir)
{
  int p;
  byte c,v;
  
  p= (int) Dir;
  p= EM_ESCENES_OFFSET + ((p-1) * S_ESCENES );
  for (c =0 ; c<Number_Circuit; c++)
  {
    v =EepromRead(p + c); 
    if (v < 250)
    {
      circuits[c].Value = v;
    }
  }
}

void CargaHora()
{
  getDateDs1307();  
  Tim30Sg=millis();
  if (minute != minutoMemory)
  {
    ActualizaMinuto();
    NewMinute();
  }
}


//Envento cada minuto.
void ActualizaMinuto()
{  
    //if ((hour==0)&&minute==0)){}//CalculaCrepusculo();}
    #ifdef HISTORICAL_SD
      if ((minute==0)||(minute==15)||(minute==30)||(minute==45)){GuardaHistorico();}
    #endif
    
    //Riego Temporizado 
    for (byte c=0;c<Number_Circuit;c++)
    {
      if ((circuits[c].Type==Riego_Temporizado)&&(circuits[c].Value>=1)){
        circuits[c].Value--;
      }
    }
  
    //Adelanta la hora.Apartir del dia 25 de Marzo, busca el primer domingo
    //y cuando se han las 2 de la noche adelanta el reloj una hora
    if(Enable_DaylightSavingTime==true && minute==0){
      if(month==3 && dayOfMonth >= 26 && dayOfWeek == 7 && hour==2){
        hour = 3;     
        setDateDs1307(/*second, minute, hour, dayOfWeek, dayOfMonth, month, year*/);
      }
      if(month==10 && dayOfMonth >= 26 && dayOfWeek == 7 && hour==3){
        if (HoraRetrasa==false){
          HoraRetrasa=true;
          hour = 2;
          setDateDs1307(/*second, minute, hour, dayOfWeek, dayOfMonth, month, year*/);
        }
      }
      
    }
    if (hour==3){HoraRetrasa=false;}      

    
    minutoMemory=minute;
    
    TipoDia=dayOfWeek;
    
    int Reg;
    //Verificacion Dia Especial 1
    for (Reg=EM_DATE_ESPECIAL1_OFSSET; Reg <= EM_DATE_ESPECIAL1_END; Reg=Reg +S_DATE_ESPECIAL){
      if (month == EepromRead(Reg) && dayOfMonth== EepromRead(Reg+1)){      
          TipoDia=8;
      }
    }
    //Verificacion Dia Especial 2
    for (Reg=EM_DATE_ESPECIAL2_OFSSET; Reg <= EM_DATE_ESPECIAL2_END; Reg=Reg +S_DATE_ESPECIAL){
       if (month == EepromRead(Reg) && dayOfMonth== EepromRead(Reg+1)){      
          TipoDia=9;
      }
    }
    
    

    int r= ((TipoDia-1)* EM_TIME_DAY_SIZE)+ EM_TIME_WEEKLY_OFFSET;
    
    //Read Timetable for days.
    for (Reg=r; Reg <=(r+EM_TIME_DAY_SIZE);Reg=Reg + S_TIME_ESPECIAL)
    {
      if ( hour==EepromRead(Reg) && minute==EepromRead(Reg+1))
      {
        timeChangeCircuit(Reg+2);
      }
    }

    //Read Triggers 
    for (Reg=EM_TRIGGER_OFFSET; Reg <= EM_TRIGGER_END ; Reg=Reg + S_TRIGGER)
    {
      if ( hour==EepromRead(Reg) && minute==EepromRead(Reg+1))
      {
        EepromWrite(Reg, 66);   //Remove the trigger.
        timeChangeCircuit(Reg+2);
      }
    }
}

//******************************************************************/
// REFRESCAR
/*****************************************************************/


#ifdef THERMOSTAT_DS18B20_NUMBER
  void InitDS18B20(){
      sensorTemp.begin();
      for (int c=0;c<THERMOSTAT_DS18B20_NUMBER;c++){sensorTemp.setResolution(Ds18B20Addres[c], TEMPERATURE_PRECISION);ThermostatHeat[c]=false;ThermostatCool[c]=false;}
      sensorTemp.setWaitForConversion(true);
      sensorTemp.requestTemperatures();
      RefreshTemperature();
      sensorTemp.setWaitForConversion(false);
   }

   void RefreshTemperature(){
     for (int c=0;c<THERMOSTAT_DS18B20_NUMBER;c++){Temperature[c] = sensorTemp.getTempC(Ds18B20Addres[c]);}
     for (int c=0;c<Number_Circuit;c++){      
       if (circuits[c].Type==ConsignaTemp){         
         float RangoTemp =((float) circuits[c].Value )+ 0.5;        
         if (Temperature[circuits[c].Device_Number]   >= RangoTemp ){ThermostatCool[circuits[c].Device_Number]=true;}
         else{RangoTemp = RangoTemp - 1;if (Temperature[circuits[c].Device_Number] <= RangoTemp){ThermostatCool[circuits[c].Device_Number]=false;}}
         RangoTemp =((float) circuits[c].Value) - 0.5;
         if (Temperature[circuits[c].Device_Number]<= RangoTemp ){ThermostatHeat[circuits[c].Device_Number]=true;} 
         else{RangoTemp = RangoTemp +1;if (Temperature[circuits[c].Device_Number] >= RangoTemp){ThermostatHeat[circuits[c].Device_Number]=false;}}  
       }
     }
     sensorTemp.requestTemperatures();
  }
#endif 

//char* CharNull(char* Val){
//      int LongCad= strlen(Val); 
//      if (Val[LongCad-1]=='\0'){return Val;}
//      char Resultado[LongCad + 1];
//      Resultado[LongCad]='\0';
//      for (int c=0;c<LongCad;c++){Resultado[c]=Val[c];}
//      return Resultado;
//}




//Funciones alarmas
void SetAlarm(int Number){if ((Number<=19)&&(Alarms[Number]==0)){Alarms[Number]=1;}}
void ResetAlarm(int Number){if ((Number<=19)&&(Alarms[Number]==2)){Alarms[Number]=0;}}


//Envento segundo
//Se ejecutan una serie de funciones.
void loopSecond(){
  //Notifications.  
  //
  #if defined (ETHERNET_SHIELD) || (WIFI_SHIELD)
    connectAndRfr();
  #endif
  // 
  LoopNewSecond();
}

//Evento 30 segundos
void loop30Second(){
  CargaHora();

  #ifdef HISTORICAL_SD
    if (SdOk){
      SecutityCopy();
    }
  #endif 
  Loop30Sg();

  
}
//Eventos Minuto.
void loopMinute(){
  
  
  #ifdef HTTP_CONNET 
  if (Connecting==false){
    for (int a=0;a<=19;a++){
      if (Alarms[a]==1){
        boolean res=Notification(GetAlarmsName(a));
        if (res){Alarms[a]=2;}
        break;
      }  
    }
  }
  #endif
}



//   Iniciar el sistema....
void setup()
{
  #ifdef HISTORICAL_SD 
    initPinSD();
    initSD();
  #endif
  
  #ifdef RECEIVER_433  
    Init433Mhz(); // Receiver on inerrupt 0 => that is pin #2
  #endif
 
 #if defined (TRANSMITER_433)  || defined (ELECTRIC_OUTLET_433)
   mySwitch.enableTransmit(11);
 #endif 
 
 #ifdef THERMOSTAT_DS18B20_NUMBER
   InitDS18B20();
 #endif 
 
 #ifdef IR_RECIVE
   irrecv.enableIRIn();
 #endif
 
 #ifdef ENABLE_WATCH_DOG
   wdt_enable(WDTO_8S );
 #endif
  
  byte ControlPersiana=0;
  byte ControlEnchufe=0;
  byte ControlTermostato=0;
  byte ControlRetroaviso=0;
  byte c;  
  //enum CircuitsTypes {Persiana,ConsignaTemp,};
   #ifdef DEBUG_MODE
      Serial.begin(9600);   
      Serial.print("System started with ");
      Serial.print(Number_Circuit);
      Serial.print(" circuit, types ");
   #endif
  for (c=0;c<Number_Circuit;c++){
    circuits[c].Type=Circuit_Type[c];        
    circuits[c].Device_Number=0;
    circuits[c].Out1_Value=Off;
    circuits[c].Out2_Value=Off;
    circuits[c].Value=0;
    circuits[c].CopyRef=0;
 //   circuits[c].Type=TipeCir[c];
    #ifdef DEBUG_MODE     
        Serial.print(circuits[c].Type);
        Serial.print(" ");   
    #endif

    if ((circuits[c].Type==Persiana)||(circuits[c].Type==Toldo)){
      LocalizadorPersiana[ControlPersiana]=c;
      circuits[c].Device_Number=ControlPersiana;
      ControlPersiana++;
    }
    
    #ifdef ELECTRIC_OUTLET_433  
      if (circuits[c].Type==EnchufeRF){
        circuits[c].Device_Number=ControlEnchufe;
        ControlEnchufe++;
        circuits[c].OldValue=99;
      }
    #endif
    
    #ifdef THERMOSTAT_DS18B20_NUMBER
      if (circuits[c].Type==ConsignaTemp){
        circuits[c].Device_Number=ControlTermostato;
        ControlTermostato++;
      }
    #endif
    
    #ifdef RETROAVISOS    
    if (circuits[c].Type==Ado_Retroaviso){
      circuits[c].Device_Number=ControlRetroaviso;
      ControlRetroaviso++;
    }
    #endif
  }
  #ifdef DEBUG_MODE
    Serial.println();
  #endif
  
  #ifdef WIFI_SHIELD
     if (WiFi.status() == WL_NO_SHIELD) {
       #ifdef DEBUG_MODE   
          Serial.println("WiFi shield not present"); 
        #endif
      while(true);// don't continue:
    } 
    WiFi.config(ip);
    ConexionWifi();     
  #endif
    
//     W5100.setRetransmissionTime(0x07D0);  //setRetransmissionTime sets the Wiznet's timeout period, where each unit is 100us, so 0x07D0 (decimal 2000) means 200ms.
//     W5100.setRetransmissionCount(3);      //setRetransmissionCount sets the Wiznet's retry count.
     //Ethernet.begin(mac);//DHCP MODE 
  #ifdef ETHERNET_SHIELD
    initUDP();
//     Ethernet.begin(mac,ip);
//     Udp.begin(localPort);
    /* #ifdef DEBUG_MODE   
         Serial.println("give the Ethernet shield a second to initialize:");               
      #endif
      delay(3000);
      #ifdef DEBUG_MODE   
         Serial.println("time completed");               
      #endif*/
  #endif
  
  for (int per=0; per<NumeroPersianas; per++){
    InDowPersiana[per]=false;
    InUpPersiana[per]=false;
  }  
  
  //Fijamos pines entrada salida
  unsigned long currenMillis = millis();
  //startDs1307();
  //setDateDs1307(0,49,01,6,15,11,2014);
  #ifdef RETROAVISOS     
    for (int c=0;c<Retroavisos_Number;c++){
      pinMode(PinEstadoCircuito[c], INPUT);
      #ifdef INTERNAL_RESISTOR
       digitalWrite(PinEstadoCircuito[c], HIGH);       // turn on pullup resistors
      #endif
    }
  #endif
  for (int i=0; i<Number_Input;i++){
    pinMode(PinInput[i], INPUT);
    #ifdef INTERNAL_RESISTOR
     digitalWrite(PinInput[i], HIGH);       // turn on pullup resistors
    #endif
    LastTimeInput[i]=currenMillis;
	InState[i]=0;
  }

  for (int i=0; i<Number_SwicthInput;i++){
	pinMode(PinSwicthInput[i], INPUT);
    #ifdef INTERNAL_RESISTOR
     digitalWrite(PinSwicthInput[i], HIGH);       // turn on pullup resistors
    #endif
    SwicthState[i] = digitalRead(PinSwicthInput[i]);
	LastTimeSwicthInput[i]=millis();
  }
  int i;
  
  // Config pinOuts 
  for (i=0; i<Number_Output;i++){
    pinMode(PinOutput[i], OUTPUT);
    digitalWrite(PinOutput[i],Off);
  }
  
  for (i=0; i < N_SETPOINTS; i++)
  {
    Consignas[i]=EepromRead(EM_SETPOINTS_OFSSET + i);
  }
  //Restore alarm resgisters.
  for (i=0; i < N_ALARMS; i++){
    Alarms[i]=EepromRead(EM_ALARMS_OFSSET+i);
    if (Alarms[i]>=5){
      Alarms[i]=0;
    }
  }
  //Fijamo valores y posicion inicio persianas
  //Fijamos el tiempo de subida bajada Persianas
  //Se encuentran apartir de la direccion 480 
  ReiniciarTiempoPersianas();
  
  #ifdef HISTORICAL_SD
    if(CargaSdOk){
     //las persianas ajustar posicion
     for (int per=0; per<NumeroPersianas; per++){CargaPosicionPersiana(per); }   
   }
   //PENDIENTE....
     else{
    //cargamos circuitos de eeprom
   }
  #endif 
   
//Inicio control Horarios
  Wire.begin();
  #ifdef DEBUG_MODE
     Serial.print(">hora");     
   #endif
  CargaHora(); 

  UserSetup();
}

void loop(){
  TimNow=millis(); 
  
   if (!(TimNow % 1000)){
       loopSecond();
       if(!(TimNow % 30000)){
         loop30Second();
         
         if(!(TimNow % 60000)){
           loopMinute();
         }
       }
   }   

  // Actualiza valor retroavisos.  
  #ifdef RETROAVISOS 
    for (int c=0;c<Number_Circuit;c++){
      if (circuits[c].Type==Ado_Retroaviso){
        circuits[c].OldValue=circuits[c].Value;
      }
    }
  #endif
  
  #ifdef ENABLE_WATCH_DOG
    wdt_reset();
  #endif
    
  #ifdef IR_RECIVE   
    ComprobarInfrarro();
  #endif 
  
  #ifdef RECEIVER_433  
      Recepcion433Mhz();
  #endif

  #ifdef THERMOSTAT_DS18B20_NUMBER
    RefreshTemperature();
  #endif
  
  #ifdef WIFI_SHIELD
    // Si no existe conexion wifi reintento conexion cada 60 segundos.
    if (WiFi.status() != WL_CONNECTED) {
      unsigned int TimActual=millis();
      if (TimActual<=TimConexion){
        TimConexion=millis();
      }
    else{
      if ((TimActual-TimConexion)>=60000){
        ConexionWifi();
      }
    }
    }else{
      TimConexion=millis();
      RecepcionPaqueteUDP();
    }
  #elif defined ETHERNET_SHIELD
    RecepcionPaqueteUDP();
  #endif
  
  InputState();
  CheckSwicth();
   
   //Control de movimiento persianas
   //Cambiar tiene que estar en gesitonCircuitos.
   for (int p =0; p<NumeroPersianas;p++){
     GestionMovPersianas(p);
   } 
   //Control de retroavisos
   //En realidad es un objeto circuito.
   #ifdef RETROAVISOS 
    ComprobarRetroavisos(); 
   #endif   
   
   GestionCircuitos(); 
   OutControl();
   UserLoop();
   saveCircuitValue();

}


