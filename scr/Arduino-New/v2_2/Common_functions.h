
//
extern void UserSetup();
extern void UserLoop();          /// En ciclo loop se tendria que cambiar a principal.

extern void OutControl();
extern void InControl();
extern void NewMinute();
extern void Loop30Sg();

// Transformar a constructor dentro root.
extern void CommonOrders(byte CommandNumber);
extern char* ReadSensor(byte NumeroSensor);
extern char* RunCommand(byte CommandNumber);
extern String GetAlarmsName(byte Number);


// Esto tiene que ser una llamada del principal.
extern void LongInputEnd(byte NumberInput);
extern void LongInput(byte NumberInput);
extern void ShortInput(byte NumberInput);
extern void SwicthStateChange(byte NumberInput);
extern void LoopNewSecond();

// PROTOTIPOS
void CargaHora();
void InputState();
void CheckSwicth();
void EnvioEstadoActual();
void SelectScene(byte);
void ActualizaMinuto();
void SubirPersiana(byte NPersiana);
//void CargarTiempoPersianas();
void ReiniciarTiempoPersianas();
void connectAndRfr();
void RecepcionPaqueteUDP();
void BajarPersiana(byte);
void CargaPosicionPersiana(byte NPersiana);
boolean CreateCabHTTP(String URL, String Key2);
boolean ComproRespuestaHTTP();
void SystemLoop();
void ConexionWifi();
void InitDS18B20();
void RefreshTemperature();
/*************************************************************************** 
  SUBRUTINAS TRATAMIENTO EEPROM
  DETECTA EL MICROCONTOLADOR DE LA TARJETA CONFIGURADA EN EL IDE ARDUINO 
  CAMBIAMOS EL USO DE EEPROM INTERNO O EXTERNO 
  ACTIVADO MODO USO EEPROM EXTERNA !!!! 
****************************************************************************/
#ifdef ARDUINO_MEGA
//    ARDUINO MEGA..........................................................   
    
    #define EepromRead   EEPROM.read
	
    void EepromWrite ( unsigned int eeaddress, byte data ){
      if(EepromRead(eeaddress) != data)
      EEPROM.write(eeaddress, data);
    }
   
#else
//    ARDUINO CON Atmeg328 (ARDUINO UNO,ARDUINO ETHERNET. ETC.............
///   USO DE MEMORIA EXTERNA.
   
      byte EepromRead( unsigned int eeaddress ) {
		unsigned long startingTime = millis();
		
        byte rdata = 0xFF;
        TWBR = 18;        //Change speed bus I2C. For 400KHz used 12
        Wire.beginTransmission(IC24C32_I2C_ADDRESS);
        Wire.write((int)(eeaddress >> 8)); // MSB
        Wire.write((int)(eeaddress & 0xFF)); // LSB
        Wire.endTransmission();
        Wire.requestFrom(IC24C32_I2C_ADDRESS,1);
        while(Wire.available() == 0){
          if((millis() - startingTime) > 2){
            Wire.begin();
            break;
          }
        }
        rdata = Wire.read();
        TWBR = 72;
        return rdata;
      }
    
  void EepromWrite( unsigned int eeaddress, byte data ) {
    
    if(EepromRead(eeaddress) == data) return;       
        Wire.beginTransmission(IC24C32_I2C_ADDRESS);
        Wire.write((int)(eeaddress >> 8));       // MSB
        Wire.write((int)(eeaddress & 0xFF));     // LSB
        Wire.write(data);
        Wire.endTransmission();
		//Espera buffer este vacio o tiempo de salida.
		//while(twi_state != 0 ){
		//	if((millis() - startingTime) >= 5)
		//	{
		//		Wire.begin();
		//		break;
		//	}
		//}
		
        delay(5);           // Retardo para asegurar escritura, dependiendo de la memoria 
                            // puede ser inferior.
        TWBR = 72;          // Speed bus I2C standard 100KHz.
      }
#endif

