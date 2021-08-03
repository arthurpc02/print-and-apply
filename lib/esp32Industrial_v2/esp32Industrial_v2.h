// Industrial v2.0 board hardware header

/* A função desse header é definir nomes, códigos e pinagens que serão reutilizados em vários projetos 
que utilizam a placa Industrial v2.0.
*/

#ifndef ESP32INDUSTRIAL_V2_H
#define ESP32INDUSTRIAL_V2_H

/////////////////////////////////////////////////
// SOFTWARE DEFINES: ////////////////////////////
#define OK 15
#define ERROR_NOTSPECIFIED 29
#define ERROR_TIMEOUT 2
#define NOT_DONE 13

#define fase0 0
#define fase1 1 // normalmente a fase1 de cada fsm contém os comandos que devem ser executados apenas uma vez,
#define fase2 2 // no começo daquela fsm. Uma outra fase para os comandos que devem ser repetidos, e a última
#define fase3 3 // para comandos executados uma vez ao sair daquela fsm
#define fase4 4
#define fase5 5
#define fase6 6
#define fase7 7
#define fase8 8
#define fase9 9
#define fase10 10

#define CORE_0 0
#define CORE_1 1
// Arduino IDE code runs on core 1 by default
// FreeRTOS runs on core 0 by default

// Zero é a prioridade mais baixa do freeRTOS
#define PRIORITY_0 0
#define PRIORITY_1 1
#define PRIORITY_2 2
#define PRIORITY_3 3
#define PRIORITY_4 4
#define PRIORITY_5 5
// Cinco é a prioridade mais alta do freeRTOS

#define ACTIVE_LOW LOW
#define ACTIVE_HIGH HIGH

/////////////////////////////////////////////////
// HARDWARE DEFINES: ////////////////////////////
// Shift registers:
#define PIN_OUTPUT_EN 33
#define PIN_IO_CLOCK 5
#define PIN_OUTPUT_DATA 32
#define PIN_INPUT_DATA 36
#define PIN_IO_LATCH 25
// Shift registers:

// Inputs:
#define PIN_HSDI1 13
#define PIN_HSDI2 14
#define PIN_HSDI3 34
#define PIN_HSDI4 39
#define PIN_EMERGENCIA 35
// Inputs:

// Outputs:
#define PIN_HSDO1 4
#define PIN_HSDO2 16
#define PIN_HSDO3 17
#define PIN_HSDO4 18
#define PIN_DO1 19
#define PIN_DO2 23
#define PIN_STATUS 2
// Outputs:

// Comunicação:
#define PIN_SDA 21
#define PIN_SCL 22

#define PIN_RS485_RX 27
#define PIN_RS485_TX 26
#define PIN_RS485_EN 12

#define RS485_TRANSMIT HIGH
#define RS485_RECEIVE LOW
// Comunicação:

// IO's multiplexados:
#define DI1 7
#define DI2 6
#define DI3 5
#define DI4 4
#define DI5 3
#define DI6 2
#define DI7 1
#define DI8 0

#define DO3 0
#define DO4 1
#define DO5 2
#define DO6 3
#define DO7 5
#define DO8 4
#define RLO1 6
#define RLO2 7
// IO's multiplexados:

// Canais:
#define CANAL_0 0
#define CANAL_1 1
#define CANAL_2 2
#define CANAL_3 3
#define CANAL_4 4
#define CANAL_5 5
#define CANAL_6 6
#define CANAL_7 7
#define CANAL_8 8
// Canais:

#endif