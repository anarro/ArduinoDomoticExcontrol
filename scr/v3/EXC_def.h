/*
	Este fichero crea las definiciones del proyecto Excontrol.

*/

//definir pines control SD
#define SS_SD 4
#define SS_UNO 10
#define SS_ETHERNET 53 //53 for mega, for other  pin  10


//Direccion de trabajo para modulos RTC.
#define DS_RTC 0x68  //Direccion Reloj

#ifdef moduleDS1307
  #define IC24C32_I2C_ADDRESS 0x50
#endif

#ifdef moduleDS3231
  #define IC24C32_I2C_ADDRESS 0x57
#endif









/*
	#Numeración de tipos de circuitos.
*/

#define Reserva       	1
#define Ado_Digital   	2
#define Ado_3Etapas   	3
#define Ado_Retroaviso 	4
#define Enchufe		8
#define EnchufeRF	9
#define	Riego		14
#define Riego_Temporizado 15
#define Valvula		16
#define ConsignaTemp	30
#define Frio		20
#define Calor		25
#define Radiante	26
#define	Persiana	35
#define Toldo		36
#define	Puerta		40
#define	Ventilador	44
#define	Piloto  	52


/*
	#Precompilador calculo posiciones memoria EEProm.
	
	
	# N_name			=> Number de objets.
	# S_name			=> Size de objets in bytes.
	# Name_OFSSET 	=> Direccion inicial bloque de memoria Eeprom
*/

/*
	NUMBER SLOTS.
*/
#define N_ESCENES 	 	10
#define N_EN_TIMETABLE	 	50	//(N_CIRCUITS, N_ESCENES, N_CONDITIONED)
#define N_CIRCUITS	 	30
#define	N_UP_TIM_SHUTTER	30
#define	N_DO_TIM_SHUTTER	30
#define N_TRIGGER		12
#define N_TIME_DAY		80	//			 daily schedule
#define N_TIME_ESPECIAL  	80
#define N_DATE_ESPECIAL         25	//month ,day
#define N_ALARMS		20	//BYTE ??
#define N_CONDITIONED		10 
#define N_SETPOINTS		10


/*
	SIZE SLOTS.
*/
#define S_ESCENES 	 	30
#define S_EN_TIMETABLE 		1	//(N_CIRCUITS, N_ESCENES, N_CONDITIONED)
#define S_CIRCUITS	 	1
#define	S_UP_TIM_SHUTTER	1
#define	S_DO_TIM_SHUTTER	1
#define S_TRIGGER		4
#define S_TIME_DAY		4
#define S_TIME_ESPECIAL  	4
#define S_DATE_ESPECIAL    	2	//month ,day			
#define S_ALARMS		1	//BYTE ??
#define S_CONDITIONED		1 
#define S_SETPOINTS		1

#define EM_TIME_ESPECIAL_SIZE    (N_TIME_ESPECIAL * S_TIME_ESPECIAL)
#define EM_DATE_ESPECIAL_SIZE    (N_DATE_ESPECIAL * S_DATE_ESPECIAL)
#define EM_TIME_DAY_SIZE         (N_TIME_DAY * S_TIME_DAY)

#define EM_ESCENES_OFFSET  1
#define EM_ESCENES_END           (N_ESCENES  * S_ESCENES)                                        //300
#define EM_EN_TIMETABLE_OFFSET   (EM_ESCENES_END + 1)                                            //301
#define EM_EN_TIMETABLE_END      (EM_ESCENES_END + (N_EN_TIMETABLE * S_EN_TIMETABLE))            //300 + (50 * 1) = 350
#define EM_CIRCUITS_OFFSET       (EM_EN_TIMETABLE_END + 1)                                       //351
#define EM_CIRCUITS_END          (EM_EN_TIMETABLE_END + ( N_CIRCUITS * S_CIRCUITS))              //350 + (30 * 1) = 380
#define	EM_UP_TIM_SHUTTER_OFFSET (EM_CIRCUITS_END + 1)                                           //381
#define	EM_UP_TIM_SHUTTER_END    (EM_CIRCUITS_END + (N_UP_TIM_SHUTTER * S_UP_TIM_SHUTTER))       //380 + (30 * 1) = 410
#define	EM_DO_TIM_SHUTTER_OFFSET (EM_UP_TIM_SHUTTER_END + 1)                                     //411
#define	EM_DO_TIM_SHUTTER_END    (EM_UP_TIM_SHUTTER_END + (N_DO_TIM_SHUTTER * S_DO_TIM_SHUTTER))// 410 + (30 * 1) = 440
#define EM_ALARMS_OFSSET         (EM_DO_TIM_SHUTTER_END + 1)
#define EM_ALARMS_END            (EM_DO_TIM_SHUTTER_END + (N_ALARMS * S_ALARMS))
#define EM_CONDITIONED_OFSSET    (EM_ALARMS_END + 1)
#define EM_CONDITIONED_END       (EM_ALARMS_END + (N_CONDITIONED * S_CONDITIONED))
#define EM_SETPOINTS_OFSSET 	 (EM_CONDITIONED_END + 1)
#define EM_SETPOINTS_END         (EM_CONDITIONED_END + (N_SETPOINTS * S_SETPOINTS))

// USO de RTC
#define	EM_TRIGGER_OFFSET        (EM_SETPOINTS_END + 1)
#define	EM_TRIGGER_END           (EM_SETPOINTS_END + (N_TRIGGER * S_TRIGGER))
#define	EM_TIME_WEEKLY_OFFSET    (EM_TRIGGER_END + 1)
#define	EM_TIME_WEEKLY_END       (EM_TRIGGER_END + ( 7 * EM_TIME_DAY_SIZE))
#define EM_TIME_ESPECIAL1_OFSSET (EM_TIME_WEEKLY_END + 1)
#define EM_TIME_ESPECIAL1_END    (EM_TIME_WEEKLY_END + EM_TIME_ESPECIAL_SIZE)
#define EM_TIME_ESPECIAL2_OFSSET (EM_TIME_ESPECIAL1_END + 1)
#define EM_TIME_ESPECIAL2_END	 (EM_TIME_ESPECIAL1_END + EM_TIME_ESPECIAL_SIZE)
#define EM_DATE_ESPECIAL1_OFSSET (EM_TIME_ESPECIAL2_END + 1)
#define EM_DATE_ESPECIAL1_END    (EM_TIME_ESPECIAL2_END + EM_DATE_ESPECIAL_SIZE)
#define EM_DATE_ESPECIAL2_OFSSET (EM_DATE_ESPECIAL1_END + 1)
#define EM_DATE_ESPECIAL2_END	 (EM_DATE_ESPECIAL1_END + EM_DATE_ESPECIAL_SIZE)
#define EM_END                   (EM_DATE_ESPECIAL2_END + 1)


#define ISCIRCUIT_CHANGED(n)      (circuits[n].Value != circuits[n].OldValue )
#define ISCIRCUIT_CHAN_LOW(n)     (circuits[n].Value <  circuits[n].OldValue )
#define ISCIRCUIT_CHAN_HIGH(n)    (circuits[n].Value >  circuits[n].OldValue )
