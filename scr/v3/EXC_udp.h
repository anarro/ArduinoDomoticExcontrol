
#define UDP_CONNET


void initUDP(
    Ethernet.begin(mac,ip);
    Udp.begin(localPort);
)

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





//MODIFICADO USO PUNTERO.
void EnviarRespuesta(char  *ReplyBuffer)
{
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
}



*Modificacion principal.....
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
          //excepci�n PENDIENTE EN APPP Ado_Retroaviso=4
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
