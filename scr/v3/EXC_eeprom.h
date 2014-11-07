

#include "EXC_def.h"


/*************************************************************************** 
  SUBRUTINAS TRATAMIENTO EEPROM
  DETECTA EL fMICROCONTOLADOR DE LA TARJETA CONFIGURADA EN EL IDE ARDUINO 
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
