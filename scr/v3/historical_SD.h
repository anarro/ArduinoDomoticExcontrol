/**************************************************************************
 #Funciones control SD,escritura y lectura de historicos, estado de los circuitos en SD.
 /* 
 
 #Dependencias:
	Libreria estandar SD.
	Es obligatorio uso de modulo RTC, system_time.h
	Registros globales del sistema "horario" "Circuito".????
 
 
***************************************************************************/


#define HISTORICAL_SD

// PROTOTIPOS EXTERNOS.
extern char* ReadSensor(byte NumeroSensor);

// Variables globales 
boolean CargaSdOk,SdOk;
File SdFile;

void EnableSD(){    
  //Para comunicar con sd desabilitamos w5100 spi (pin 10 HIGH)
  //Para comunicar con sd habilitamos sd spi (pin 4 low) 
  #ifdef ARDUINO_MEGA
    digitalWrite(SS_ETHERNET, HIGH);
  #endif 
  digitalWrite(SS_SD, LOW);
  digitalWrite(SS_UNO, HIGH);
}

void EnableEthernet(){  
  #ifdef ARDUINO_MEGA
    digitalWrite(SS_ETHERNET, LOW);
  #endif 
  digitalWrite(SS_SD, HIGH);
  digitalWrite(SS_UNO, LOW);
  
}

void initPinSD(void){
  boolean CargaSdOk=false;
  pinMode(SS_SD, OUTPUT);//pines de control spi
  pinMode(SS_UNO,  OUTPUT);
  
  #ifdef ARDUINO_MEGA
    pinMode(SS_ETHERNET, OUTPUT);//pines de control spi
  #endif 
}



void initSD(void){
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
       SdOk=true;
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
}

void SecutityCopy(){
      
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