/******************************************************************/
//  FUNCIONES RELOJ
/*****************************************************************/

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// Stops the DS1307, but it has the side effect of setting seconds to 0
// Probably only want to use this for testing
/*void stopDs1307()
{
  Wire.beginTransmission(DS_RTC);
  Wire.send(0);
  Wire.send(0x80);
  Wire.endTransmission();
}*/

// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers
void setDateDs1307(byte second,        // 0-59
                   byte minute,        // 0-59
                   byte hour,          // 1-23
                   byte dayOfWeek,     // 1-7
                   byte dayOfMonth,    // 1-28/29/30/31
                   byte month,         // 1-12
                   byte year)          // 0-99
{
   Wire.beginTransmission(DS_RTC);
   Wire.write(0);
   Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
   Wire.write(decToBcd(minute));
   Wire.write(decToBcd(hour));      // If you want 12 hour am/pm you need to set
                                   // bit 6 (also need to change readDateDs1307)
   Wire.write(decToBcd(dayOfWeek));
   Wire.write(decToBcd(dayOfMonth));
   Wire.write(decToBcd(month));
   Wire.write(decToBcd(year));
   Wire.endTransmission();
}

// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
          byte *minute,
          byte *hour,
          byte *dayOfWeek,
          byte *dayOfMonth,
          byte *month,
          byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS_RTC);
  Wire.write(0);
  Wire.endTransmission();
 
  Wire.requestFrom(DS_RTC, 7);

  // A few of these need masks because certain bits are control bits
  if (Wire.available()==7){
    *second     = bcdToDec(Wire.read() & 0x7f);
    *minute     = bcdToDec(Wire.read());
    *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
    *dayOfWeek  = bcdToDec(Wire.read());
    *dayOfMonth = bcdToDec(Wire.read());
    *month      = bcdToDec(Wire.read());
    *year       = bcdToDec(Wire.read());
  }
}

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
    circuits[c].Out1_Value=LOW;
    circuits[c].Out2_Value=LOW;
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

  for (int per=0; per<NumeroPersianas; per++){
    InDowPersiana[per]=false;
    InUpPersiana[per]=false;
  }  
  
  //Fijamos pines entrada salida
  unsigned long currenMillis = millis();
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
   
  

  
  #ifdef WIFI_SHIELD
     if (WiFi.status() == WL_NO_SHIELD) {
       #ifdef DEBUG_MODE   
          Serial.println("WiFi shield not present"); 
        #endif
      while(true);// don't continue:
    } 
    WiFi.config(ip);
    ConexionWifi();
  #else
    
