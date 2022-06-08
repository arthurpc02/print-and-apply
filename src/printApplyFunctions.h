/* Projeto Print Apply - Software desenvolvido para a
placa industrial V2.0 comunicando com a IHM - v1.0 */

// Biblioteca Print Apply - v1.0

#ifndef PRINTAPPLYFUNCTIONS_H
#define PRINTAPPLYFUNCTIONS_H

#include <Arduino.h>

#include <Wire.h>
#include <EEPROM.h>

#include <esp32Industrial_v2.1.h>
#include <SunnyAccelStepper.h>
#include <definesPrintApply.h>
#include <ihmSunnytecMaster.h>

enum Estado
{
    // Estados:
    PARADA_EMERGENCIA,
    ATIVO,
    ERRO,
    ESTADO_TESTE,
    // Sub-estados:
    EMERGENCIA_TOP,
    MANUTENCAO,
    REFERENCIANDO_INIT,
    REFERENCIANDO_CICLO,
    PRONTO,
    CICLO
};

typedef struct
{
    Estado estado = ESTADO_TESTE;
    Estado sub_estado = EMERGENCIA_TOP;
} Fsm;
Fsm fsm;

SemaphoreHandle_t mtx_ios;
SemaphoreHandle_t mutex_rs485;

AccelStepper motor(AccelStepper::DRIVER, PIN_PUL, PIN_DIR);
AccelStepper motor_espatula(AccelStepper::DRIVER, PIN_PUL_ESP, PIN_DIR_ESP);

ihmSunnytecMaster ihm;

TaskHandle_t h_eeprom;

extern HardwareSerial rs485;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// IO's:
uint8_t output_state = 0; // estado das saídas de uso geral. Usado na função updateIOs().
uint8_t input_state = 0;  // estado das entradas de uso geral. Usado na função updateIOs().
// IO's:

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Parâmetros:
// Menu:
int32_t produto = 1;

int32_t atrasoSensorProduto = 100;
int32_t atrasoImpressaoEtiqueta = 1000;
int32_t velocidadeLinearmmps = 150;
int32_t espacamentoProdutomm = 20;

int32_t posicaoBracoInicial = 10;
int32_t posicaoBracoAplicacao = 250;

int32_t tempoFinalizarAplicacao = 250;

int32_t contadorCiclo = 0;

int32_t rampa = 10;
int32_t statusIntertravamentoIn = INTERTRAVAMENTO_IN_OFF;
// Menu:

// Variáveis para os motores:
// Pulsos:
const int32_t pulsosEspatulaRecuoInit = 3000;
const int32_t pulsosEspatulaAvancoInit = 750;

const int32_t pulsosEspatulaAvanco = 2350;
const int32_t pulsosEspatulaRecuo = 3000;

const int32_t pulsosBracoMaximo = 60000;
const int32_t pulsosBracoForaSensor = 10000;

int32_t pulsosBracoEspacamento = 0;

int32_t pulsosBracoInicial = 0;
int32_t pulsosBracoAplicacao = 0;
int32_t pulsosBracoProduto = 0;

int32_t pulsosBracoEmergencia = 0;
// Pulsos:

// Posições:
int32_t posicaoEspatulaSensor = 0;
int32_t posicaoEspatulaReferencia = 0;
int32_t posicaoEspatulaCorrecao = 0;

int32_t posicaoBracoSensor = 0;
int32_t posicaoBracoReferencia = 0;
int32_t posicaoBracoCorrecao = 0;
int32_t posicaoBracoDeteccaoProduto = 0;

int32_t posicaoBracoProduto = 440;

int32_t posicaoBracoEmergencia = 462;
// Posições:
// Variáveis para os motores:

const int32_t pi = 3.14159265358979;
const int32_t raio = 20;
const int32_t subdivisao = 50;
const int32_t pulsosporVolta = 200;
const int32_t resolucao = round((pulsosporVolta * subdivisao) / (2 * pi * raio));

int32_t velocidadeLinearPulsos = 0;
int32_t velocidadeCiclommps = 0;
uint32_t aceleracaoLinearPulsos = 0;
int32_t pulsosRampa = 0;

int32_t contadorAbsoluto = 0;
const uint16_t quantidadeParaBackups = 100;

int16_t testeStatusImpressora = 0;
int32_t tempoRequestStatusImpressora = 15000;
String printerStatus = "1";

int16_t tempoLedStatus = 500;

int32_t tempoReinicioEspatula = 100;
int32_t tempoParaEstabilizarMotorBraco = 2500;

