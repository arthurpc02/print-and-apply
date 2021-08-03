/* Projeto IHM - Software desenvolvido para a placa industrial v2.0 comunicando com a IHM v1.0

Funções Protocolo IHM - v1.0

*/

#include "protocoloIhm.h"

HardwareSerial rs485(2);

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Funções:
protocoloIhm::protocoloIhm()
{
}

// Função de configuração do RS485:
void protocoloIhm::init()
{
    rs485.begin(Baudrate, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
    rs485.setTimeout(5);

    pinMode(PIN_RS485_EN, OUTPUT);
    digitalWrite(PIN_RS485_EN, RS485_RECEIVE);
}
// Função de configuração do RS485:

// Funções do LCD:
void protocoloIhm::print(String dados)
{
    String informacao = "";
    uint8_t comprimento_inicial = 0;
    uint8_t comprimento_final = 20;

    uint16_t tamanho = dados.length();

    if (tamanho <= 20)
        concatenador(String(FUNCTION_PRINT), dados, String(VALOR2), String(VALOR3));
    else
    {
        informacao = dados.substring(comprimento_inicial, comprimento_final);
        concatenador(String(FUNCTION_PRINT), informacao, String(VALOR2), String(VALOR3));
    }
}

void protocoloIhm::clear()
{
    concatenador(String(FUNCTION_CLEAR), String(VALOR1), String(VALOR2), String(VALOR3));
}

void protocoloIhm::setCursor(String Linha, String Coluna)
{
    uint16_t tamanho_linha = Linha.toInt();
    if (tamanho_linha < 4)
        concatenador(String(FUNCTION_SETCURSOR), Linha, Coluna, String(VALOR3));
}

void protocoloIhm::concatenador(String mensagem1, String mensagem2, String mensagem3, String mensagem4)
{
    String str = "";

    str = HEADER;
    str.concat(SEPARADOR);
    str.concat(ID_CPU);
    str.concat(SEPARADOR);
    str.concat(COMMAND_W);
    str.concat(SEPARADOR);

    if (mensagem1.compareTo(String(FUNCTION_CLEAR)) == 0) 
    {
        str.concat(mensagem1);
        str.concat(SEPARADOR);
        str.concat(mensagem2);
        str.concat(SEPARADOR);
        str.concat(mensagem3);
        str.concat(SEPARADOR);
        str.concat(mensagem4);
        str.concat(SEPARADOR);
        str.concat(END_HEADER);

        envio485(str);
    }
    else if (mensagem1.compareTo(String(FUNCTION_PRINT)) == 0)
    {
        str.concat(mensagem1);
        str.concat(SEPARADOR);
        str.concat(mensagem2);
        str.concat(SEPARADOR);
        str.concat(mensagem3);
        str.concat(SEPARADOR);
        str.concat(mensagem4);
        str.concat(SEPARADOR);
        str.concat(END_HEADER);

        envio485(str);
    }
    else if (mensagem1.compareTo(String(FUNCTION_SETCURSOR)) == 0)
    {
        str.concat(mensagem1);
        str.concat(SEPARADOR);
        str.concat(mensagem2);
        str.concat(SEPARADOR);
        str.concat(mensagem3);
        str.concat(SEPARADOR);
        str.concat(mensagem4);
        str.concat(SEPARADOR);
        str.concat(END_HEADER);

        envio485(str);
    }
}
// Funções do LCD:

// Funções da impressora Zebra:
void protocoloIhm::imprimirImpressoraZebra()
{
    String str = FUNC_IMP_ETIQUETA;
    envio485(str);
}

void protocoloIhm::funcaoPlayImpressoraZebra()
{
    String str = FUNC_PLAY_ETIQUETA;
    envio485(str);
}

void protocoloIhm::statusImpressoraZebra()
{
    String str = FUNC_STATUS_IMP;
    envio485(str);
}
// Funções da impressora Zebra:

// Função de envio da mensagem pelo RS485:
void protocoloIhm::envio485(String mensagem)
{
    digitalWrite(PIN_RS485_EN, RS485_TRANSMIT);

    rs485.print(mensagem);
    rs485.flush();
    Serial.println (mensagem);
    
    delay(10);
    digitalWrite(PIN_RS485_EN, RS485_RECEIVE);
}
// Função de envio da mensagem pelo RS485:

// Funções:

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////