//     W5100.setRetransmissionTime(0x07D0);  //setRetransmissionTime sets the Wiznet's timeout period, where each unit is 100us, so 0x07D0 (decimal 2000) means 200ms.
//     W5100.setRetransmissionCount(3);      //setRetransmissionCount sets the Wiznet's retry count.
     //Ethernet.begin(mac);//DHCP MODE  
     Ethernet.begin(mac,ip);
     Udp.begin(localPort);
  #endif
   
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
#ifdef WIFI_SHIELD
void ConexionWifi() {
    #ifdef DEBUG_MODE   
        Serial.println("Iniciando Conexon Wifi");  
    #endif
    
    if (Net_Type == OPEN){WiFi.begin(ssid);  }//Open Network
    if (Net_Type == WEP){WiFi.begin(ssid, pass); }//WPA NETWORK
    if (Net_Type == WPA){WiFi.begin(ssid, keyIndex, pass); }//WEP 
    
    #ifdef ENABLE_WATCH_DOG
      wdt_disable();
    #endif 
    
    delay(10000);  //Esperamos 10 segundos para conexion
    #ifdef ENABLE_WATCH_DOG
      wdt_enable(WDTO_8S); 
    #endif 
    
    TimConexion=millis();
}
#endif

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
            circuits[i].OldValue=1;
            circuits[i].Value=1;
          }
          else{
            circuits[i].OldValue=0;
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

// NO ENTENDER????
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
             
             if (circuits[c].Value==1){Electric_Outlet_Control(circuits[c].Device_Number+1,true);}else{Electric_Outlet_Control(circuits[c].Device_Number+1,false);}
             circuits[c].OldValue=circuits[c].Value;
           }
         }
     #endif 
  
  
     //Gestion retroaviso
     #ifdef RETROAVISOS   
        if (circuits[c].Type==Ado_Retroaviso){
          if (circuits[c].OldValue!=circuits[c].Value){if(circuits[c].Out1_Value==Off){circuits[c].Out1_Value=On;}else{circuits[c].Out1_Value=Off;}}       
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

/*Modificacion principal.....
	Modificados:
		ESTADOINST
		READHOR  Antes (envio 320) Ahora (envio 80) + 3  x HOREAD 		
	Nuevos comandos:
		HOREAD	 Lectura de Horarios partidos 80 bytes...
		HORWRI   Escritura de Horariso partidos 80 bytes...
		HIST
		CARG
		ALRM
		SETNOTI
	
	ESTADOINST Cambio ???
*/

//MODIFICADO USO PUNTERO.
void EnviarRespuesta(char  *ReplyBuffer)
{
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
}

void RecepcionPaqueteUDP(){
 
  const char COMPLETED='%';
  const char EXTERNAL_REPLY='@';
  
  int  indexstr = 0;
  int c, p, indexdata;
  char adata[1];
  byte data;
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 
  
  //if (Connecting==true){return;}//hay que probarlo con y sin!
  #ifdef DEBUG_MODE
	//Serial.println(F("freeUDP()="));
	//Serial.println(freeMemory());
  #endif
  p = Udp.parsePacket();  
  
  if(p>0){    
     #ifdef DEBUG_MODE        
      Serial.println(F("UDP Packet recive"));   
    #endif
    
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    //Test secure connection, return if not same.
    if  (SecureConnection){
      for(c=0; c<8;c++){
        if (Key[7-c]!=packetBuffer[p -(c+1)]){
          return;
        }
      }
    }   
    
    // Test comand receiver. Execute and request.
    if (strncmp(packetBuffer, "COMCOMM", 7)==0){
      CommonOrders(packetBuffer[7]);
      strcpy(packetBuffer , "COMCOMOK");  
    }
    else if (strncmp(packetBuffer, "ALRM", 4)==0){
      strcpy(packetBuffer,"ESAL"); 
      for (byte  i = 4; i<24;i++){packetBuffer[i]=Alarms[i-4]+1;}    
      packetBuffer[24]='\0';
    }
    else if (strncmp(packetBuffer, "SETNOTI", 7)==0){
       byte pos=packetBuffer[7]-1;
       Alarms[pos]=packetBuffer[8]-1;
       strcpy(packetBuffer, "WIALMO");  
       EepromWrite(EM_ALARMS_OFSSET + pos,Alarms[pos]);
    }
       
    else if (strncmp(packetBuffer, "SVAL", 4)==0){
      data = packetBuffer[5]-1;    
      circuits[packetBuffer[4]-1].Value=data;
      EnvioEstadoActual();
      packetBuffer[0]=EXTERNAL_REPLY;
    }    
    else if (strncmp(packetBuffer, "VACT", 4)==0){
      int LongCad;
      #ifdef THERMOSTAT_DS18B20_NUMBER
        LongCad=35+THERMOSTAT_DS18B20_NUMBER;
      #else  
        LongCad=35;
      #endif 
    
      char Respuesta[LongCad];
      byte  indexstr,c;
    
      strcpy(Respuesta, "VVAL"); 
      indexstr=4;
    
      for (c=0; c<30;c++)
      {
         if ((c)<Number_Circuit){
           Respuesta[indexstr]=circuits[c].Value+1;
         }
         else{
           Respuesta[indexstr]=1;
         }  
         indexstr++;
      }
  
    #ifdef THERMOSTAT_DS18B20_NUMBER
      int v;
      for (int t=0; t<THERMOSTAT_DS18B20_NUMBER;t++)
      {
        if ((Temperature[t] >=1)&&(Temperature[t]<=49)){
          v =(Temperature[t]*10)/2;
          Respuesta[indexstr++]=v+1;
        }
        else{
          Respuesta[indexstr++]=1;
        }
      }
    #endif 
      Respuesta[indexstr]='\0';
      EnviarRespuesta(Respuesta); 
      packetBuffer[0]=EXTERNAL_REPLY;
    }
	// MODIFICADO cambio total de estructura.
    else if (strncmp(packetBuffer, "CARG", 4)==0){ 
      strcpy(packetBuffer, "VALC");
      indexstr=4;
	  //Envio buffer 34 los no asignados parecen 1
      for (c=0; c < 30; c++){
        if (c < Number_Circuit){
          //excepción PENDIENTE EN APPP Ado_Retroaviso=4
          if (circuits[c].Type==Ado_Retroaviso){
            packetBuffer[indexstr]=Ado_Digital;
          }
          else{
            packetBuffer[indexstr]=circuits[c].Type;
          }
        }
        else{
          packetBuffer[indexstr]=1;
        }
        indexstr++;
      }
      packetBuffer[indexstr]='\0';
    }
    	
    else if (strncmp(packetBuffer, "CLEARHORARIO", 11)==0){
           
      for (p = EM_TRIGGER_OFFSET; p <= EM_TIME_ESPECIAL2_END; p++){          // Size slot 80 * 4bytes data *7 day of the week + 
        EepromWrite(p, 66);
      }
      strcpy(packetBuffer, "HORARIOS BORRADOS");     
    }
    else if (strncmp(packetBuffer, "CLEARESPCDAY", 12)==0){
      for (p = EM_DATE_ESPECIAL1_OFSSET; p <= EM_DATE_ESPECIAL2_END; p++){          // size slot (100)  50 * 4bytes data * 2 special days
        EepromWrite(p, 0);
      }
      strcpy(packetBuffer, "DIAS ESPECIALES BORRADOS");     
    }
    else if (strncmp(packetBuffer, "SETFH", 5)==0){
      setDateDs1307(packetBuffer[5] ,packetBuffer[6], packetBuffer[7], packetBuffer[8], packetBuffer[9], packetBuffer[10], packetBuffer[11]);
      CargaHora();
      strcpy(packetBuffer, "SETFHOK");
    }
    else if (strncmp(packetBuffer, "GETSENSOR", 9)==0){          
      adata[0]=packetBuffer[9];                // store the number of sensor
      strcpy(packetBuffer, "SENSOR");     
      strncat(packetBuffer, adata,1);     
      strcat(packetBuffer, ReadSensor(adata[0]));
    }  
    else if (strncmp(packetBuffer, "READDAY", 7)==0)
    {
      if (packetBuffer[7]=='2'){
        p=EM_DATE_ESPECIAL2_OFSSET;                                // Pointer, first address memory slot. Eeprom.
      }
      else{
        p=EM_DATE_ESPECIAL1_OFSSET;
      }
      strcpy(packetBuffer,"CFDA");             
           
      indexstr=4;                              // Index to string constructor.
      for (c = 0; c<EM_DATE_ESPECIAL_SIZE; c++){                  // Number of Iterations = slot size.
        packetBuffer[indexstr]=EepromRead(p);  // Read Eeprom, data stored in the same packetBuffer
        indexstr++;
        p++;
      }
      packetBuffer[indexstr]='\0';              // End of string. NULL termintation.        
              
    }    
    else if (strncmp(packetBuffer, "WRIDAYE", 7)==0)
    {
       
       if (packetBuffer[7]==2){
         p=EM_DATE_ESPECIAL2_OFSSET;
       }
       else{
         p=EM_DATE_ESPECIAL1_OFSSET;
       }
         
       indexdata=8;                               //Index to first data in packetBuffer. 
       
       for (c=0; c<EM_DATE_ESPECIAL_SIZE; c++){
           EepromWrite(p++, packetBuffer[indexdata++]);    //Write de data.        
       }
       packetBuffer[0]=COMPLETED;              
    }      
    else if (strncmp(packetBuffer, "RETRIGGER", 9)==0)
    {      
      strcpy(packetBuffer , "TIGR");
      indexstr=4;    
      for (p=EM_TRIGGER_OFFSET; p <= EM_TRIGGER_END; p++){
        packetBuffer[indexstr++] = EepromRead(p)+1;        
      }
      packetBuffer[indexstr]='\0';      
    }    
    else if (strncmp(packetBuffer, "WTGR", 4)==0){
      indexdata=4;      
      for (p=EM_TRIGGER_OFFSET; p <= EM_TRIGGER_END; p++){
        EepromWrite(p, packetBuffer[indexdata++]);
      }
      packetBuffer[0]=COMPLETED;         
    }   
    else if (strncmp(packetBuffer, "READHOR", 7)==0){ 
		// p=packetBuffer[7]*320+1000;
       p=(packetBuffer[7] * EM_TIME_DAY_SIZE) + EM_TIME_WEEKLY_OFFSET;
       strcpy(packetBuffer, "EHR");       
       packetBuffer[3]=1;  
       indexstr=4;  
       for (c = 0; c<80; c++){
         packetBuffer[indexstr++]=EepromRead(p++)+1;
       }
       packetBuffer[indexstr]='\0';   
    }
    else if (strncmp(packetBuffer, "HOREAD", 6)==0){
      p=(packetBuffer[7] * EM_TIME_DAY_SIZE) + EM_TIME_WEEKLY_OFFSET;
      p+=(packetBuffer[6]-1) * 80;
      strcpy(packetBuffer, "EHR");
      packetBuffer[3]=packetBuffer[6];
      indexstr=4;
      
      for (c = 0; c<80; c++){
        packetBuffer[indexstr++]=EepromRead(p++)+1;
      }
      packetBuffer[indexstr]='\0';	   
    }
    else if (strncmp(packetBuffer, "HORWRI", 6)==0){
      p=(packetBuffer[7] * EM_TIME_DAY_SIZE) + EM_TIME_WEEKLY_OFFSET;
      data=packetBuffer[6];
      p+=(data-1)*80;
      indexdata=8;
      for (c=0; c<80 ;c++){
         EepromWrite(p++, packetBuffer[indexdata++]);      
       }
	   strcpy(packetBuffer, "HWRT");
	   packetBuffer[4]=data;
	   packetBuffer[5]='\0';
    }
    else if (strncmp(packetBuffer, "SSCE", 4)==0){     
      SelectScene(packetBuffer[4]);
      EnvioEstadoActual();
      packetBuffer[0]=EXTERNAL_REPLY;
    }
    //ESCENES
    else if (strncmp(packetBuffer, "WESC", 4)==0){
      p=(int) packetBuffer[4];          
      p = EM_ESCENES_OFFSET + ( (p-1) * S_ESCENES ) ;
      indexdata=5;      
      for (c = 0; c < S_ESCENES; c++){
         EepromWrite(p++, packetBuffer[indexdata++]-1);
       }
       packetBuffer[0]=COMPLETED;       
    }
    else if (strncmp(packetBuffer, "RESC", 4)==0){
       indexdata=(int) packetBuffer[4];
       p = ((indexdata -1) * S_ESCENES ) + EM_ESCENES_OFFSET;    
       strcpy(packetBuffer,"VESC");     
       indexstr=4;
       packetBuffer[indexstr++]=indexdata;
      
       for (c = 0; c < S_ESCENES; c++){
         data=EepromRead(p++);
         if (data<=254){data++;}
         packetBuffer[indexstr++]=data;
       }
       packetBuffer[indexstr]='\0';      
    }        
    else if (strncmp(packetBuffer, "ESTADOINST",10)==0){      
      strcpy(packetBuffer, "ESTACT");     
      indexstr=6;
      packetBuffer[indexstr++]=TipoDia + 1;
      packetBuffer[indexstr++]=hour + 1;  
      packetBuffer[indexstr++]=minute + 1;
      packetBuffer[indexstr++]=dayOfMonth + 1;
      packetBuffer[indexstr++]=month + 1;
      packetBuffer[indexstr++]=year + 1;       
      packetBuffer[indexstr]='\0';  
    }

    else if (strncmp(packetBuffer, "ENABLEHOR", 9)==0 ){      
         
       strcpy(packetBuffer, "ENHOR");       
       indexstr=5;
       p=EM_EN_TIMETABLE_OFFSET;
       for (c=0; c<N_EN_TIMETABLE; c++){
           packetBuffer[indexstr ++]=EepromRead(p++)+1;
       }
       packetBuffer[indexstr]='\0';
       
    }
    else if (strncmp(packetBuffer, "WHOR", 4)==0){
       indexdata=4;
       p=EM_EN_TIMETABLE_OFFSET;
       for (c = 0; c<N_EN_TIMETABLE; c++){
         EepromWrite(p++, packetBuffer[indexdata++]-1);
       }
       packetBuffer[0]=COMPLETED;
    }    
    else if (strncmp(packetBuffer, "CONENABLE", 9)==0){
              
       strcpy(packetBuffer, "ENCON"); 
       indexstr=5; 
       for (c = 0; c < 10; c++){
         if (Condicionados[c]==true){
           packetBuffer[indexstr]=2;
         }else{
           packetBuffer[indexstr]=1;
         }
         indexstr++;
       }
       packetBuffer[indexstr]='\0';     
    }    
    else if (strncmp(packetBuffer, "WCON", 4)==0){  
      indexdata=4; 
      for (c = 0; c<10; c++){
         if ( packetBuffer[indexdata]==2){
           Condicionados[c] = true;
         }else{
           Condicionados[c] = false;
         }
         indexdata++;
       }
       packetBuffer[0]=COMPLETED;
    }   
    else if (strncmp(packetBuffer, "COMANDO", 7)==0){
      EnviarRespuesta(RunCommand(packetBuffer[7]));
      packetBuffer[0]=EXTERNAL_REPLY;      
    }  
    else if (strncmp(packetBuffer, "TIMPERSIANA", 11)==0){
      strcpy(packetBuffer, "LECPE");      
      indexstr=5;       
      p=EM_UP_TIM_SHUTTER_OFFSET;  
      for (c=0; c < (N_UP_TIM_SHUTTER *2) ; c++){
        packetBuffer[indexstr++]=EepromRead(p++)+1;
      }   
      packetBuffer[indexstr]='\0';      
    }
    else if (strncmp(packetBuffer, "WCOW", 4)==0){
      data = packetBuffer[5]-1;
      indexdata=packetBuffer[4]-1;
      p = indexdata + EM_SETPOINTS_OFSSET;      
      EepromWrite(p, data);
      Consignas[indexdata]=data;      
      goto requestSP;          // OJO salto con Goto no mover.
    }
    else if (strncmp(packetBuffer, "SETPOINT", 8)==0){     
    requestSP:
       strcpy(packetBuffer, "SEPOI");
       indexstr=5;        
       for (c=0; c<10; c++){
         packetBuffer[indexstr++]= Consignas[c]+1;
       }
       packetBuffer[indexstr]='\0';    
    } 
    //Conrol persiana.    
    else if (strncmp(packetBuffer, "WPERS", 5)==0){
      p=EM_UP_TIM_SHUTTER_OFFSET;
      indexdata=5;
      for (byte  i = 0; i< (N_UP_TIM_SHUTTER *2) ;i++){
        EepromWrite(p++, packetBuffer[indexdata++]-1);
      }
      ReiniciarTiempoPersianas();
      packetBuffer[0]=COMPLETED;       
    }    
    else if (strncmp(packetBuffer, "RESTPER", 7)==0){
        ReiniciarPosicionPersiana(packetBuffer[7]-1);
        strcpy(packetBuffer,"RESETEANDO PERSIANA");        
    }
        
    else if (strncmp(packetBuffer, "HIST", 4)==0){
      #ifdef HISTORICAL_SD
       byte b=packetBuffer[4];
       
       char Ruta[] = {'H', 'I', 'T', '/', '0', '0', '-','0', '0', '-','0', '0', '.','C','S','V','\0'};
       String Val;
       Val = String(b);
       if (Val.length()==2){
         Ruta[4]=Val.charAt(0);
         Ruta[5]=Val.charAt(1);
       }
       else{
         Ruta[5]=Val.charAt(0);
       }
       b=packetBuffer[5];
       Val = String(b);
       if (Val.length()==2){
         Ruta[7]=Val.charAt(0);
         Ruta[8]=Val.charAt(1);
       }else{
         Ruta[8]=Val.charAt(0);
       }
       b=packetBuffer[6];
       Val = String(b);
       if (Val.length()==2){
         Ruta[10]=Val.charAt(0);
         Ruta[11]=Val.charAt(1);
       }else{
         Ruta[11]=Val.charAt(0);
       }
      
       String Line = ReadFile(packetBuffer[7]-1,Ruta)+"HT";
       char l[80];
       Line.toCharArray(l, 80);       
       EnviarRespuesta(l); 
       return; 
      #else
        strcpy(packetBuffer , "NOFOUND!!");
      #endif  
    }
    else {
      strcpy(packetBuffer,"REPETIRMSG");     //esto se tiene que estudiar no deberia responder ni guardar ip.
    }    
    if (packetBuffer[0]!= EXTERNAL_REPLY){
      
      if (packetBuffer[0]== COMPLETED){
       strcpy(packetBuffer,"COMPLETED");
     }
     EnviarRespuesta(packetBuffer);
    }  
  }
}


//MODIFICADO.
void EnvioEstadoActual()
{
  int LongCad;
  #ifdef THERMOSTAT_DS18B20_NUMBER
    LongCad=35+THERMOSTAT_DS18B20_NUMBER;
  #else  
    LongCad=35;
  #endif 
  
  char Respuesta[LongCad];
  byte  indexstr,c;
  
  strcpy(Respuesta, "EVAL"); 
  indexstr=4;
  
  for (c=0; c<30;c++)
  {
     if ((c)<Number_Circuit){
       Respuesta[indexstr]=circuits[c].Value+1;
     }
     else{
       Respuesta[indexstr]=1;
     }  
     indexstr++;
  }

  #ifdef THERMOSTAT_DS18B20_NUMBER
    int v;
    for (int t=0; t<THERMOSTAT_DS18B20_NUMBER;t++)
    {
      if ((Temperature[t] >=1)&&(Temperature[t]<=49)){
        v =(Temperature[t]*10)/2;
        Respuesta[indexstr++]=v+1;
      }
      else{
        Respuesta[indexstr++]=1;
      }
    }
  #endif 
  Respuesta[indexstr]='\0';
  EnviarRespuesta(Respuesta); 
}


//Modificado 2.3
void SelectScene(byte Dir)
{
  int p;
  byte c,v;
  
  p= (int) Dir;
  p= EM_ESCENES_OFFSET + ((p-1) * S_ESCENES );
  for (c =0 ; c<Number_Circuit; c++){
    v =EepromRead(p + c); 
    if (v < 250){
      circuits[c].Value = v;
    }
  }
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




void CargaHora()
{
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
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
        setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
      }
      if(month==10 && dayOfMonth >= 26 && dayOfWeek == 7 && hour==2){
        if (HoraRetrasa==false){
          HoraRetrasa=true;
          hour = 1;          
          setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
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


/******************************************************************/
// REFRESCAR
/*****************************************************************/
/******************************************************************/
// REFRESCAR
/*****************************************************************/
boolean Notification(String Text){
  if (Mail==""){return true;}
  if (Connecting){return false;}
  #ifdef DEBUG_MODE   
    Serial.println("Notification = "+ Text);               
  #endif
  Text.replace(" ", "%20%20");
  boolean result =CreateCabHTTP("GET http://excontrol.es/Users/Noti.aspx?Mail=",Text);
  if (result){result=ComproRespuestaHTTP();}
  else{
    client.stop();
    client.flush();
    Connecting=false;
    #ifdef DEBUG_MODE   
      Serial.println("Server no aviable");               
    #endif
  }
  return result;
}
void connectAndRfr(){
  EspRfrIp=50;
  if (Connecting){return;}   
  if (Mail==""){return;}
  
  boolean result = CreateCabHTTP("GET http://excontrol.es/Users/IpSet.aspx?Mail=","");
  if (result){ComproRespuestaHTTP(); }
}
boolean CreateCabHTTP(String URL, String Key2){
  #ifdef DEBUG_MODE   
    Serial.println("Coneccting http server");               
  #endif
  if (client.connect("www.excontrol.es", 80)) {
    client.print(URL);
    if (Key2==""){client.print(Mail + "&Key=" + Key);}
    else{client.print(Mail + "&Key=" + Key + "&Key2=" + Key2);}
    client.println(" HTTP/1.0");
    client.println();
    Connecting=true;
    #ifdef DEBUG_MODE   
      Serial.println("Coneccting completed");               
    #endif
    return true;
  }
  Connecting=false;
  return false;
}
boolean ComproRespuestaHTTP(){
  int Reintento=0;
  while(true){
    if (client.available()) {
      int c;
      for (c=0;c<5;c++){char c = client.read();}
      client.stop();
      client.flush();
      EspRfrIp=50;
      Connecting=false;
      #ifdef DEBUG_MODE   
          Serial.println("TCP Connection Complete");               
       #endif
      return true;
    }
    else{
      SystemLoop();
      delay(10);
      if (Reintento >= 700 ){
        #ifdef DEBUG_MODE   
          Serial.println("Time Out TCP");               
        #endif
        client.stop();
        client.flush();
        Connecting=false;
        return false;
      }
    }
    Reintento++;
  }
}


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

void loop(){
  
  if((TimNow - TimSg) >= 1000) {
    LoopNewSecond();
    TimSg=TimNow;
    if (Connecting==false){
      for (int a=0;a<=19;a++){
        if (Alarms[a]==1){
          boolean res=Notification(GetAlarmsName(a));
          if (res){Alarms[a]=2;}
          break;
        }  
      }
    }
  }
  SystemLoop();
}

void SystemLoop()
{
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
  
  TimNow=millis();     
   if(TimNow < TimSg ) {TimSg=TimNow;}
   if(TimNow < Tim30Sg ) {CargaHora();}
   if((TimNow - Tim30Sg) >= 30000) {

    #ifdef THERMOSTAT_DS18B20_NUMBER
      RefreshTemperature();
    #endif
    
    #ifdef HISTORICAL_SD
      if (SdOk){
        SecutityCopy();
      }
    #endif 
     
    CargaHora();
    Loop30Sg();
    if (EspRfrIp<1){
      connectAndRfr();
    }else{
      EspRfrIp--;
    }
  }
  #ifdef WIFI_SHIELD
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
   #else
    RecepcionPaqueteUDP();
   #endif

   InputState();
   CheckSwicth();
   
   //Control de movimiento persianas
   for (int p =0; p<NumeroPersianas;p++){
     GestionMovPersianas(p);
   } 
   
   #ifdef RETROAVISOS 
    ComprobarRetroavisos(); 
   #endif   
   
   GestionCircuitos();
   OutControl();
   UserLoop();

}
