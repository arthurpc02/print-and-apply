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
#define EPR_quantoOcupaCadaProdutoNaMemoria 16    // quanto de memória, em bytes, um produto ocupa
#define EPR_inicioDaMemoriaDosProdutos 64 // as variaveis que dependem de produto são armazenadas depois desse endereço, as que não dependem de produto, antes.

// PARÂMETROS Globais:
#define EPR_produto 0
#define EPR_contadorTotal 4
#define EPR_tempoFinalizarAplicacao 8
#define EPR_posicaoLimite_dcmm 12
#define EPR_posicaoDePegarEtiqueta_dcmm 16
#define EPR_posicaoDeRepouso_dcmm 20
#define EPR_velocidadeDeReferenciacao_dcmm 24
#define EPR_rampa_dcmm 28
#define EPR_velocidadeRebobinador 32
#define EPR_aceleracaoRebobinador 36
#define EPR_habilitaPortasDeSeguranca 40
#define EPR_startNF 44 // to do:
#define EPR_potenciaVentilador 48
#define EPR_rampaReferenciacao_dcmm 52
#define EPR_modoDeFuncionamento 56
#define EPR_modoDeImpressao 60

// PARÂMETROS ESPECÍFICOS
#define EPR_atrasoSensorProduto 0
#define EPR_posicaoDeAguardarProduto_dcmm 1
#define EPR_distanciaProduto_dcmm 2
#define EPR_velocidadeDeTrabalho_dcmm 3

/////////////////////////////////////////////////
// HARDWARE DEFINES: ////////////////////////////
// Sensores:
#define PIN_SENSOR_PRODUTO PIN_HSDI1 // Sensor de Produto ou sinal de START
#define PIN_SENSOR_HOME PIN_HSDI2 // Sensor de Posição (HOME 1)
#define PIN_SENSOR_APLICACAO PIN_HSDI3 // Sensor de Aplicação

#define PIN_EMERGENCIA DI5
#define PIN_SENSOR_DE_PORTAS DI6
// Sensores:

// SATO / impressora:
#define PIN_PREND PIN_HSDI4
#define PIN_IMPRESSORA_ONLINE PIN_DI1 // quando a impressora funcionar em modo "padrao" (com buffer de impressao na SATO),
                                      // tem que utilizar o EXT 9PIN SELECT no modo 3 (SERVICE MODE > SETTING > EXT 9PIN SELECT > MODE3).
                                      // E quando utilizar a impressora em modo "diversos produtos" (bartender seleciona qual etiqueta
                                      // e envia uma a uma), tem que utilizar o EXT 9PIN SELECT no modo 3(SERVICE MODE > SETTING > EXT 9PIN SELECT > MODE2).

#define PIN_PRIN PIN_DO1
#define PIN_PRIN2 PIN_DO2

// Ventilador:
#define PIN_VENTILADOR PIN_DO3
#define VENTILADOR_CANAL CANAL_0

// outras interfaces:
#define PIN_SUNNYVISION_A PIN_DI2
#define PIN_SUNNYVISION_B DI3
#define PIN_SUNNYVISION_INTT DI4
#define PIN_FIM_DE_APLICACAO DO5

// Motores:
// Motor Principal:
#define PIN_PUL_BRACO PIN_HSDO1
#define PIN_DIR_BRACO PIN_HSDO3

// Motor Rebobinador:
#define PIN_PUL_REBOBINADOR PIN_HSDO2          
#define PIN_DIR_REBOBINADOR PIN_SDA

// Habilita Motores:
#define PIN_ENABLE_MOTORES DO4

#define PIN_TORRE_LUMINOSA RLO2
#define PIN_INTERTRAVAMENTO_OUT RLO1

// HARDWARE DEFINES: ////////////////////////////
/////////////////////////////////////////////////
#endif