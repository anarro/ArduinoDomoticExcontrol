
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


#ifdef IR_RECIVE

void ComprobarInfrarro() {
  
  if (irrecv.decode(&results)){            
     //RecepcionInfrarrojos(&results);
     int codeType = -1; // The type of code
     decode_results *result;
     result=&results;
     codeType = result->decode_type;
     
     if (codeType == NEC) {
       //En este caso es nec pero puede ser JVC, PANASONIC...
       //Tienes que adaptarlo a tu mando a distancia
       switch (result->value){
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
       /*
       //Subir 20% Persiana 1
        case  0x40BFB847://cambia el codigo por el que envio tu mando a distancia 
          if (Value[23]<=80){Value[23]+=20;}else{Value[23]=100;}
          break;  
        //bajar 20% Persiana 1
        case  0xC03F807F://cambia el codigo por el que envio tu mando a distancia
          if (Value[23]>=20){Value[23]-=20;}else{Value[23]=0;}
          break;  
        //Subir 20% Persiana 2  
        case  0x40BF38C7://cambia el codigo por el que envio tu mando a distancia
          if (Value[24]<=80){Value[24]+=20;}else{Value[24]=100;}
          break;  
        //bajar 20% Persiana 2
        
        case  0xC03F00FF://cambia el codigo por el que envio tu mando a distancia
          if (Value[24]>=20){Value[24]-=20;}else{Value[24]=0;}
          break; 
         */
       }  
      }  
     
     
     
     
     irrecv.resume(); // Receive the next value.
     
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

#ifdef LED_IR
void SendIr(byte CommandNumber){
  //Example  lg tv
  //Ejemplo uso tv lg
 
  switch (CommandNumber) {
    case 1:    
      //APAGAR
      irsend.sendNEC(0x20DF10EF, 32); 
      delay(40);      
      
    case 2:    
      //INPUT
      irsend.sendNEC(0x20DFD02F, 32); 
      delay(40);
      break;
      
    case 3:    
      //TV-RADIO
      irsend.sendNEC(0x20DF0FF0, 32); 
      delay(40);
      break;
  
    case 4:    
      //GUIA
      irsend.sendNEC(0x20DFD52A, 32); 
      delay(40);
      break;    
     case 5:    
      //SUBIR CANAL
      irsend.sendNEC(0x20DF00FF, 32); 
      delay(40);
      break;  
      
     case 6:    
      //BAJAR CANAL
      irsend.sendNEC(0x20DF807F, 32); 
      delay(40);
      break;   
      
     case 7:    
      //SUBIR VOLUMEN
      irsend.sendNEC(0x20DF40BF, 32); 
      delay(40);
      break;    
      
    case 8:    
      //BAJAR VOLUMEN
      irsend.sendNEC(0x20DFC03F, 32); 
      delay(40);
      break; 
   
     case 9:   
      //1
      irsend.sendNEC(0x20DF8877, 32); 
      delay(40);
      break; 
      
     case 10:   
      //2
      irsend.sendNEC(0x20DF48B7, 32); 
      delay(40);
      break;   
     
     case 11:   
      //3
      irsend.sendNEC(0x20DFC837, 32); 
      delay(40);
      break;  
     
     case 12:   
      //4
      irsend.sendNEC(0x20DF28D7, 32); 
      delay(40);
      break;  
        
     case 13:   
      //5
      irsend.sendNEC(0x20DFA857, 32); 
      delay(40);
      break;  
    
    case 14:   
      //6
      irsend.sendNEC(0x20DF6897, 32); 
      delay(40);
      break;    
    
        case 15:   
      //7
      irsend.sendNEC(0x20DFE817, 32); 
      delay(40);
      break;    
    
    case 16:   
      //8
      irsend.sendNEC(0x20DF18E7, 32); 
      delay(40);
      break;   
    
     case 17:   
      //9
      irsend.sendNEC(0x20DF9867, 32); 
      delay(40);
      break;   
          
     case 18:   
      //0
      irsend.sendNEC(0x20DF08F7, 32); 
      delay(40);
      break;   
     
     case 19:   
      //SILENCIO
      irsend.sendNEC(0x20DF906F, 32); 
      delay(40);
      break;   
     }   
   #ifdef IR_RECIVE
     irrecv.enableIRIn(); // Re-enable receiver
   #endif 
}
#endif 


