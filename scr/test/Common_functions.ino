
void setup(){
  
  #ifdef DEBUG_MODE   
    Serial.begin(9600);      
    Serial.println("Sytem Start");   
  #endif
  boolean CargaSdOk=false;
  #ifdef SD_CARD
    pinMode(SS_ETHERNET, OUTPUT);//pines de control spi
    pinMode(SS_SD, OUTPUT);//pines de control spi
    pinMode(SS_UNO,  OUTPUT);
  #endif  
  byte ControlPersiana=0;
  byte ControlTermostato=0;
  byte ControlRetroaviso=0;
 
  //enum CircuitsTypes {Persiana,ConsignaTemp,};
  for (int c=0;c<Number_Circuit;c++){
    
    circuits[c].Device_Number=0;
    circuits[c].Out1_Value=Off;
    circuits[c].Out2_Value=Off;
    circuits[c].Value=0;
    circuits[c].CopyRef=0;
    circuits[c].Type=TipeCir[c];
    circuits[c].OldValue=99;
    //if (circuits[c].Type==Persiana){NumeroPersianas++;}
    
    if ((circuits[c].Type==Persiana)||(circuits[c].Type==Toldo)){LocalizadorPersiana[ControlPersiana]=c;circuits[c].Device_Number=ControlPersiana;ControlPersiana++;}

    #ifdef THERMOSTAT_DS18B20_NUMBER
      if (circuits[c].Type==ConsignaTemp){circuits[c].Device_Number=ControlTermostato;ControlTermostato++;}
    #endif
    #ifdef RETROAVISOS 
      if (circuits[c].Type==Ado_Retroaviso){circuits[c].Device_Number=ControlRetroaviso;ControlRetroaviso++;}
    #endif
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
    LastTimeInput[i]=currenMillis;InState[i]=0;
  }
  unsigned int reading;
  for (int i=0; i<Number_SwicthInput;i++){pinMode(
    PinSwicthInput[i], INPUT);
    #ifdef INTERNAL_RESISTOR
     digitalWrite(PinSwicthInput[i], HIGH);       // turn on pullup resistors
    #endif
    reading = digitalRead(PinSwicthInput[i]);LastTimeSwicthInput[i]=millis();SwicthState[i]=reading;
  }
  int i;
  for (i=0; i<Number_Output;i++){pinMode(PinOutput[i], OUTPUT);digitalWrite(PinOutput[i],Off);}
  for (i=0; i < N_SETPOINTS; i++) {Consignas[i]=EepromRead(EM_SETPOINTS_OFSSET + i); }
  for (i=0; i < N_ALARMS; i++){}{Alarms[i]=EepromRead(EM_ALARMS_OFSSET+i);if (Alarms[i]>=5){Alarms[i]=0;}}
  
  //Fijamo valores y posicion inicio persianas
  //Fijamos el tiempo de subida bajada Persianas
  for (int per=0; per<NumeroPersianas; per++){InDowPersiana[per]=false;InUpPersiana[per]=false;}
  ReiniciarTiempoPersianas();
   
  
 
  #ifdef SD_CARD
    EnableSD();    
    #ifdef DEBUG_MODE   
      Serial.println("Initializing SD card...");             
    #endif  
    if (!SD.begin(SS_SD)) {
      #ifdef DEBUG_MODE   
        Serial.println("ERROR - SD card initialization failed!");           
      #endif        
        SdOk=false;
    }else{
       #ifdef DEBUG_MODE   
        Serial.println("SUCCESS - SD card initialized.");              
     #endif
      if (!SD.exists("HIT/")){SD.mkdir("HIT");} 
    }
    
    if (SdOk){
      
        //Log File
      SdFile = SD.open("log.txt", FILE_WRITE);
      String Log= "Start at " + ((String)hour) + ":" +((String)minute) + " - " +((String)dayOfMonth) + "/" + ((String)month) + "/"+((String)year) + "/";
      SdFile.println(Log);
      SdFile.close();
        
  
        if (SD.exists("elc.txt")) {
            #ifdef DEBUG_MODE   
              Serial.println("Load circuit state");           
            #endif  
            SdFile = SD.open("Elc.txt");
            
            if (SdFile) {
            int c=0; 
            while ((SdFile.available())&&(c<Number_Circuit)) {circuits[c].Value=SdFile.read();c++;}SdFile.close();CargaSdOk=true;}
            
          }
        else{ 
          
          #ifdef DEBUG_MODE   
            Serial.println("ERROR - Found file Elc.");
          #endif
        }
     }
     EnableEthernet();  
    
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
  #else
    //Ethernet.begin(mac);//Borrar esta linea vale la de abajo
     //W5100.setRetransmissionTime(0x07D0);  //setRetransmissionTime sets the Wiznet's timeout period, where each unit is 100us, so 0x07D0 (decimal 2000) means 200ms.
     //W5100.setRetransmissionCount(3);      //setRetransmissionCount sets the Wiznet's retry count.
     Ethernet.begin(mac,ip);
     Udp.begin(localPort);
  #endif
   

   if(CargaSdOk){
     //las persianas ajustar posicion
     for (int per=0; per<NumeroPersianas; per++){CargaPosicionPersiana(per); }   
   }
   else{for (int per=0; per<NumeroPersianas; per++){ReiniciarPosicionPersiana(per);}}

//Inicio control Horarios
  Wire.begin();
  CargaHora();

 #ifdef RECEIVER_433  
    Init433Mhz();// Receiver on inerrupt 0 => that is pin #2
 #endif
 #if defined (TRANSMITER_433)  
   mySwitch.enableTransmit(11);
 #endif
 

 //Iniciamos Termostatos
 #ifdef THERMOSTAT_DS18B20_NUMBER
   InitDS18B20();
 #endif 
  #ifdef IR_RECIVE
   irrecv.enableIRIn();
 #endif
 //Iniciamos perro guardian
 #ifdef ENABLE_WATCH_DOG
   wdt_enable(WDTO_8S );
 #endif 
 
  UserSetup();
}
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
void SystemLoop(){
  #ifdef RETROAVISOS 
    for (int c=0;c<Number_Circuit;c++){ if (circuits[c].Type==Ado_Retroaviso){circuits[c].OldValue=circuits[c].Value;}}
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
     #ifdef SD_CARD
       if (SdOk){SecutityCopy();}
     #endif     
     CargaHora();Loop30Sg();
     if (EspRfrIp<1){connectAndRfr();}else{EspRfrIp--;}
   }
   #ifdef WIFI_SHIELD
    if (WiFi.status() != WL_CONNECTED) {
      unsigned int TimActual=millis();
      if (TimActual<=TimConexion){TimConexion=millis();}
    else{
      if ((TimActual-TimConexion)>=60000){ConexionWifi();}
    }
    }else{TimConexion=millis();RecepcionPaqueteUDP();}
   #else
    RecepcionPaqueteUDP();
   #endif

   InputState();
   CheckSwicth();
   for (int p =0; p<NumeroPersianas;p++){GestionMovPersianas(p);} //Control de movimiento persianas
   
   #ifdef RETROAVISOS 
    ComprobarRetroavisos(); 
   #endif
   
   #ifdef ENABLE_WATCH_DOG
    wdt_reset();
   #endif 
   GestionCircuitos();
   OutControl();
   UserLoop();

}

