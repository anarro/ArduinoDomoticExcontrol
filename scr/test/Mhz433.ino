 #ifdef RECEIVER_433  
  void Init433Mhz(){mySwitch.enableReceive(0);}// Receiver on inerrupt 0 => that is pin #2

  void Recepcion433Mhz() {
    if (mySwitch.available()) {
      
      int value = mySwitch.getReceivedValue();
      #ifdef DEBUG_MODE   
         if (value == 0) {
          Serial.print("Unknown encoding");
         } else {
          Serial.print("Received ");
          Serial.print(value);
          Serial.print(" / ");
          Serial.print( mySwitch.getReceivedBitlength() );
          Serial.print("bit ");
          Serial.print("Protocol: ");
          Serial.println( mySwitch.getReceivedProtocol() );
        }
      #endif
      if (value == RfInputOn0){SwicthStateChange(100, HIGH);}
      if (value == RfInputOn1){SwicthStateChange(101, HIGH);}
      if (value == RfInputOn2){SwicthStateChange(102, HIGH);}
      if (value == RfInputOn3){SwicthStateChange(103, HIGH);}
      if (value == RfInputOn4){SwicthStateChange(104, HIGH);}
      if (value == RfInputOn5){SwicthStateChange(105, HIGH);}
      if (value == RfInputOn6){SwicthStateChange(106, HIGH);}
      if (value == RfInputOn7){SwicthStateChange(107, HIGH);}
      if (value == RfInputOn8){SwicthStateChange(108, HIGH);}
      if (value == RfInputOn9){SwicthStateChange(109, HIGH);}
      if (value == RfInputOn10){SwicthStateChange(110, HIGH);}
      if (value == RfInputOn11){SwicthStateChange(111, HIGH);}
      if (value == RfInputOn12){SwicthStateChange(112, HIGH);}
      if (value == RfInputOn13){SwicthStateChange(113, HIGH);}
      if (value == RfInputOn14){SwicthStateChange(114, HIGH);}
      
      if (value == RfInputOff0){SwicthStateChange(100, LOW);}
      if (value == RfInputOff1){SwicthStateChange(101, LOW);}
      if (value == RfInputOff2){SwicthStateChange(102, LOW);}
      if (value == RfInputOff3){SwicthStateChange(103, LOW);}
      if (value == RfInputOff4){SwicthStateChange(104, LOW);}
      if (value == RfInputOff5){SwicthStateChange(105, LOW);}
      if (value == RfInputOff6){SwicthStateChange(106, LOW);}
      if (value == RfInputOff7){SwicthStateChange(107, LOW);}
      if (value == RfInputOff8){SwicthStateChange(108, LOW);}
      if (value == RfInputOff9){SwicthStateChange(109, LOW);}
      if (value == RfInputOff10){SwicthStateChange(110, LOW);}
      if (value == RfInputOff11){SwicthStateChange(111, LOW);}
      if (value == RfInputOff12){SwicthStateChange(112, LOW);}
      if (value == RfInputOff13){SwicthStateChange(113, LOW);}
      if (value == RfInputOff14){SwicthStateChange(114, LOW);}
      
      mySwitch.resetAvailable();
   }
}
#endif
#ifdef TRANSMITER_433
  void MhzOutControl(int OutNumber,boolean  On){ 
  /*************************************************************/
  if((OutNumber==0)&&On){mySwitch.send(5000, 24);}
  if((OutNumber==0)&&(!On)){mySwitch.send(5100, 24);}
  if((OutNumber==1)&&On){mySwitch.send(5001, 24);}
  if((OutNumber==1)&&(!On)){mySwitch.send(5101, 24);}
  if((OutNumber==2)&&On){mySwitch.send(5002, 24);}
  if((OutNumber==2)&&(!On)){mySwitch.send(5102, 24);}
  if((OutNumber==3)&&On){mySwitch.send(5003, 24);}
  if((OutNumber==3)&&(!On)){mySwitch.send(5103, 24);}
  if((OutNumber==4)&&On){mySwitch.send(5004, 24);}
  if((OutNumber==4)&&(!On)){mySwitch.send(5104, 24);}
  if((OutNumber==5)&&On){mySwitch.send(5005, 24);}
  if((OutNumber==5)&&(!On)){mySwitch.send(5105, 24);}
  if((OutNumber==6)&&On){mySwitch.send(5006, 24);}
  if((OutNumber==6)&&(!On)){mySwitch.send(5106, 24);}
  if((OutNumber==7)&&On){mySwitch.send(5007, 24);}
  if((OutNumber==7)&&(!On)){mySwitch.send(5107, 24);}
  if((OutNumber==8)&&On){mySwitch.send(5008, 24);}
  if((OutNumber==8)&&(!On)){mySwitch.send(5108, 24);}
  if((OutNumber==9)&&On){mySwitch.send(5009, 24);}
  if((OutNumber==9)&&(!On)){mySwitch.send(5109, 24);}
  if((OutNumber==10)&&On){mySwitch.send(5010, 24);}
  if((OutNumber==10)&&(!On)){mySwitch.send(5110, 24);}
  if((OutNumber==11)&&On){mySwitch.send(5011, 24);}
  if((OutNumber==11)&&(!On)){mySwitch.send(5111, 24);}
  if((OutNumber==12)&&On){mySwitch.send(5012, 24);}
  if((OutNumber==12)&&(!On)){mySwitch.send(5112, 24);}
  if((OutNumber==13)&&On){mySwitch.send(5013, 24);}
  if((OutNumber==13)&&(!On)){mySwitch.send(5113, 24);}
  if((OutNumber==14)&&On){mySwitch.send(5014, 24);}
  if((OutNumber==14)&&(!On)){mySwitch.send(5114, 24);}

  #ifdef DEBUG_MODE  
     Serial.print("Mhz433 Out ");Serial.print(OutNumber);if (On){Serial.println("=ON");}else{Serial.println("=Off");}
   #endif
  }
#endif
