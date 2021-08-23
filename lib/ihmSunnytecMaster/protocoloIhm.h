#ifndef PROTOCOLOIHM_V1_H
#define PROTOCOLOIHM_V1_H

/* Projeto IHM - Software desenvolvido para a placa industrial v2.0 comunicando com a IHM - v1.0

Biblioteca Protocolo IHM - v1.0

Informações sobre os comandos:
Print: 28
Clear: 29
SetCursor: 31

Para a utilização da impressora zebra utilizar a função imprimirZebra() e statusZebra()
Tamanho máximo de informações 20 caractéres (LCD: 20x4)
*/

#define Baudrate 19200

// Pinos Esp32 para transmissão de dados - RS485:
#define PIN_RS485_RX 27
#define PIN_RS485_TX 26
#define PIN_RS485_EN 12
// Pinos Esp32 para transmissão de dados - RS485:

// Estado do RS485:
#define RS485_TRANSMIT HIGH
#define RS485_RECEIVE LOW
// Estado do RS485:

// Estrutura do protocolo:
#define TAM_msg 8

#define HEADER "#@"
#define ID_CPU 71

#define COMMAND_R 10
#define COMMAND_W 11
#define COMMAND_A 12

#define VALOR1 0
#define VALOR2 0
#define VALOR3 0

#define END_HEADER "@#"

#define SEPARADOR ';'
// Estrutura do protocolo:

// Comandos ZPL:
#define FUNC_IMP_ETIQUETA "~PS"
#define FUNC_PLAY_ETIQUETA "~PS"
#define FUNC_STATUS_IMP "~HQES"
// Comandos ZPL:

#include <Arduino.h>
#include <inttypes.h>
#include <Print.h>
#include <string.h>
#include <string>

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

enum FunctionCode
{
    FUNCTION_PRINT = 28,

    FUNCTION_CLEAR = 29,

    FUNCTION_SETCURSOR = 31,
};

class protocoloIhm
{
public:
    protocoloIhm();

    // Funções:
    void init();

    void print(String);

    void clear();

    void setCursor(String , String);

    void concatenador (String, String, String, String);

    void imprimirImpressoraZebra();

    void funcaoPlayImpressoraZebra();

    void statusImpressoraZebra();

    void envio485(String);
    // Funções:
};
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#endif