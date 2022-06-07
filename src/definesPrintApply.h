/* Projeto Print Apply - Software desenvolvido para a 
placa industrial V2.0 comunicando com a IHM - v1.0 */

// Defines utilizados no Projeto Print Apply Linear

#ifndef DEFINESPRINTAPPLY_H
#define DEFINESPRINTAPPLY_H


/////////////////////////////////////////////////
// SOFTWARE DEFINES: ////////////////////////////
// Eeprom: (512 bytes no esp32)
#define EEPROM_SIZE 512
#define EPR_maxProdutos 15      // qual o número máximo de produtos/receitas/programas a máquina vai armazenar
#define EPR_offsetProduto 12    // quanto de memória, em bytes, um produto ocupa
#define EPR_offsetEspecificos 32 // as variaveis que dependem de produto são armazenadas depois desse endereço, as que não dependem de produto, antes.

/* PARÂMETROS GERAIS: Os parametros abaixo são salvos na eeprom uma única vez por produto,
são PARÂMETROS NÃO-ESPECÍFICOS: */
#define EPR_produto 0

#define EPR_pulsosBracoInicial 4
#define EPR_pulsosBracoAplicacao 8
#define EPR_espacamentoProdutomm 12

#define EPR_tempoFinalizarAplicacao 16

#define EPR_rampa 20

#define EPR_contadorAbsoluto 24

#define EPR_statusIntertravamentoIn 28

/* PARÂMETROS GERAIS: Os parametros abaixo são salvos na eeprom uma única vez por produto,
são PARÂMETROS NÃO-ESPECÍFICOS: */

// PARÂMETROS ESPECÍFICOS: salvos uma vez para cada produto
#define EPR_atrasoSensorProduto 0
#define EPR_atrasoImpressaoEtiqueta 4
#define EPR_velocidadeLinearmmps 8
// PARÂMETROS ESPECÍFICOS: salvos uma vez para cada produto
// Eeprom:

// Fases:
#define fase11 11
#define fase12 12
#define fase13 13
#define fase14 14
#define fase15 15
#define fase16 16
#define fase17 17
#define fase18 18
#define fase19 19
// Fases:
/////////////////////////////////////////////////
// SOFTWARE DEFINES: ////////////////////////////


/////////////////////////////////////////////////
// HARDWARE DEFINES: ////////////////////////////
// Sensores:
#define PIN_SENSOR_PRODUTO PIN_HSDI1 // Sensor de Produto ou sinal de START
#define PIN_SENSOR_HOME PIN_HSDI2 // Sensor de Posição (HOME 1)
#define PIN_SENSOR_APLICACAO PIN_HSDI3 // Sensor de Aplicação
#define PIN_SENSOR_ESPATULA PIN_HSDI4 // Sensor de Espátula (HOME 2)

#define PIN_EMERGENCIA DI5
#define PIN_SENSOR_DE_PORTAS DI6
// Sensores:

// SATO / impressora:
#define PIN_PREND PIN_HSDI4
#define PIN_M_ERR DI1

#define PIN_PRIN PIN_DO1
#define PIN_PRIN2 PIN_DO2

// Botões:
#define BUTTON_START PIN_DI1 // to do:
#define BUTTON_CIMA DI5
#define BUTTON_BAIXO DI6
#define BUTTON_ESQUERDA DI7
#define BUTTON_DIREITA DI8
// Botões:

// Ventilador:
#define PWM_VENTILADOR PIN_DO3
#define FEEDBACK_VENTILADOR DI2
#define VENTILADOR_CANAL CANAL_0
// Ventilador:

// Motores:
// Motor Principal:
#define PIN_PUL PIN_HSDO1
#define PIN_DIR PIN_DO1
// Motor Principal:
// Motor Espátula:
#define PIN_PUL_ESP PIN_SCL          
#define PIN_DIR_ESP PIN_SDA
// Motor Espátula:

// Habilita Motores:
#define PIN_ENABLE_MOTORES DO4
// Habilita Motores:

#define DIRECAO_HORA 0
#define DIRECAO_ANTIHORA 1
// Motores:

// Intertravamento:
#define INTERTRAVAMENTO_IN_1 DI3
#define PIN_INTERTRAVAMENTO_OUT RLO1
// Intertravamento:

// Status Intertravamento:
#define INTERTRAVAMENTO_IN_OFF 0
#define INTERTRAVAMENTO_IN_ON 1
// Status Intertravamento:

// Sinalização:
#define LED_STATUS RLO2
// Sinalização:

// HARDWARE DEFINES: ////////////////////////////
/////////////////////////////////////////////////
#endif