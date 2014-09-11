/*
	Este fichero crea las definiciones del proyecto Excontrol.

*/

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
	
	# EM_nombre_OFSSET 	=> Direccion inicial bloque de memoria Eeprom
	# EM_N_nombre		=> Numero de objetos.
	# EM_S_nombre		=> Tamaño de objetos en bytes.
*/

#define EM_N_CIRCUITOS  		30
#define EM_N_ESCENAS 	 		10
#define EM_N_CONDICIONADOS		10 
#define EM_N_CONSIGNAS			10

#define EM_N_TRIGGER		12 
#define EM_N_TIMETABLE		
#define EM_N_ENABLETIMETABLE	(EM_N_CIRCUITOS, EM_N_ESCENAS, EM_N_CONDICIONADOS)
#define EM_N_DAY1			16
#define EM_N_DAY2			16

	
#define EM_S_ESCENAS  10
#define EM_S_TRIGGER	4  //Circuito o escena o condicionado, estado circuito, hora, minuto.
#define EM_S_DAY1		3	//dia, mes , año
#define EM_S_DAY2		3

#define EM_ESCENAS_OFFSET  0 
#define EM_ESCENAS_END 	(EM_ESCENAS_OFFSET  +( EM_N_CIRCUITOS  * EM_S_ESCENAS)-1)
#define EM_ENHOR_OFFSET (EM_ESCENAS_END+ 1)
#define EM_ENHOR_END   	(EM_ENHOR_OFFSET + (  EM_N_CIRCUITOS + EM_N_ESCENAS +  EM_N_CONDICIONADOS) -1)
#define EM_SEGURIDAD_OFFSET ( EM_ENHOR_END +1 )
#define EM_SEGURIDAD_CIRCUITS_END     (	(EM_SEG_CIRCUITS_OFFSET + EM_N_CIRCUITOS)-1)
#define EM_CONSIGNAS_OFFSET
#define EM_TRIGER_OFFSET
#define EM_SEMANAL_OFFSET
#define EM_DIA7_OFFSET
#define EM_DIA8_OFFSET