

#if defined (ETHERNET_SHIELD) || (WIFI_SHIELD)
  #define HTTP
#endif

#ifndef HTTP
  void connectAndRfr(){
  }
#else
// Refresco Ip con el servidor, 
#define TIME_REFRESH_IP 50

static byte EspRfrIp=TIME_REFRESH_IP;

boolean ComproRespuestaHTTP(){
  int Reintento=0;
  while(true){
    if (client.available()) {
      int c;
      for (c=0;c<5;c++){char c = client.read();}
      #ifdef DEBUG_MODE   
        Serial.println("Coneccting completed");               
      #endif
      client.stop();
      client.flush();
      EspRfrIp=50;
      Connecting=false;
      return true;
    }
    else{
 //     SystemLoop();
      delay(10);
      if (Reintento >= 700 ){
        #ifdef DEBUG_MODE   
          Serial.println("Coneccting HTTP ERROR");               
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
    Serial.println("Conected");               
    #endif
    return true;
  }
   #ifdef DEBUG_MODE   
    Serial.println("Coneccting error");               
    #endif
  Connecting=false;
  return false;
}


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
  
  if ((Connecting) || (Mail=="")){
    return;
  }   
 
  if (EspRfrIp-- ==  0){  
    boolean result = CreateCabHTTP("GET http://excontrol.es/Users/IpSet.aspx?Mail=","");
    if (result){ComproRespuestaHTTP(); }
  }
}
#endif