#ifdef RETROAVISOS 
void ComprobarRetroavisos(){
   
  int reading;
  for (int i=0;i<Number_Circuit;i++){ 
    if (circuits[i].Type==Ado_Retroaviso){
      reading = digitalRead(PinEstadoCircuito[circuits[i].Device_Number]);
      unsigned long InputMillis = millis();
      if (reading==circuits[i].Out2_Value){LastTimeEstadoRetroaviso[i]=InputMillis;}
      else{
      if(LastTimeEstadoRetroaviso[i]>InputMillis){LastTimeEstadoRetroaviso[i]=InputMillis;}
      if ((InputMillis-LastTimeEstadoRetroaviso[i])>=60){
        circuits[i].Out2_Value=reading;
        
        if (reading==HIGH){
          circuits[i].OldValue=1;
          circuits[i].Value=1;
        }
        else
        {
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
  for (int i=0; i<Number_SwicthInput;i++){
    reading = digitalRead(PinSwicthInput[i]);
    unsigned long InputMillis = millis();
    if (reading==SwicthState[i]){LastTimeSwicthInput[i]=InputMillis;}
    else{
      if(LastTimeSwicthInput[i]>InputMillis){LastTimeSwicthInput[i]=InputMillis;}
      if ((InputMillis-LastTimeSwicthInput[i])>=60){SwicthState[i]=reading;SwicthStateChange(i,reading);}    
     }
  }
}
void GestionCircuitos(){
      
  //  ,,,,,,,Persiana,ConsignaTemp,,
  for (int c=0;c<Number_Circuit;c++){
    
    if (circuits[c].Type!=Reserva){
      if ((circuits[c].Type==Ado_Digital)||(circuits[c].Type==Puerta)||(circuits[c].Type==Enchufe)||(circuits[c].Type==Riego)||(circuits[c].Type==Riego_Temporizado)||(circuits[c].Type==Frio)||(circuits[c].Type==Calor)||(circuits[c].Type==Valvula)||(circuits[c].Type==Radiante)||(circuits[c].Type==Ventilador)||(circuits[c].Type==Piloto)) {
        if (circuits[c].Value>=1){circuits[c].Out1_Value=On;circuits[c].Out2_Value=Off;}
        else{circuits[c].Out1_Value=Off;circuits[c].Out2_Value=Off;}
      }
      if (circuits[c].Type==Ado_3Etapas){
        switch (circuits[c].Value) {
        case 0:    
          circuits[c].Out1_Value=Off;circuits[c].Out2_Value=Off;
          break;
        case 1:    
          circuits[c].Out1_Value=On;circuits[c].Out2_Value=Off;
          break;
        case 2:    
         circuits[c].Out1_Value=Off;circuits[c].Out2_Value=On; 
          break;
         case 3:    
           circuits[c].Out1_Value=On;circuits[c].Out2_Value=On; 
          break;
         default:    
          circuits[c].Value=0;circuits[c].Out1_Value=Off;circuits[c].Out2_Value=Off;
          break;
         } 
        }
      }
      if ((circuits[c].Type==Persiana)||(circuits[c].Type==Toldo)){//PersianaS
       circuits[c].Out1_Value=Off;circuits[c].Out2_Value=Off;
       if ((OutDowPersiana[circuits[c].Device_Number]==true)||(OutUpPersiana[circuits[c].Device_Number]==true))
       {
         if ((OutDowPersiana[circuits[c].Device_Number]==true)&&(OutUpPersiana[circuits[c].Device_Number]==false)){circuits[c].Out1_Value=On; circuits[c].Out2_Value=On;}
         if ((OutDowPersiana[circuits[c].Device_Number]==false)&&(OutUpPersiana[circuits[c].Device_Number]==true)){circuits[c].Out1_Value=On; circuits[c].Out2_Value=Off;}     
       }
      }
      //Gestion Enchufes Radio Frecuencia

  
  
     //Gestion retroaviso
     #ifdef RETROAVISOS   
        if (circuits[c].Type==Ado_Retroaviso){
          if (circuits[c].OldValue!=circuits[c].Value){if(circuits[c].Out1_Value==Off){circuits[c].Out1_Value=On;}else{circuits[c].Out1_Value=Off;}}       
        }
      #endif    
   }
}
void InputState(){
  for (int i=0; i<Number_Input;i++){
     unsigned long InputMillis = millis()-LastTimeInput[i];
     if ((InState[i]>=4)||(-1 >= InState[i])){InState[i]=0;}
     if (digitalRead(PinInput[i]) == LOW ) {
       if ((InState[i]==0)&&(InputMillis>=60)){LastTimeInput[i]=millis();InState[i]=1;}
       if ((InState[i]==1)&&(InputMillis>=440)){LongInput(i);InState[i]=2;}
       if (InState[i]==2){LastTimeInput[i]=millis();}
       if (InState[i]==3){LastTimeInput[i]=millis();InState[i]=1;}
     }
     else{

       if (InState[i]==0){LastTimeInput[i]=millis();}
       if (InState[i]==1){LastTimeInput[i]=millis();InState[i]=3;}
       if ((InState[i]==2) &&(InputMillis>=60)){LastTimeInput[i]=millis();InState[i]=0;LongInputEnd(i);}
       if ((InState[i]==3)&&(InputMillis>=60)){LastTimeInput[i]=millis();InState[i]=0;ShortInput(i);}
     }
  }
} 
/*************************************************************/
   //Gestion Persianas
/*************************************************************/ 

void GestionMovPersianas(int NPersiana){
  if (InUpPersiana[NPersiana] || InDowPersiana[NPersiana])
  {  //Funcionamiento Manual
    if (InUpPersiana[NPersiana] && InDowPersiana[NPersiana]){OutDowPersiana[NPersiana]=false;OutUpPersiana[NPersiana]=false;}
    else{
      if (InUpPersiana[NPersiana]){SubirPersiana(NPersiana);} 
      if (InDowPersiana[NPersiana]){BajarPersiana(NPersiana);}
      circuits[LocalizadorPersiana[NPersiana]].Value=PosicionPersiana[NPersiana];    
    }    
  }
  else
  {  //Funcionamiento Automatico;
    if (circuits[LocalizadorPersiana[NPersiana]].Value==PosicionPersiana[NPersiana]){OutDowPersiana[NPersiana]=false;OutUpPersiana[NPersiana]=false;}
    else
    {      
       if (circuits[LocalizadorPersiana[NPersiana]].Value > PosicionPersiana[NPersiana]){SubirPersiana(NPersiana);}
       else  {if (circuits[LocalizadorPersiana[NPersiana]].Value< PosicionPersiana[NPersiana])  {BajarPersiana(NPersiana);}}    
    }  
  }
}

void SubirPersiana(int NPersiana){
  if (OutDowPersiana[NPersiana]==true){BajarPersiana(NPersiana);OutDowPersiana[NPersiana]=false;OutControl();delay(200);}
  long TiempoActual = micros();
  if (OutUpPersiana[NPersiana]==false){OutUpPersiana[NPersiana]=true;}
  else{
    long DiferenciaTiempo;
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

   
void BajarPersiana(int NPersiana){
  if (OutUpPersiana[NPersiana]==true){SubirPersiana(NPersiana);OutUpPersiana[NPersiana]=false;OutControl();delay(200);}
  long TiempoActual = micros();
  if (OutDowPersiana[NPersiana]==false){OutDowPersiana[NPersiana]=true;}
  else{
    long DiferenciaTiempo;
    if (TiempoActual<TiempoMovPersiana[NPersiana]){DiferenciaTiempo=TiempoActual;}else{DiferenciaTiempo = TiempoActual-TiempoMovPersiana[NPersiana];}
    if ((TiempoPosPersianaDown[NPersiana] + DiferenciaTiempo)<TimDowPersiana[NPersiana]){TiempoPosPersianaDown[NPersiana]=TiempoPosPersianaDown[NPersiana] + DiferenciaTiempo;}else{TiempoPosPersianaDown[NPersiana]=TimDowPersiana[NPersiana];}
   
    byte porcentajeBajada = TiempoPosPersianaDown[NPersiana] / (TimDowPersiana[NPersiana]/100);
    byte porcentajeSubida=100-porcentajeBajada;
    PosicionPersiana[NPersiana]=porcentajeSubida;
    TiempoPosPersianaUp[NPersiana]=porcentajeSubida*(TimUpPersiana[NPersiana]/100);
  }  
  TiempoMovPersiana[NPersiana]=TiempoActual;
}

void ReiniciarPosicionPersiana(int NumPersiana){ TiempoPosPersianaUp[NumPersiana]=0;TiempoPosPersianaDown[NumPersiana]=TimDowPersiana[NumPersiana];circuits[LocalizadorPersiana[NumPersiana]].Value=100;}
void ReiniciarTiempoPersianas()
{
  for ( byte c =0; c < NumeroPersianas; c++){
    TimUpPersiana[c]=(EepromRead(EM_UP_TIM_SHUTTER_OFFSET + c))*  1000000; 
    TimDowPersiana[c]=(EepromRead(EM_DO_TIM_SHUTTER_OFFSET + c))* 1000000;
  }
}
void CargaPosicionPersiana(int NPersiana){
  PosicionPersiana[NPersiana]=circuits[LocalizadorPersiana[NPersiana]].Value;
  byte porcentajeBajada=100-PosicionPersiana[NPersiana];
  TiempoPosPersianaDown[NPersiana]=porcentajeBajada*(TimDowPersiana[NPersiana]/100);
  TiempoPosPersianaUp[NPersiana]=PosicionPersiana[NPersiana]*(TimUpPersiana[NPersiana]/100);
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
      #ifdef SD_CARD
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
void EnviarRespuesta(char  *ReplyBuffer)
{
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
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
/*
void RecepcionPaqueteUDP()
{
  if (Connecting==true){return;}//hay que probarlo con y sin!
  int packetSize = Udp.parsePacket();
  
  if(packetSize)
  { 
    #ifdef DEBUG_MODE        
      Serial.println("UDP Packet recive");   
    #endif
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    
    int LongCadena;
    if  (SecureConnection){
      for(int c=0; c<8;c++){if (Key[7-c]!=packetBuffer[packetSize-(c+1)]){return;}}
      LongCadena=packetSize-7;}
    else{LongCadena=packetSize + 1;}

    char  CadenaEntrada[LongCadena];
    for (int c=0;c<LongCadena;c++){CadenaEntrada[c]=packetBuffer[c];}
    CadenaEntrada[LongCadena-1]='\0';
    String CadenaIn = (String)CadenaEntrada;
    if (CadenaIn=="ALRM"){
      char Respuesta[25];
      Respuesta[0]='E';Respuesta[1]='S';Respuesta[2]='A';Respuesta[3]='L';
      for (byte  i = 4; i<24;i++){Respuesta[i]=Alarms[i-4]+1;}
      Respuesta[24]='\0';
      EnviarRespuesta(Respuesta);
      return;
    }
    if (CadenaIn.indexOf("SETNOTI")>-1){
       byte pos=packetBuffer[7]-1;
       Alarms[pos]=packetBuffer[8]-1;EnviarRespuesta("WIALMO");  
       EepromWrite(3880+pos,Alarms[pos]);
       return;    
    }
    if (CadenaIn.indexOf("HIST")>-1){
      #ifdef SD_CARD
       char Ruta[] = {'H', 'I', 'T', '/', '0', '0', '-','0', '0', '-','0', '0', '.','C','S','V','\0'};
       String Val;byte b=packetBuffer[4];Val = String(b);
       if (Val.length()==2){Ruta[4]=Val.charAt(0);Ruta[5]=Val.charAt(1);}else{Ruta[5]=Val.charAt(0);}
       b=packetBuffer[5];Val = String(b);
       if (Val.length()==2){Ruta[7]=Val.charAt(0);Ruta[8]=Val.charAt(1);}else{Ruta[8]=Val.charAt(0);}
       b=packetBuffer[6];Val = String(b);
       if (Val.length()==2){Ruta[10]=Val.charAt(0);Ruta[11]=Val.charAt(1);}else{Ruta[11]=Val.charAt(0);}
      
       String Line = ReadFile(packetBuffer[7]-1,Ruta)+"HT";
       char l[80];
       Line.toCharArray(l, 80);
       EnviarRespuesta(l); return; 
      #else
       String Line = "NOFOUND!!";
       char l[90];
       Line.toCharArray(l, 80);
       EnviarRespuesta(l); return;
      #endif  
    }  
     //cONTROL TRAMAS HORARIOS.
    //DEBE CONTESTAR AL SISTEMA TRADICIOAL Y UTILIZAR OTRO CON PAQUETES PARTIDOS
     if (CadenaIn.indexOf("READHOR")>-1){//Primer paquete de horarios
       char Respuesta[85];
       Respuesta[0]='E';
       Respuesta[1]='H';
       Respuesta[2]='R';
       Respuesta[3]=1;
       int Reg;
       int Pos=4;
       Reg=packetBuffer[7]*320+1000;
       for (int i = Reg; i<(Reg+80);i++){Respuesta[Pos]=EepromRead(i)+1;Pos++;}
       Respuesta[84]='\0';
       EnviarRespuesta(Respuesta);   
       return;    
    }
  if (CadenaIn.indexOf("HOREAD")>-1){
       char Respuesta[85];
       Respuesta[0]='E';
       Respuesta[1]='H';
       Respuesta[2]='R';
       Respuesta[3]=packetBuffer[6];
       int Reg;
       int Pos=4;
       Reg=packetBuffer[7]*320+1000;
       Reg+=(packetBuffer[6]-1)*80;
       Serial.print("Registro ");Serial.println(Reg);
       for (int i = Reg; i<(Reg+80);i++){Respuesta[Pos]=EepromRead(i)+1;Pos++;}
       Respuesta[84]='\0';
       EnviarRespuesta(Respuesta);   
       return;    
    }
  
    if (CadenaIn.indexOf("HORWRI")>-1){//Escritura Horario 
       int Reg;
       int Pos=8;
       Reg=packetBuffer[7]*320+1000;
       Reg+=(packetBuffer[6]-1)*80;
       for (int i = Reg; i<(Reg+80);i++){EepromWrite(i,packetBuffer[Pos]);Pos++;}
       char Respuesta[36];
       Respuesta[0]='H';
       Respuesta[1]='W';
       Respuesta[2]='R';
       Respuesta[3]='T';
       Respuesta[4]=packetBuffer[6];
       Respuesta[5]='\0';
       EnviarRespuesta(Respuesta);  
       return;    
    }
    if (CadenaIn.indexOf("CARG")>-1){
      
        char Respuesta[35];
        Respuesta[0]='V';
        Respuesta[1]='A';
        Respuesta[2]='L';
        Respuesta[3]='C';
        for (byte  i = 4; i<34;i++){Respuesta[i]= 1;} 
        
        for (byte  i = 0; i<Number_Circuit;i++){
          if (circuits[i].Type==Reserva){Respuesta[i+4]= 0;}
          else if (circuits[i].Type==Ado_Digital){Respuesta[i+4]= 1;}
          else if (circuits[i].Type==Ado_Retroaviso){Respuesta[i+4]= 1;}
          else if (circuits[i].Type==Ado_3Etapas){Respuesta[i+4]= 2;}
          else if (circuits[i].Type==Enchufe){Respuesta[i+4]= 7;}
          else if (circuits[i].Type==EnchufeRF){Respuesta[i+4]= 8;}
          else if (circuits[i].Type==Riego){Respuesta[i+4]= 13;}
          else if (circuits[i].Type==Riego_Temporizado){Respuesta[i+4]= 14;}
          else if (circuits[i].Type==Valvula){Respuesta[i+4]= 15;}
          else if (circuits[i].Type==ConsignaTemp){Respuesta[i+4]= 29;}
          else if (circuits[i].Type==Frio){Respuesta[i+4]= 19;}
          else if (circuits[i].Type==Calor){Respuesta[i+4]= 24;}
          else if (circuits[i].Type==Radiante){Respuesta[i+4]= 25;}
          else if (circuits[i].Type==Persiana){Respuesta[i+4]= 34;}
          else if (circuits[i].Type==Toldo){Respuesta[i+4]= 35;}
          else if (circuits[i].Type==Puerta){Respuesta[i+4]= 39;}
          else if (circuits[i].Type==Ventilador){Respuesta[i+4]= 43;}
          else if (circuits[i].Type==Piloto){Respuesta[i+4]= 51;}
          else {Respuesta[i+4]= 0;}
          Respuesta[i+4]++;
        }
        Respuesta[34]='\0';
        EnviarRespuesta(Respuesta); 
        return;
    }
   
    ///////////////////////////
    //FINAL CONTROL HORARIOS
    //////////////////////////////////
    if (CadenaIn.indexOf("COMCOMM")>-1){CommonOrders(packetBuffer[7]);EnviarRespuesta("COMCOMOK");return;}
    if (CadenaIn=="CLEARHORARIO"){ 
       #ifdef ENABLE_WATCH_DOG
          wdt_reset();
       #endif 
      for (int i = 950; i <= 3879; i++){EepromWrite(i, 66);
        #ifdef ENABLE_WATCH_DOG
          if (i==2500){wdt_reset();}
       #endif 
      }
      EnviarRespuesta("HORARIOS BORRADOS"); return;}  
    if (CadenaIn=="CLEARESPCDAY"){for (int i = 3900; i <= 3999; i++){EepromWrite(i, 0);}EnviarRespuesta("DIAS ESPECIALES BORRADOS");return; }
    if (CadenaIn.indexOf("SETFH")>-1){
      setDateDs1307(packetBuffer[5] ,packetBuffer[6], packetBuffer[7], packetBuffer[8], packetBuffer[9], packetBuffer[10], packetBuffer[11]);
      CargaHora();
      EnviarRespuesta("SETFHOK");
      return;
    }
     if (CadenaIn.indexOf("GETSENSOR")>-1)
    {  
      char* Resultado;
      int LongCad;
      Resultado = ReadSensor(packetBuffer[9]);
      LongCad = strlen(Resultado); 
      char Respuesta[LongCad  + 8];
        Respuesta[0]='S';Respuesta[1]='E';Respuesta[2]='N';Respuesta[3]='S';Respuesta[4]='O';Respuesta[5]='R';Respuesta[6]=packetBuffer[9];        
        int p;
        for (p=0;p<LongCad;p++){ Respuesta[p+7]=Resultado[p];}
        Respuesta[LongCad  + 7]='\0';
        EnviarRespuesta(Respuesta);
        return;
    }   
    if (CadenaIn.indexOf("READDAY")>-1){
        char Respuesta[55];
        Respuesta[0]='C';
        Respuesta[1]='F';
        Respuesta[2]='D';
        Respuesta[3]='A';
        
        int PunteroRegistro;
        if (packetBuffer[7]=='2'){PunteroRegistro=3950;}else{PunteroRegistro=3900;}
        for (int  i = 0; i<50;i++){Respuesta[i+4]=EepromRead(PunteroRegistro + i); } 
        Respuesta[324]='\0';
        EnviarRespuesta(Respuesta);
        return;
    }
    if (CadenaIn.indexOf("WRIDAYE")>-1){
       int Reg;
       int Pos=8;
       if (packetBuffer[7]==2){Reg=3950;}else{Reg=3900;}
       for (int i = Reg; i<(Reg+50);i++){EepromWrite(i,packetBuffer[Pos]);Pos++;}
       EnviarRespuesta("COMPLETED");  
       return;    
    }
    
    if (CadenaIn.indexOf("RETRIGGER")>-1){
       char Respuesta[53];
       Respuesta[0]='T';
       Respuesta[1]='I';
       Respuesta[2]='G';
       Respuesta[3]='R';
       int Pos=4;
       for (int i = 950; i<998;i++){Respuesta[Pos]=EepromRead(i)+1;Pos++;}
       Respuesta[52]='\0';
       EnviarRespuesta(Respuesta);   
       return;    
    }
      if (CadenaIn.indexOf("WTGR")>-1){
       int Pos=4;
       for (int i = 950; i<998;i++){EepromWrite(i,packetBuffer[Pos]);Pos++;}
       EnviarRespuesta("COMPLETED");  
       return;    
    }   
    
    
   
    if (CadenaIn.indexOf("SVAL")>-1){circuits[packetBuffer[4]-1].Value=packetBuffer[5]-1;EnvioEstadoActual();return;} 

    if (CadenaIn=="VACT"){
      int LongCad;
      #ifdef THERMOSTAT_DS18B20_NUMBER
        LongCad=35+THERMOSTAT_DS18B20_NUMBER;
      #else  
        LongCad=35;
      #endif 
      char Respuesta[LongCad];
      Respuesta[0]='V';Respuesta[1]='V';Respuesta[2]='A';Respuesta[3]='L';
      for (byte  i = 4; i<34;i++)
      {
         if ((i-4)<Number_Circuit){
         Respuesta[i]=circuits[i-4].Value+1;
         }else{Respuesta[i]=1;}
      }
      #ifdef THERMOSTAT_DS18B20_NUMBER
        int v;
        for (int t=0; t<THERMOSTAT_DS18B20_NUMBER;t++){
          if ((Temperature[t] >=1)&&(Temperature[t]<=49)){v =(Temperature[t]*10)/2; Respuesta[34+t]=v+1;}else{Respuesta[34+t]=1;}
        }
      #endif 
      Respuesta[LongCad-1]='\0';
      EnviarRespuesta(Respuesta);
      return;}  
    
    if (CadenaIn.indexOf("SSCE")>-1)
    {  
      int Dir;
      Dir = (int)packetBuffer[4];
      SelectScene(Dir);
      EnvioEstadoActual();
      return;
    }
    if (CadenaIn.indexOf("WESC")>-1)
    {
      int Dir;
      Dir = (int)packetBuffer[4];
      Dir = (Dir-1) * 30;
       for (byte  i = 0; i<30;i++){EepromWrite(i+Dir, packetBuffer[5+i]-1);}
       EnviarRespuesta("COMPLETED"); 
       return;
    } 
    if (CadenaIn == "ESTADOINST"){ReadDate();return;}
    if (CadenaIn.indexOf("RESC")>-1)
    {
       int Dir;
       Dir = (int)packetBuffer[4];
       Dir = (Dir-1) * 30;
          
       char Respuesta[36];
       Respuesta[0]='V';
       Respuesta[1]='E';
       Respuesta[2]='S';
       Respuesta[3]='C';
       Respuesta[4]=packetBuffer[4];       
         
       for (int i = 0; i < 30;i++)
       {
         byte V=EepromRead(Dir+i);
         if (V<=254){V++;}
         Respuesta[i + 5 ]=V;
       }
       Respuesta[35]='\0';
       EnviarRespuesta(Respuesta);  
       return; 
    }
    
    if (CadenaIn == "ENABLEHOR")
    {
      char Respuesta[56];
       Respuesta[0]='E';
       Respuesta[1]='N';
       Respuesta[2]='H';
       Respuesta[3]='O';
       Respuesta[4]='R';         
         
       for (int i = 0; i < 50;i++){Respuesta[i + 5 ]=EepromRead(400+i)+1;}
       Respuesta[55]='\0';
       EnviarRespuesta(Respuesta);  
       return; 
    }
    if (CadenaIn.indexOf("WHOR")>-1)
    {
       for (byte  i = 0; i<50;i++){EepromWrite(i+400, packetBuffer[4+i]-1);}
       EnviarRespuesta("COMPLETED"); 
       return;
    } 
    
     if (CadenaIn == "CONENABLE")
    {
      char Respuesta[16];
       Respuesta[0]='E';
       Respuesta[1]='N';
       Respuesta[2]='C';
       Respuesta[3]='O';
       Respuesta[4]='N';         
         
       for (int i = 0; i < 10;i++){if (Condicionados[i]==true){Respuesta[i + 5 ]=2;}else{Respuesta[i + 5 ]=1;}}
       Respuesta[15]='\0';
       EnviarRespuesta(Respuesta);  
       return; 
    }
    if (CadenaIn.indexOf("WCON")>-1)
    {  
       for (byte  i = 0; i<10;i++){if ( packetBuffer[4+i]==2){Condicionados[i] = true;}else{Condicionados[i] = false;}}
       EnviarRespuesta("COMPLETED"); 
       return;
    } 
    
    if (CadenaIn.indexOf("COMANDO")>-1){EnviarRespuesta(RunCommand(packetBuffer[7]));return;}
  
    if (CadenaIn == "TIMPERSIANA")
    {
      char Respuesta[66];
       Respuesta[0]='L';
       Respuesta[1]='E';
       Respuesta[2]='C';
       Respuesta[3]='P';
       Respuesta[4]='E';         
         
       for (int i = 0; i < 60;i++){ Respuesta[i + 5 ]=EepromRead(480+i)+1;}
       Respuesta[65]='\0';
       EnviarRespuesta(Respuesta);  
       return; 
    
    }
     if (CadenaIn == "SETPOINT")
    {
      char Respuesta[16];
       Respuesta[0]='S';
       Respuesta[1]='E';
       Respuesta[2]='P';
       Respuesta[3]='O';
       Respuesta[4]='I';  
       for (int i = 0; i < 10;i++){Respuesta[i + 5 ]= Consignas[i]+1;}
       Respuesta[15]='\0';
       EnviarRespuesta(Respuesta);  
       return; 
    
    }
    
    
     if (CadenaIn.indexOf("WCOW")>-1)
    {
      char Respuesta[16];
      byte v = packetBuffer[5]-1;
      byte p = packetBuffer[4]-1;
      
      EepromWrite(p + 940, v);      
      Consignas[p]=v;
       
      Respuesta[0]='S';
      Respuesta[1]='E';
      Respuesta[2]='P';
      Respuesta[3]='O';
      Respuesta[4]='I';  
      for (int i = 0; i < 10;i++){Respuesta[i + 5 ]= Consignas[i]+1;}
      Respuesta[15]='\0';
      EnviarRespuesta(Respuesta);  
      return; 
    } 
    
     if (CadenaIn.indexOf("WPERS")>-1)
    {
       for (byte  i = 0; i<60;i++){EepromWrite(i+480, packetBuffer[5+i]-1);}
       ReiniciarTiempoPersianas();
       EnviarRespuesta("COMPLETED"); 
       return;
    } 
     if (CadenaIn.indexOf("RESTPER")>-1){
        ReiniciarTiempoPersianas();
        ReiniciarPosicionPersiana(packetBuffer[7]-1);
        EnviarRespuesta("RESETEANDO PERSIANA");
        return;
    }

    EnviarRespuesta("REPETIRMSG"); 
  }
}
void EnvioEstadoActual()
{
  int LongCad;
  #ifdef THERMOSTAT_DS18B20_NUMBER
    LongCad=35+THERMOSTAT_DS18B20_NUMBER;
  #else  
    LongCad=35;
  #endif 
  char Respuesta[LongCad];
  Respuesta[0]='E';Respuesta[1]='V';Respuesta[2]='A';Respuesta[3]='L';
  for (byte  i = 4; i<34;i++)
  {
     if ((i-4)<Number_Circuit){
     Respuesta[i]=circuits[i-4].Value+1;
     }else{Respuesta[i]=1;}
  }
  #ifdef THERMOSTAT_DS18B20_NUMBER
    int v;
    for (int t=0; t<THERMOSTAT_DS18B20_NUMBER;t++){
      if ((Temperature[t] >=1)&&(Temperature[t]<=49)){v =(Temperature[t]*10)/2; Respuesta[34+t]=v+1;}else{Respuesta[34+t]=1;}
    }
  #endif 
  Respuesta[LongCad-1]='\0';
  EnviarRespuesta(Respuesta); 
}
void SelectScene(int Dir){Dir = (Dir-1) * 30;for (byte i =0 ; i<Number_Circuit;i++){byte val =EepromRead(Dir+i); if (val<250){circuits[i].Value=val;}}}
void EnviarRespuesta(char  ReplyBuffer[])
{
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
}
void ActualizaMinuto()
{  
  //if ((hour==0)&&minute==0)){}//CalculaCrepusculo();}
  #ifdef SD_CARD
    if ((minute==0)||(minute==15)||(minute==30)||(minute==45)){GuardaHistorico();}
  #endif
  for (int c=0;c<Number_Circuit;c++){if ((circuits[c].Type==Riego_Temporizado)&&(circuits[c].Value>=1)){circuits[c].Value--;}}//Riego Temporizado
  if(Enable_DaylightSavingTime==true)
  {
      //Adelanta la hora.Apartir del dia 25 de Marzo, busca el primer domingo
      //y cuando se han las 2 de la noche adelanta el reloj una hora
      if (month==3)
      {
         if (dayOfMonth >= 26)
         {
           if (dayOfWeek == 7)      
           {
             if (hour==2)
             {
                hour = 3;
                setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
             }
           }
           
         }
      }
      //Retrasa la hora.Apartir del dia 25 de Octubre, busca el primer domingo
      //y cuando se han las 2 de la noche retrasa el reloj una hora
      if (month==10)
      {
         if (dayOfMonth >= 26)
         {
           if (dayOfWeek == 7)      
           {
             if ((hour==2)&&(HoraRetrasa==false))
             {
                hour = 1;
                HoraRetrasa=true;
                setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
             }
           }
           
         }
      }
    }
    if (hour==3){HoraRetrasa=false;}
    minutoMemory=minute;
    

    TipoDia=dayOfWeek;
    int Reg;
  
    for (Reg=3900; Reg<=3948;Reg=Reg+2){if (month == EepromRead(Reg)){if (dayOfMonth== EepromRead(Reg+1)){TipoDia=8;}}}//Verificacion Dia Especial 1
    for (Reg=3950; Reg<=3998;Reg=Reg+2){if (month == EepromRead(Reg)){if (dayOfMonth== EepromRead(Reg+1)){TipoDia=9;}}}//Verificacion Dia Especial 2

    int R= ((TipoDia-1)*320)+1000;
    for (Reg=R; Reg<=(R+316);Reg=Reg+4)    {if ((hour==EepromRead(Reg))&&(minute==EepromRead(Reg+1))){byte ci=EepromRead(Reg+2);if ((ci>=0) &&(ci<50) && (EepromRead(ci+400)==1)){if (ci<Number_Circuit){circuits[ci].Value=EepromRead(Reg+3);}else{if ((ci>29)&&(ci<40)){SelectScene(ci-29);}else{if (EepromRead(Reg+3)==1){Condicionados[ci-40]=true;}else{Condicionados[ci-40]=false;}}}}}} 

    for (Reg=950; Reg<=994;Reg=Reg+4)    {if ((hour==EepromRead(Reg))&&(minute==EepromRead(Reg+1))){EepromWrite(Reg, 66); byte ci=EepromRead(Reg+2);if ((ci>=0) &&(ci<50) ){if (ci<Number_Circuit){circuits[ci].Value=EepromRead(Reg+3);}else{if ((ci>29)&&(ci<40)){SelectScene(ci-29);}else{if (EepromRead(Reg+3)==1){Condicionados[ci-40]=true;}else{Condicionados[ci-40]=false;}}}}}} 

}
*/
//Envento cada minuto.
void ActualizaMinuto()
{  
    #ifdef SD_CARD
      MinutesLstHist++;
      if (MinutesLstHist>=MinutesHistorico){GuardaHistorico();MinutesLstHist=0;}
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
void ReadDate(){
  char Respuesta[26];
  Respuesta[0]='E';
  Respuesta[1]='S';
  Respuesta[2]='T';
  Respuesta[3]='A';
  Respuesta[4]='C';
  Respuesta[5]='T';
       
  Respuesta[6]=TipoDia + 1;
  Respuesta[7]=hour + 1;
  Respuesta[8]=minute + 1;
  Respuesta[9]=dayOfMonth + 1;
  Respuesta[10]=month + 1;
  Respuesta[11]=year + 1;
       
  Respuesta[12]='\0';
  
  EnviarRespuesta(Respuesta); 
}


void CargaHora()
{
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  Tim30Sg=millis();
  if (minute != minutoMemory){ActualizaMinuto();NewMinute();  }
}

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
  return result;
}
void connectAndRfr(){
  EspRfrIp=50;
  if (Connecting){return;}   
  if (Mail==""){return;}
  
  boolean result = CreateCabHTTP("GET http://excontrol.es/Users/IpSet.aspx?Mail=","");
  if (result){ComproRespuestaHTTP(); }
}
boolean CreateCabHTTP(char* URL, String Key2){
  #ifdef DEBUG_MODE   
    Serial.println("Coneccting http server");               
  #endif
  if (client.connect("www.excontrol.es", 80)) {
     #ifdef DEBUG_MODE   
      Serial.println("Conected");               
    #endif
    client.print(URL);
    if (Key2==""){client.print(Mail + "&Key=" + Key);}
    else{client.print(Mail + "&Key=" + Key + "&Key2=" + Key2);}
    client.println(" HTTP/1.1");
    client.println("Host: www.excontrol.es");
    client.println("Connection: close");
    client.println();
    Connecting=true;
    return true;
  }
  #ifdef DEBUG_MODE   
    Serial.println("ERROR Coneccting server");               
  #endif
  Connecting=false;
  return false;
}
boolean ComproRespuestaHTTP(){
  #ifdef DEBUG_MODE   
    Serial.println("Esperando Respuesta Server");               
  #endif
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
        Serial.println("Respuesta Completa");               
      #endif
      return true;
    }
    else{
      SystemLoop();
      delay(10);
      if (Reintento >= 700 ){
        #ifdef DEBUG_MODE   
            Serial.println("No se recibio respuesta de servidor");               
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
#ifdef SD_CARD
  void SecutityCopy(){
    if (SdOk==false){return;}  
    boolean Exit=true;
    for (int c=0;c<=Number_Circuit;c++){
      if (circuits[c].Value!=circuits[c].CopyRef){Exit=false;circuits[c].CopyRef=circuits[c].Value; } 
    }
    if (Exit){return;}
  
    EnableSD();
    if (SD.exists("elc.txt")) {SD.remove("elc.txt");}
    SdFile = SD.open("elc.txt", FILE_WRITE);
    if (SdFile) {for (int c=0;c<Number_Circuit;c++){SdFile.write(circuits[c].Value);}
      SdFile.close();
      #ifdef DEBUG_MODE   
         Serial.println("Security copy sd full");               
      #endif
      
    }  
    
    else { 
      #ifdef DEBUG_MODE   
         Serial.println("error opening sd file");               
      #endif
    }
    EnableEthernet();
  }
  void EnableSD(){
    #ifdef DEBUG_MODE   
         Serial.print("SPI SD ENABLED.");
    #endif
    //Para comunicar con sd desabilitamos w5100 spi (pin 10 HIGH)
    // Para comunicar con sd habilitamos sd spi (pin 4 low)
    digitalWrite(SS_ETHERNET, HIGH);
    digitalWrite(SS_SD, LOW);
    digitalWrite(SS_UNO, HIGH);
  }
  void EnableEthernet(){
     #ifdef DEBUG_MODE   
         Serial.print("SPI ETHERNET ENABLED.");
    #endif
    digitalWrite(SS_ETHERNET, LOW);
    digitalWrite(SS_SD, HIGH);
    digitalWrite(SS_UNO, LOW);
  }
  void GuardaHistorico(){

    if (SdOk==false){return;}
    EnableSD();
    #ifdef ENABLE_WATCH_DOG
    wdt_reset();
    #endif
    char Ruta[] = {'H', 'I', 'T', '/', '0', '0', '-','0', '0', '-','0', '0', '.','C','S','V','\0'};
    String Val;
    Val = String(year);
    if (Val.length()==2){Ruta[4]=Val.charAt(0);Ruta[5]=Val.charAt(1);}else{Ruta[5]=Val.charAt(0);}
    Val = String(month);
    if (Val.length()==2){Ruta[7]=Val.charAt(0);Ruta[8]=Val.charAt(1);}else{Ruta[8]=Val.charAt(0);}
    Val = String(dayOfMonth);
    if (Val.length()==2){Ruta[10]=Val.charAt(0);Ruta[11]=Val.charAt(1);}else{Ruta[11]=Val.charAt(0);}
    SdFile = SD.open(Ruta, FILE_WRITE);
    
    if (SdFile) {    
      char s[]={'0','0',':','0','0','\0'};
      Val = String(hour);
      if (Val.length()==2){s[0]=Val.charAt(0);s[1]=Val.charAt(1);}else{s[1]=Val.charAt(0);}
      Val = String(minute);
      if (Val.length()==2){s[3]=Val.charAt(0);s[4]=Val.charAt(1);}else{s[3]=Val.charAt(0);}  
      SdFile.println(s);
      int c=1;
      String Line="";
      char* se  =ReadSensor(c);
     
      
         
      while ((se!="RESERVA")&&(c<15)){
        int LongCad = strlen(se); 
        char Respuesta[LongCad  + 1];
        for (int p=0;p<LongCad;p++){ Respuesta[p]=se[p];}
        Respuesta[LongCad]='\0';       
        c++;
        Line=Line +  Respuesta + ',';
        se  =ReadSensor(c);
      }
      
      SdFile.println(Line);
      SdFile.close();
      
    }
    else
      {
        #ifdef DEBUG_MODE   
        Serial.println("Error escritura fichero historico");   
        Serial.println(Ruta);
        #endif
     }
     EnableEthernet();
}
String ReadFile(int Linea,char Ruta[]){
    
    if (SdOk==false){return"NOFOUND!!";}
    EnableSD();
    #ifdef ENABLE_WATCH_DOG
      wdt_reset();
    #endif
    
    int Lin=0;
    String Resultado;
    byte Bin;
    SdFile = SD.open(Ruta);
    if (SdFile) {
    while (SdFile.available()) {
      Bin=SdFile.read();
      if (Bin==13){Lin++;SdFile.read();}
      else
      {
        if (Lin==Linea){Resultado=Resultado+(char(Bin));}
        if (Lin>Linea){SdFile.close();EnableEthernet();
          if (Resultado==""){return"NOFOUND!!";}else {return Resultado;}
        }
      }
      }
      EnableEthernet();SdFile.close();return"NOFOUND!!";
    
    }
    EnableEthernet();return"NOFOUND!!";
}
#endif
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
         float RangoTemp =((float) circuits[c].Value )+ Histeresis;        
         if (Temperature[circuits[c].Device_Number]   >= RangoTemp ){ThermostatCool[circuits[c].Device_Number]=true;}
         else{RangoTemp = RangoTemp - 1;if (Temperature[circuits[c].Device_Number] <= RangoTemp){ThermostatCool[circuits[c].Device_Number]=false;}}
         RangoTemp =((float) circuits[c].Value) - Histeresis;
         if (Temperature[circuits[c].Device_Number]<= RangoTemp ){ThermostatHeat[circuits[c].Device_Number]=true;} 
         else{RangoTemp = RangoTemp +1;if (Temperature[circuits[c].Device_Number] >= RangoTemp){ThermostatHeat[circuits[c].Device_Number]=false;}}  
       }
     }
     sensorTemp.requestTemperatures();
  }
#endif 

char* CharNull(char* Val){
      int LongCad= strlen(Val); 
      if (Val[LongCad-1]=='\0'){return Val;}
      char Resultado[LongCad + 1];
      Resultado[LongCad]='\0';
      for (int c=0;c<LongCad;c++){Resultado[c]=Val[c];}
      return Resultado;
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

//Funciones alarmas
void SetAlarm(int Number){if ((Number<=19)&&(Alarms[Number]==0)){Alarms[Number]=1;}}
void ResetAlarm(int Number){if ((Number<=19)&&(Alarms[Number]==2)){Alarms[Number]=0;}}
#ifdef ARDUINO_MEGA
//    ARDUINO MEGA..........................................................   
    

    byte EepromRead ( unsigned int eeaddress){ EEPROM.read(eeaddress);}
    void EepromWrite ( unsigned int eeaddress, byte data ){
      if(EepromRead(eeaddress) != data)
      EEPROM.write(eeaddress, data);
    }
   
#else
//    ARDUINO CON Atmeg328 (ARDUINO UNO,ARDUINO ETHERNET. ETC.............
///   USO DE MEMORIA EXTERNA.
  #ifdef IC24C32_I2C_ADDRESS 
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
      #else
        byte EepromRead ( unsigned int eeaddress){return 0;}
        void EepromWrite ( unsigned int eeaddress, byte data ){return;}
      #endif
#endif