// Processo:
// Fases da fsm:
uint16_t fsm_emergencia = fase1;
uint16_t fsm_manutencao = fase1;

uint16_t fsm_referenciando_init = fase1;
uint16_t fsm_referenciando_init_espatula = fase1;
uint16_t fsm_referenciando_init_motor = fase1;

uint16_t fsm_referenciando_ciclo = fase1;
uint16_t fsm_referenciando_ciclo_espatula = fase1;
uint16_t fsm_referenciando_ciclo_motor = fase1;

uint16_t fsm_pronto_init = fase1;
uint16_t fsm_pronto_ciclo = fase1;

uint16_t fsm_ciclo = fase1;

uint16_t fsm_erro_aplicacao = fase1;
uint16_t fsm_erro_impressora = fase1;
uint16_t fsm_erro_intertravamento = fase1;

// Fases da fsm:
// Processo:

// Flag's:
bool flag_comandoPlay = false;
bool flag_statusImpressora = false;
bool flag_intertravamentoIn = true;
bool flag_emergencia = false;
bool flag_debugEnabled = false;
bool flag_restartDisplay = false;
bool flag_continuo = false;
bool flag_manutencao = false;
// Flag's:
// Parâmetros:
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Criando menu:
Menu menu_produto = Menu("Produto", PARAMETRO, &produto, " ", 1u, 1u, (unsigned)(EPR_maxProdutos));

Menu menu_atrasoSensorProduto = Menu("Atraso Produto", PARAMETRO, &atrasoSensorProduto, "ms", 10u, 10u, 5000u, &produto);
Menu menu_atrasoImpressaoEtiqueta = Menu("Atraso Imp Etiqueta", PARAMETRO, &atrasoImpressaoEtiqueta, "ms", 10u, 50u, 3000u, &produto);
Menu menu_velocidadeLinearmmps = Menu("Velocidade Braco", PARAMETRO, &velocidadeLinearmmps, "mm/s", 10u, 10u, 550u, &produto);

Menu menu_contador = Menu("Contador", READONLY, &contadorCiclo);

Menu menu_posicaoBracoInicial = Menu("Posicao Inicial", PARAMETRO_MANU, &posicaoBracoInicial, "mm", 1u, 0u, 400u);
Menu menu_posicaoBracoAplicacao = Menu("Posicao Aplicacao", PARAMETRO_MANU, &posicaoBracoAplicacao, "mm", 10u, 100u, 450u);
Menu menu_espacamentoProdutomm = Menu("Espacamento Produto", PARAMETRO_MANU, &espacamentoProdutomm, "mm", 1u, 20u, 200u);
Menu menu_tempoFinalizarAplicacao = Menu("Finalizar Aplicacao", PARAMETRO_MANU, &tempoFinalizarAplicacao, "ms", 10u, 20u, 500u);

Menu menu_rampa = Menu("Rampa", PARAMETRO_MANU, &rampa, "mm", 1u, 1u, 200u);
Menu menu_statusIntertravamentoIn = Menu("Intertravamento In", PARAMETRO_STRING, "     ON ou OFF      ");
// Criando menu:

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Prototypes:
void createTasks();

void t_requestStatusImpressoraZebra(void *p);
void t_receiveStatusImpressoraZebra(void *p);
void imprimirZebra();
void trataDadosImpressora(String);

void motorSetup();
void motorEnable();
void motorDisable();
void motorRun();

int checkSensorProduto();
int checkSensorHomeInit();
int checkSensorHome();
int checkSensorEspatulaInit();
int checkSensorEspatula();
int checkSensorAplicacao();

int checkBotaoStart();

void t_ihm(void *p);
bool checkBotaoCima();
bool checkBotaoBaixo();
bool checkBotaoEsquerda();
bool checkBotaoDireita();

void t_eeprom(void *p);
void restoreBackupParameters();
void loadProductFromEEPROM(uint16_t);
void presetEEPROM();

void t_emergencia(void *p);
void t_intretravamentoIN(void *p);
void updateIntertravamentoIn();

void t_manutencao(void *p);
void liberaMenusDeManutencao();
void bloqueiaMenusDeManutencao();

void t_io(void *p);
void resetBits(uint8_t);
void setBits(uint8_t);
void updateOutput(uint8_t);
void ligaOutput(uint8_t);
void desligaOutput(uint8_t);
void desligaTodosOutput();

void ventiladorConfig();
void ventiladorSetup(uint16_t, uint16_t, uint16_t);
void ventiladorAttachPin(uint16_t, uint16_t);
void ventiladorWrite(uint16_t, uint16_t);

void incrementaContador();

void piscaLedStatus();

void t_blink(void *p);

void t_debug(void *p);

void pin_mode(); // to do: remover essa funcao e usar a pinInitialization da lib esp32_v2.1
void ligaPrint();
void desligaPrint();
// Prototypes:

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void createTasks()
{
    // xTaskCreatePinnedToCore(t_ihm, "ihm task", 4096, NULL, PRIORITY_4, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_io, "io task", 2048, NULL, PRIORITY_3, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_emergencia, "emergencia task", 2048, NULL, PRIORITY_2, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_intretravamentoIN, "intertravamento in task", 2048, NULL, PRIORITY_2, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_manutencao, "manutencao task", 2048, NULL, PRIORITY_1, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_eeprom, "eeprom task", 8192, NULL, PRIORITY_1, &h_eeprom, CORE_0);
    // xTaskCreatePinnedToCore(t_receiveStatusImpressoraZebra, "resposta status impressora task", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreatePinnedToCore(t_blink, "blink task", 1024, NULL, PRIORITY_1, NULL, CORE_0);

    if (flag_debugEnabled)
        xTaskCreatePinnedToCore(t_debug, "Debug task", 2048, NULL, PRIORITY_1, NULL, CORE_0);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_requestStatusImpressoraZebra(void *p)
{
    xSemaphoreTake(mutex_rs485, portMAX_DELAY);
    ihm.statusImpressoraRS485();
    xSemaphoreGive(mutex_rs485);

    vTaskDelete(NULL);
}

void t_receiveStatusImpressoraZebra(void *p)
{
    delay(tempoRequestStatusImpressora);

    while (1)
    {
        if (rs485.available() > 0)
        {
            xSemaphoreTake(mutex_rs485, portMAX_DELAY);
            String frame = rs485.readString();
            xSemaphoreGive(mutex_rs485);
            trataDadosImpressora(frame);
        }
        delay(1000);
    }
}

void ligaPrint()
{
    digitalWrite(PIN_PRIN, LOW);
}

void desligaPrint()
{
    digitalWrite(PIN_PRIN, HIGH);
}

void imprimirZebra()
{
    ihm.imprimirRS485();
}

void playZebra()
{
    ihm.playRS485();
}

void trataDadosImpressora(String mensagemImpressora)
{
    // if (mensagemImpressora.compareTo(" ") == 0)
    //     Serial.print("Mensagem impressora: Vazia... ");

    testeStatusImpressora = mensagemImpressora.indexOf(printerStatus, 45);

    if (testeStatusImpressora != -1)
    {
        flag_statusImpressora = true;
        Serial.println(mensagemImpressora);
        flag_comandoPlay = true;
    }
    else if (testeStatusImpressora == -1)
    {
        flag_statusImpressora = false;
        Serial.println(mensagemImpressora);
        if (flag_comandoPlay)
        {
            playZebra();
            flag_comandoPlay = false;
        }
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void motorSetup()
{
    const int32_t velocidadeReferencia = 165;
    const int32_t velocidadeReferenciaEspatula = 1750;
    const int32_t aceleracaoReferenciaEspatula = 5000;

    pulsosRampa = resolucao * rampa;

    int32_t velocidadeLinearPulsosBracoInit = round(velocidadeReferencia * resolucao);
    int32_t aceleracaoLinearPulsosBracoInit = round(((velocidadeLinearPulsosBracoInit * velocidadeLinearPulsosBracoInit) / (2 * pulsosRampa)));

    motor.setMaxSpeed(velocidadeLinearPulsosBracoInit); // escolhe velocidade em que o motor vai trabalhar
    motor.setAcceleration(aceleracaoLinearPulsosBracoInit);
    motor.setPinsInverted(DIRECAO_HORA); // muda direção do motor

    motor_espatula.setMaxSpeed(velocidadeReferenciaEspatula);
    motor_espatula.setAcceleration(aceleracaoReferenciaEspatula);
    motor_espatula.setPinsInverted(DIRECAO_ANTIHORA);

    velocidadeCiclommps = velocidadeReferencia;
}

void motorEnable()
{
    setBits(PIN_ENABLE_MOTORES);
}

void motorDisable()
{
    resetBits(PIN_ENABLE_MOTORES);
}

void motorRun()
{
    const int32_t velocidadeEspatula = 8000;
    const int32_t aceleracaoEspatula = 80000;

    if (velocidadeCiclommps != velocidadeLinearmmps)
    {
        pulsosRampa = resolucao * rampa;

        motor_espatula.setMaxSpeed(velocidadeEspatula);
        motor_espatula.setAcceleration(aceleracaoEspatula);

        velocidadeLinearPulsos = round(velocidadeLinearmmps * resolucao);
        aceleracaoLinearPulsos = round(((velocidadeLinearPulsos * velocidadeLinearPulsos) / (2 * pulsosRampa)));

        motor.setMaxSpeed(velocidadeLinearPulsos);
        motor.setAcceleration(aceleracaoLinearPulsos);
        velocidadeCiclommps = velocidadeLinearmmps;

        Serial.print("Velocidade Braco: ");
        Serial.println(velocidadeLinearPulsos);

        Serial.print("Aceleracao: ");
        Serial.println(aceleracaoLinearPulsos);

        Serial.print("Resolucao: ");
        Serial.println(resolucao);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int checkSensorProduto()
{
    static bool flag_sp = 0;

    if (flag_sp == 0)
    {
        if (digitalRead(PIN_SENSOR_PRODUTO) == HIGH)
        {
            flag_sp = 1;
        }
    }
    else if (flag_sp == 1)
    {
        if (digitalRead(PIN_SENSOR_PRODUTO) == LOW)
        {
            flag_sp = 0;
            return 1;
        }
    }
    return 0;
}

int checkSensorHomeInit()
{
    if (digitalRead(PIN_SENSOR_HOME) == LOW)
    {
        return 1;
    }

    return 0;
}

int checkSensorHome()
{
    static bool flag_sh = 0;

    if (flag_sh == 0)
    {
        if (digitalRead(PIN_SENSOR_HOME) == HIGH)
        {
            flag_sh = 1;
        }
    }
    else if (flag_sh == 1)
    {
        if (digitalRead(PIN_SENSOR_HOME) == LOW)
        {
            flag_sh = 0;
            return 1;
        }
    }
    return 0;
}

int checkSensorEspatulaInit()
{
    if (digitalRead(PIN_SENSOR_ESPATULA) == LOW)
    {
        return 1;
    }

    return 0;
}

int checkSensorEspatula()
{
    static bool flag_se = 0;

    if (flag_se == 0)
    {
        if (digitalRead(PIN_SENSOR_ESPATULA) == LOW)
        {
            flag_se = 1;
        }
    }
    else if (flag_se == 1)
    {
        if (digitalRead(PIN_SENSOR_ESPATULA) == HIGH)
        {
            flag_se = 0;
            return 1;
        }
    }
    return 0;
}

int checkSensorAplicacao()
{
    static bool flag_sa = 0;

    if (flag_sa == 0)
    {
        if (digitalRead(PIN_SENSOR_APLICACAO) == HIGH)
        {
            flag_sa = 1;
        }
    }
    else if (flag_sa == 1)
    {
        if (digitalRead(PIN_SENSOR_APLICACAO) == LOW)
        {
            flag_sa = 0;
            return 1;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int checkBotaoStart()
{
    static bool flag_c_start = false;

    const uint8_t start = bit(BUTTON_START);

    if (flag_c_start == 0)
    {
        if ((input_state & start) == start)
        {
            flag_c_start = 1;
        }
    }
    else if (flag_c_start == 1)
    {
        if ((input_state & start) == 0)
        {
            flag_c_start = 0;
            return 1;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Checa se os botoes foram pressionados e já atualiza o display
void t_ihm(void *p)
{
    mutex_rs485 = xSemaphoreCreateMutex();

    uint32_t timer_display = 0;
    const uint32_t resetDisplay = 120000;
    String cont_str = "Contador: ";

    cont_str.concat(contadorAbsoluto);
    ihm.configDefaultMsg(cont_str);
    ihm.configDefaultMsg2("PRINT APPLY LINEAR");

    xSemaphoreTake(mutex_rs485, portMAX_DELAY);
    ihm.setup();
    xSemaphoreGive(mutex_rs485);

    delay(3000);

    ihm.addMenuToIndex(&menu_produto);

    ihm.addMenuToIndex(&menu_atrasoSensorProduto);
    ihm.addMenuToIndex(&menu_atrasoImpressaoEtiqueta);
    ihm.addMenuToIndex(&menu_velocidadeLinearmmps);

    ihm.addMenuToIndex(&menu_contador);

    ihm.focus(&menu_produto); // Direciona a ihm para iniciar nesse menu

    while (1)
    {
        if (fsm.estado == PARADA_EMERGENCIA || fsm.sub_estado == PRONTO)
        {
            xSemaphoreTake(mutex_rs485, portMAX_DELAY);
            if (checkBotaoCima())
            {
                Menu *checkMenu = ihm.getMenu();
                if (checkMenu == &menu_produto)
                {
                    menu_produto.addVar(MAIS);
                    loadProductFromEEPROM(produto);
                }
                else if (checkMenu == &menu_statusIntertravamentoIn)
                    updateIntertravamentoIn();
                else
                {
                    checkMenu->addVar(MAIS);
                }

                timer_display = millis();
            }
            else if (checkBotaoBaixo())
            {
                Menu *checkMenu = ihm.getMenu();
                if (checkMenu == &menu_produto)
                {
                    menu_produto.addVar(MENOS);
                    loadProductFromEEPROM(produto);
                }
                else if (checkMenu == &menu_statusIntertravamentoIn)
                    updateIntertravamentoIn();
                else
                {
                    checkMenu->addVar(MENOS);
                }

                timer_display = millis();
            }
            else if (checkBotaoEsquerda())
            {
                ihm.changeMenu(PREVIOUS);
                timer_display = millis();
            }

            else if (checkBotaoDireita())
            {
                ihm.changeMenu(NEXT);
                timer_display = millis();
            }

            xSemaphoreGive(mutex_rs485);
        }

        if (millis() - timer_display >= resetDisplay)
        {
            flag_restartDisplay = true;
            timer_display = millis();
        }
        if (flag_restartDisplay)
        {
            Wire.flush();
            ihm.displayFull();

            flag_restartDisplay = false;
        }

        ihm.task();

        delay(25);
    }
}

bool checkBotaoCima()
{
    static bool flag_c_cima = false;
    const uint8_t cima = bit(BUTTON_CIMA);

    static uint32_t timer_hold = 0;
    const uint16_t timeout_hold = 1500; // ms

    if (flag_c_cima == 0)
    {
        if ((input_state & cima) == 0)
        {
            flag_c_cima = 1;
        }
        else
        {
            if (millis() - timer_hold >= timeout_hold)
            {
                return 1;
            }
        }
    }
    else if (flag_c_cima == 1)
    {
        if ((input_state & cima) == cima)
        {
            flag_c_cima = 0;
            timer_hold = millis();
            return 1;
        }
    }
    return 0;
}

bool checkBotaoBaixo()
{
    static bool flag_c_baixo = false;
    const uint8_t baixo = bit(BUTTON_BAIXO);

    static uint32_t timer_hold = 0;
    const uint16_t timeout_hold = 1500; // ms

    if (flag_c_baixo == 0)
    {
        if ((input_state & baixo) == 0)
        {
            flag_c_baixo = 1;
        }
        else
        {
            if (millis() - timer_hold >= timeout_hold)
            {
                return 1;
            }
        }
    }
    else if (flag_c_baixo == 1)
    {
        if ((input_state & baixo) == baixo)
        {
            flag_c_baixo = 0;
            timer_hold = millis();
            return 1;
        }
    }
    return 0;
}

bool checkBotaoEsquerda()
{
    static bool flag_c_esquerda = false;
    const uint8_t esquerda = bit(BUTTON_ESQUERDA);

    if (flag_c_esquerda == 0)
    {
        if ((input_state & esquerda) == esquerda)
        {
            flag_c_esquerda = 1;
        }
    }
    else if (flag_c_esquerda == 1)
    {
        if ((input_state & esquerda) == 0)
        {
            flag_c_esquerda = 0;
            return 1;
        }
    }
    return 0;
}

bool checkBotaoDireita()
{
    static bool flag_c_direita = false;
    const uint8_t direita = bit(BUTTON_DIREITA);

    if (flag_c_direita == 0)
    {
        if ((input_state & direita) == direita)
        {
            flag_c_direita = 1;
        }
    }
    else if (flag_c_direita == 1)
    {
        if ((input_state & direita) == 0)
        {
            flag_c_direita = 0;
            return 1;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_eeprom(void *p)
{
    const uint16_t intervaloEntreBackups = 5000; // ms

    while (1)
    {
        EEPROM.put(EPR_produto, produto);

        EEPROM.put(EPR_pulsosBracoInicial, posicaoBracoInicial);
        EEPROM.put(EPR_pulsosBracoAplicacao, posicaoBracoAplicacao);
        EEPROM.put(EPR_espacamentoProdutomm, espacamentoProdutomm);
        EEPROM.put(EPR_tempoFinalizarAplicacao, tempoFinalizarAplicacao);
        EEPROM.put(EPR_rampa, rampa);
        EEPROM.put(EPR_statusIntertravamentoIn, statusIntertravamentoIn);

        EEPROM.put(EPR_offsetEspecificos + (produto - 1) * EPR_offsetProduto + EPR_atrasoSensorProduto, atrasoSensorProduto);
        EEPROM.put(EPR_offsetEspecificos + (produto - 1) * EPR_offsetProduto + EPR_atrasoImpressaoEtiqueta, atrasoImpressaoEtiqueta);
        EEPROM.put(EPR_offsetEspecificos + (produto - 1) * EPR_offsetProduto + EPR_velocidadeLinearmmps, velocidadeLinearmmps);

        if ((contadorAbsoluto % quantidadeParaBackups) == 0)
            EEPROM.put(EPR_contadorAbsoluto, contadorAbsoluto);

        EEPROM.commit();

        delay(intervaloEntreBackups);
    }
}

/* Salva os parâmetros do equipamento de tempos em tempos os primeiros endereços
são reservados para parâmetros não-específicos, os demais endereços são separados
por produto/receita o produto é exibido para o usuário no display como 1 a 10,
mas o software sempre trata ele como produto -1, para conter o zero. */

// Recupera os parâmetros salvos na eeprom
void restoreBackupParameters()
{
    EEPROM.get(EPR_produto, produto);

    EEPROM.get(EPR_pulsosBracoInicial, posicaoBracoInicial);
    EEPROM.get(EPR_pulsosBracoAplicacao, posicaoBracoAplicacao);
    EEPROM.get(EPR_espacamentoProdutomm, espacamentoProdutomm);
    EEPROM.get(EPR_tempoFinalizarAplicacao, tempoFinalizarAplicacao);
    EEPROM.get(EPR_rampa, rampa);
    EEPROM.get(EPR_statusIntertravamentoIn, statusIntertravamentoIn);
    EEPROM.get(EPR_contadorAbsoluto, contadorAbsoluto);

    loadProductFromEEPROM(produto);
}

// essa função é chamada toda vez que o usuário usa o display para trocar de produto
// os parâmetros de cada produto são então carregados nas suas devidas variáveis globais.
void loadProductFromEEPROM(uint16_t prod)
{
    EEPROM.get(EPR_offsetEspecificos + (prod - 1) * EPR_offsetProduto + EPR_atrasoSensorProduto, atrasoSensorProduto);
    EEPROM.get(EPR_offsetEspecificos + (prod - 1) * EPR_offsetProduto + EPR_atrasoImpressaoEtiqueta, atrasoImpressaoEtiqueta);
    EEPROM.get(EPR_offsetEspecificos + (prod - 1) * EPR_offsetProduto + EPR_velocidadeLinearmmps, velocidadeLinearmmps);
}

// use essa função para restar os valores de todos os produtos.
// para utilizá-la, chame ela na função setup(), e não use a função loadFromEEPROM()
// o valor escolhido na inicialização das variáveis é o que será salvo nos presets.
void presetEEPROM()
{
    for (int i = 0; i < EPR_maxProdutos; i++)
    {
        EEPROM.put(EPR_offsetEspecificos + i * EPR_offsetProduto + EPR_atrasoSensorProduto, atrasoSensorProduto);
        EEPROM.put(EPR_offsetEspecificos + i * EPR_offsetProduto + EPR_atrasoImpressaoEtiqueta, atrasoImpressaoEtiqueta);
        EEPROM.put(EPR_offsetEspecificos + i * EPR_offsetProduto + EPR_velocidadeLinearmmps, velocidadeLinearmmps);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_emergencia(void *p)
{
    while (1)
    {
        flag_emergencia = digitalRead(PIN_EMERGENCIA); // to do:
        delay(100);
    }
}

void t_intretravamentoIN(void *p)
{
    static uint32_t timer_hold_on = 0;
    static uint32_t timer_hold_off = 0;
    const uint16_t timeout_hold = 750; // ms

    bool flag_intertravamentoIn_Hold_On = false;

    while (1)
    {
        if (statusIntertravamentoIn == INTERTRAVAMENTO_IN_ON)
        {
            flag_intertravamentoIn_Hold_On = !(input_state & bit(INTERTRAVAMENTO_IN_1));

            if (flag_intertravamentoIn_Hold_On == HIGH)
            {
                if (millis() - (timer_hold_on) >= timeout_hold)
                {
                    flag_intertravamentoIn = !(input_state & bit(INTERTRAVAMENTO_IN_1));
                    timer_hold_on = millis();
                    timer_hold_off = millis();
                }
                else
                {
                    timer_hold_off = millis();
                }
            }
            else if (flag_intertravamentoIn_Hold_On == LOW)
            {
                if (millis() - (timer_hold_off) >= timeout_hold)
                {
                    flag_intertravamentoIn = !(input_state & bit(INTERTRAVAMENTO_IN_1));
                    timer_hold_off = millis();
                    timer_hold_on = millis();
                }
                else
                {
                    timer_hold_on = millis();
                }
            }
        }

        else
        {
            statusIntertravamentoIn = INTERTRAVAMENTO_IN_OFF;
            menu_statusIntertravamentoIn.changeMsg(F("OFF"));
        }

        delay(500);
    }
}

void updateIntertravamentoIn()
{
    if (statusIntertravamentoIn == INTERTRAVAMENTO_IN_OFF)
    {
        statusIntertravamentoIn = INTERTRAVAMENTO_IN_ON;
        menu_statusIntertravamentoIn.changeMsg(F("ON"));
    }
    else
    {
        statusIntertravamentoIn = INTERTRAVAMENTO_IN_OFF;
        menu_statusIntertravamentoIn.changeMsg(F("OFF"));
    }

    ihm.signalVariableChange();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_manutencao(void *p)
{
    static uint32_t timer_manutencao = 0;
    const uint16_t tempoParaAtivarMenuManutencao = 3000;

    while (1)
    {
        if (flag_manutencao)
        {
            if ((input_state & bit(BUTTON_DIREITA)) == bit(BUTTON_DIREITA)) // botao direita
            {
                if (millis() - timer_manutencao >= tempoParaAtivarMenuManutencao)
                {
                    fsm.sub_estado = MANUTENCAO;
                    fsm_emergencia = fase1;
                }
            }
            else
            {
                timer_manutencao = millis();
            }
        }
        delay(100);
    }
}

void liberaMenusDeManutencao()
{
    ihm.addMenuToIndex(&menu_posicaoBracoInicial);
    ihm.addMenuToIndex(&menu_posicaoBracoAplicacao);
    ihm.addMenuToIndex(&menu_espacamentoProdutomm);
    ihm.addMenuToIndex(&menu_tempoFinalizarAplicacao);
    ihm.addMenuToIndex(&menu_rampa);
    ihm.addMenuToIndex(&menu_statusIntertravamentoIn);
}

void bloqueiaMenusDeManutencao()
{
    ihm.removeMenuFromIndex();
    ihm.removeMenuFromIndex();
    ihm.removeMenuFromIndex();
    ihm.removeMenuFromIndex();
    ihm.removeMenuFromIndex();
    ihm.removeMenuFromIndex();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Thread que controla as entradas e saídas gerais
void t_io(void *p)
{
    while (1)
    {
        if (xSemaphoreTake(mtx_ios, pdMS_TO_TICKS(5))) // mutex output_state
        {
            digitalWrite(PIN_IO_LATCH, LOW);
            delayMicroseconds(50);
            digitalWrite(PIN_IO_LATCH, HIGH);
            uint8_t input_buffer = 0;
            for (int i = 0; i < 8; i++)
            {
                input_buffer = input_buffer | (digitalRead(PIN_INPUT_DATA) << (7 - i));
                digitalWrite(PIN_OUTPUT_DATA, !!(output_state & (1 << (7 - i))));
                digitalWrite(PIN_IO_CLOCK, HIGH);
                delayMicroseconds(20);
                digitalWrite(PIN_IO_CLOCK, LOW);
                delayMicroseconds(20);
            }
            input_state = ~input_buffer;
            digitalWrite(PIN_IO_LATCH, LOW);
            xSemaphoreGive(mtx_ios); // mutex output_state
        }
        delay(3);
    }
}

void resetBits(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mtx_ios, pdMS_TO_TICKS(1))) // mutex output_state
    {
        output_state &= ~bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mtx_ios); // mutex output_state
    }
    else
    {
        Serial.println("mtx reset");
    }
}

void setBits(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mtx_ios, pdMS_TO_TICKS(1))) // mutex output_state
    {
        output_state |= bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mtx_ios); // mutex output_state
    }
    else
    {
        Serial.println("mtx set");
    }
}

void updateOutput(uint8_t outputByte)
{
    digitalWrite(PIN_IO_LATCH, LOW);
    delayMicroseconds(50);
    digitalWrite(PIN_IO_LATCH, HIGH);
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(PIN_OUTPUT_DATA, !!(outputByte & (1 << (7 - i))));
        digitalWrite(PIN_IO_CLOCK, HIGH);
        delayMicroseconds(20);
        digitalWrite(PIN_IO_CLOCK, LOW);
        delayMicroseconds(20);
    }
    digitalWrite(PIN_IO_LATCH, LOW);
    output_state = outputByte;
}

void ligaOutput(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mtx_ios, pdMS_TO_TICKS(1))) // Mutex output_state
    {
        output_state &= ~bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mtx_ios); // Mutex output_state
    }
    else
    {
        Serial.println("mtx reset");
    }
}

void desligaOutput(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mtx_ios, pdMS_TO_TICKS(1))) // Mutex output_state
    {
        output_state |= bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mtx_ios); // Mutex output_state
    }
    else
    {
        Serial.println("mtx set");
    }
}

void desligaTodosOutput()
{
    updateOutput(bit(DO4) | bit(DO5) | bit(DO6) | bit(DO7) | bit(DO8) | bit(RLO1) | bit(RLO2));
    digitalWrite(PIN_DO1, HIGH);
    digitalWrite(PIN_DO2, HIGH);
    digitalWrite(PIN_HSDO1, LOW);
    digitalWrite(PIN_HSDO2, LOW);
    digitalWrite(PIN_HSDO3, LOW);
    digitalWrite(PIN_HSDO4, LOW);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void ventiladorConfig()
{
    const uint16_t frequencia = 1000;
    const uint16_t resolucao = 8;

    ventiladorSetup(VENTILADOR_CANAL, frequencia, resolucao);
    ventiladorAttachPin(PWM_VENTILADOR, VENTILADOR_CANAL);
}

void ventiladorSetup(uint16_t canal, uint16_t freq, uint16_t resolucao)
{
    ledcSetup(canal, freq, resolucao);
}

void ventiladorAttachPin(uint16_t pino, uint16_t canal)
{
    ledcAttachPin(pino, canal);
}

void ventiladorWrite(uint16_t canal, uint16_t intensidade)
{
    uint16_t dutyCicle = map(intensidade, 0, 100, 0, 255);

    ledcWrite(canal, dutyCicle);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void incrementaContador()
{
    contadorCiclo++;
    contadorAbsoluto++;
    if ((contadorAbsoluto % quantidadeParaBackups) == 0)
        vTaskResume(h_eeprom);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void piscaLedStatus()
{
    static uint32_t timer_led = 0;

    setBits(LED_STATUS);
    if (millis() - timer_led >= tempoLedStatus)
    {
        resetBits(LED_STATUS);
        timer_led = millis();
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_blink(void *p)
{
    uint16_t interval = 400;

    while (1)
    {
        GPIO.out_w1ts = (1 << PIN_STATUS);
        delay(interval / 2);
        GPIO.out_w1tc = (1 << PIN_STATUS);
        delay(interval / 2);
        GPIO.out_w1ts = (1 << PIN_STATUS);
        delay(interval / 2);
        GPIO.out_w1tc = (1 << PIN_STATUS);
        delay(interval * 2.5);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_debug(void *p)
{
    while (1)
    {
        Serial.print("SP: ");
        Serial.println(digitalRead(PIN_SENSOR_PRODUTO));
        Serial.print("SH: ");
        Serial.println(digitalRead(PIN_SENSOR_HOME));
        Serial.print("SA: ");
        Serial.println(digitalRead(PIN_SENSOR_APLICACAO));
        Serial.print("SE: ");
        Serial.println(digitalRead(PIN_SENSOR_ESPATULA));

        delay(2000);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void pin_mode()
{
    // Outputs:
    pinMode(PIN_STATUS, OUTPUT);

    pinMode(PIN_OUTPUT_DATA, OUTPUT);
    pinMode(PIN_IO_CLOCK, OUTPUT);
    pinMode(PIN_IO_LATCH, OUTPUT);

    pinMode(PIN_DIR, OUTPUT);
    pinMode(PWM_VENTILADOR, OUTPUT);

    pinMode(PIN_PUL, OUTPUT);
    pinMode(PIN_PUL_ESP, OUTPUT);
    pinMode(PIN_DIR_ESP, OUTPUT);
    pinMode(PIN_HSDO4, OUTPUT);

    pinMode(PIN_RS485_EN, OUTPUT);
    // Outputs:

    //  Inputs:
    pinMode(PIN_INPUT_DATA, INPUT);

    pinMode(PIN_SENSOR_PRODUTO, INPUT);
    pinMode(PIN_SENSOR_HOME, INPUT);
    pinMode(PIN_SENSOR_APLICACAO, INPUT);
    pinMode(PIN_SENSOR_ESPATULA, INPUT);

    //  Inputs:
}
// Functions:
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#endif