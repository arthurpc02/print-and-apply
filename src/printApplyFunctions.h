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
#include <ihmSunnytecMaster_v2.0.h>
#include <checkSensorPulse.h>
#include <extendedIOs.h>

// cada falha do equipamento é armazenada em um bit diferente do faultRegister
#define FALHA_EMERGENCIA (1 << 0)
#define FALHA_IMPRESSAO (1 << 1)
#define FALHA_SENSORES (1 << 2)
#define FALHA_IHM (1 << 3)
#define FALHA_IMPRESSORA (1 << 4)
#define FALHA_PORTA_ABERTA (1 << 5)
#define FALHA_APLICACAO (1 << 6)
#define FALHA_DESCONHECIDA (1 << 7)

enum Estado
{
    ESTADO_EMERGENCIA,
    ESTADO_STOP,
    ESTADO_DESATIVADO,
    ESTADO_REFERENCIANDO,
    ESTADO_APLICACAO,
    ESTADO_POSICIONANDO,
    ESTADO_FALHA,
    ESTADO_TESTE_DE_IMPRESSAO,
    ESTADO_TESTE_DO_BRACO,
    ESTADO_TESTE_DO_VENTILADOR,
    // Estados:
    PARADA_EMERGENCIA_OLD,
    ATIVO_OLD,
    ERRO_OLD,
    // Sub-estados:
    EMERGENCIA_TOP_OLD,
    MANUTENCAO_OLD,
    REFERENCIANDO_INIT_OLD,
    REFERENCIANDO_CICLO_OLD,
    PRONTO_OLD,
    CICLO_OLD,
} fsm;
uint16_t fsm_substate = fase1;

enum Evento
{
    EVT_NENHUM,
    EVT_FALHA,
    EVT_SEM_FALHAS,
    EVT_PARADA_EMERGENCIA,
    EVT_TESTE,
    EVT_PLAY_PAUSE,
    EVT_START,
    EVT_HOLD_PLAY_PAUSE,
    EVT_IMPRESSAO_CONCLUIDA,
};

typedef struct
{
    Estado estado = ESTADO_DESATIVADO;
    Estado sub_estado = EMERGENCIA_TOP_OLD;
} Fsm;
Fsm fsm_old;

SemaphoreHandle_t mutex_ios;
SemaphoreHandle_t mutex_rs485;
SemaphoreHandle_t mutex_fault; // controla o acesso à variável faultRegister, que é utilizada por várias threads
QueueHandle_t eventQueue;      // os eventos são armazenados em uma fila

AccelStepper braco(AccelStepper::DRIVER, PIN_PUL_BRACO, PIN_DIR_BRACO);
AccelStepper rebobinador(AccelStepper::DRIVER, PIN_PUL_REBOBINADOR, PIN_DIR_REBOBINADOR); // na verdade o DIR do rebobinador não está conecta. Então defini um pino que não está sendo utilizado.

TaskHandle_t h_eeprom;
TaskHandle_t h_botoesIhm;

ihmSunnytecMaster ihm{protocoloIhm{PIN_RS485_RX, PIN_RS485_TX, PIN_RS485_EN}};
extendedIOs extIOs = extendedIOs(PIN_IO_CLOCK, PIN_IO_LATCH, PIN_INPUT_DATA, PIN_OUTPUT_DATA);
checkSensorPulse sensorDeProdutoOuStart = checkSensorPulse(PIN_SENSOR_PRODUTO, 1);
checkSensorPulse sinalPrintEnd = checkSensorPulse(PIN_PREND, 1);
checkSensorPulse sensorAplicacao = checkSensorPulse(PIN_SENSOR_APLICACAO, 1);
checkSensorPulse sensorHome = checkSensorPulse(PIN_SENSOR_HOME, 1);
checkSensorPulse sinalImpressoraOnline = checkSensorPulse(PIN_IMPRESSORA_ONLINE, 1);

extern HardwareSerial rs485;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// IO's:
uint8_t output_state = 0; // estado das saídas de uso geral. Usado na função updateIOs().
uint8_t input_state = 0;  // estado das entradas de uso geral. Usado na função updateIOs().

uint16_t quantidadeDeMenusDeManutencao = 1;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Parâmetros:

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

// Variáveis para os motores:

const int32_t pi = 3.14159265358979;
const int32_t raio = 20;
const int32_t subdivisao = 50;
const int32_t pulsosporVolta = 200;
// const int32_t resolucao = round((pulsosporVolta * subdivisao) / (2 * pi * raio));

int32_t velocidadeLinearPulsos = 0;
int32_t velocidadeCiclommps = 0;
uint32_t aceleracaoLinearPulsos = 0;
int32_t pulsosRampa = 0;

const uint16_t quantidadeParaBackups = 100;

int16_t testeStatusImpressora = 0;
int32_t tempoRequestStatusImpressora = 15000;
String printerStatus = "1";

int16_t tempoLedStatus = 500;

int32_t tempoReinicioEspatula = 100;
int32_t tempoParaEstabilizarMotorBraco = 2500;

// Menu:
int32_t atrasoImpressaoEtiqueta = 1000;

int32_t posicaoBracoInicial = 10;
int32_t posicaoBracoAplicacao = 250;

int32_t statusIntertravamentoIn = INTERTRAVAMENTO_IN_OFF;

int32_t faultRegister = 0; // o fault byte armazena as falhas da máquina como se fosse um registrador.
                           // a vantagem de utilizar o faultRegister, é que é possível ter mais de uma falha ao mesmo tempo.

// parâmetros comuns:
int32_t contadorDeCiclos = 0;
int32_t produto = 1;
int32_t atrasoSensorProduto = 1000; // ms
int32_t posicaoDeAguardarProduto_dcmm = 1800;
int32_t distanciaProduto_dcmm = 750;
int32_t velocidadeDeTrabalho_dcmm = 1500;
// to do: trocar os 'dcmms' dos nomes da variaveis para 'dcmm' mesmo

// parâmetros manutenção:
int32_t tempoFinalizarAplicacao = 250;
int32_t posicaoLimite_dcmm = 4200;
int32_t posicaoDePegarEtiqueta_dcmm = 430;
int32_t posicaoDeRepouso_dcmm = 1250;
int32_t velocidadeDeReferenciacao_dcmm = 2200;
int32_t rampa_dcmm = 80;
int32_t flag_simulaEtiqueta = false;
int32_t velocidadeRebobinador = 9600;
int32_t aceleracaoRebobinador = 12000;
int32_t habilitaPortasDeSeguranca = 1;
int32_t potenciaVentilador = 50; // por centagem
int32_t contadorTotal = 0; // to do: mudar nome para contadorTotal

// parâmetros de instalação (só podem ser alterados na compilação do software):
const int32_t tamanhoMaximoDoBraco_dcmm = 4450;
const uint32_t rebobinador_ppr = 3200; // pulsos/revolucao

const float resolucaoNaCalibracao = 20.4803; // steps/dcmm
const uint32_t pprNaCalibracao = 25000;      // pulsos/revolucao

const uint32_t braco_ppr = 3200;                                             // pulsos/revolucao
const float resolucao = braco_ppr * resolucaoNaCalibracao / pprNaCalibracao; // steps/dcmm

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

// Flags:
bool flag_referenciou = false;
bool flag_cicloEmAndamento = false;

bool flag_comandoPlay = false;
bool flag_statusImpressora = false;
bool flag_intertravamentoIn = true;
bool flag_emergencia = false;
bool flag_debugEnabled = true;
bool flag_restartDisplay = false;
bool flag_continuo = false;
bool flag_manutencao = false;
bool flag_habilitaConfiguracaoPelaIhm = true; // se true, todos os botões da ihm serão processados. Se false, apenas play/stop serão processados.

// Flags:
// Parâmetros:
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Menu menu_contadorDeCiclos = Menu("Contador", READONLY, &contadorDeCiclos);
Menu menu_produto = Menu("Produto", PARAMETRO, &produto, " ", 1u, 1u, (unsigned)(EPR_maxProdutos));
Menu menu_atrasoSensorProduto = Menu("Atraso Produto", PARAMETRO, &atrasoSensorProduto, "ms", 10u, 10u, 5000u, &produto);
Menu menu_posicaoDeAguardarProduto_dcmm = Menu("Pos Aguarda Produto", PARAMETRO, &posicaoDeAguardarProduto_dcmm, "mm", 10, 10, tamanhoMaximoDoBraco_dcmm, &produto, 1);
Menu menu_distanciaProduto_dcmm = Menu("Distancia Produto", PARAMETRO, &distanciaProduto_dcmm, "mm", 10u, 10u, tamanhoMaximoDoBraco_dcmm, &produto, 1);
Menu menu_velocidadeDeTrabalho_dcmm = Menu("Velocidade Aplicacao", PARAMETRO, &velocidadeDeTrabalho_dcmm, "mm/s", 10u, 100u, 15000u, &produto, 1);

// manutencao:
// to do: menu de falhas.
Menu menu_simulaEtiqueta = Menu("Simula Etiqueta", PARAMETRO, &flag_simulaEtiqueta, " ", 1u, 0u, 1u, NULL);
Menu menu_habilitaPortasDeSeguranca = Menu("Portas de Seguranca", PARAMETRO, &habilitaPortasDeSeguranca, " ", 1u, 0u, 1u, NULL);
Menu menu_tempoFinalizarAplicacao = Menu("Finalizar Aplicacao", PARAMETRO, &tempoFinalizarAplicacao, "ms", 10u, 20u, 500u);
Menu menu_posicaoDePegarEtiqueta_dcmm = Menu("Pos Pega Etiqueta", PARAMETRO, &posicaoDePegarEtiqueta_dcmm, "mm", 5, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_posicaoLimite_dcmm = Menu("Pos Limite", PARAMETRO, &posicaoLimite_dcmm, "mm", 10, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_posicaoDeRepouso_dcmm = Menu("Pos Repouso", PARAMETRO, &posicaoDeRepouso_dcmm, "mm", 10, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_velocidadeDeReferenciacao_dcmm = Menu("Veloc Referenciacao", PARAMETRO, &velocidadeDeReferenciacao_dcmm, "mm/s", 10u, 100u, 15000u, NULL, 1);
Menu menu_rampa_dcmm = Menu("Rampa", PARAMETRO, &rampa_dcmm, "mm", 5u, 10u, 500u, NULL, 1);
Menu menu_contadorTotal = Menu("Contador Total", READONLY, &contadorTotal, " ");
Menu menu_velocidadeRebobinador = Menu("Veloc Rebobinador", PARAMETRO, &velocidadeRebobinador, "pulsos", 100u, 1000u, 50000u, NULL);
Menu menu_aceleracaoRebobinador = Menu("Acel Rebobinador", PARAMETRO, &aceleracaoRebobinador, "pulsos", 100u, 1000u, 50000u, NULL);

Menu menu_posicaoBracoInicial = Menu("Posicao Inicial", PARAMETRO_MANU, &posicaoBracoInicial, "mm", 1u, 0u, 400u);
Menu menu_posicaoBracoAplicacao = Menu("Posicao Aplicacao", PARAMETRO_MANU, &posicaoBracoAplicacao, "mm", 10u, 100u, 450u);
Menu menu_statusIntertravamentoIn = Menu("Intertravamento In", PARAMETRO_STRING, "     ON ou OFF      ");
Menu menu_atrasoImpressaoEtiqueta = Menu("Atraso Imp Etiqueta", PARAMETRO, &atrasoImpressaoEtiqueta, "ms", 10u, 50u, 3000u, &produto);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Prototypes:
void createTasks();

void t_requestStatusImpressoraZebra(void *p);
void t_receiveStatusImpressoraZebra(void *p);
void imprimirZebra();
void trataDadosImpressora(String);

void motorSetup();
void habilitaMotoresEAguardaEstabilizar();
void desabilitaMotores();
void habilitaMotores();
void motorRun();

int checkSensorProduto();
int checkSensorHomeInit();
int checkSensorHome();
int checkSensorEspatulaInit();
int checkSensorEspatula();
int checkSensorAplicacao();

int checkBotaoStart();

void t_ihm_old(void *p);
bool checkBotaoCima();
bool checkBotaoBaixo();
bool checkBotaoEsquerda();
bool checkBotaoDireita();

void saveParametersToEEPROM();
void saveProdutoToEEPROM(int16_t _produto);
void loadParametersFromEEPROM();
void loadProdutoFromEEPROM(int16_t);
void presetEEPROM();
void salvaContadorNaEEPROM();
void t_eeprom(void *p);

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
void desligaTodosOutputs();

void ventiladorSetup();
void ventiladorWrite(uint16_t, uint16_t);
void ligaVentilador();
void desligaVentilador();

void piscaLedStatus();

void imprimeEtiqueta();
void ligaPrint();
void desligaPrint();
void ligaReprint();
void desligaReprint();

bool emCimaDoSensorHome();
bool sensorDeAplicacaoDetectouProduto();

void t_blink(void *p);
void t_debug(void *p);
void t_printEtiqueta(void *);
void t_simulaPrintEtiqueta(void *);
void t_rebobina(void *);

void t_ihm(void *);
void t_botoesIhm(void *);
void liberaMenusDaIhm();
void liberaMenusDeManutencao();
void voltaParaPrimeiroMenu();
void incrementaContadores();
void habilitaConfiguracaoPelaIhm();
void desabilitaConfiguracaoPelaIhm();

void enviaEvento(Evento event);
Evento recebeEventos();
void changeFsmState(Estado estado);

float dcmm_to_steps(int32_t _dcmm);
int32_t steps_to_dcmm(float _steps);
void braco_moveTo(int32_t _dcmm);
void braco_move(int32_t _dcmm);
void braco_setup(int32_t, int32_t);
void rebobinador_setup(int32_t, int32_t);
void calculaVelocidadeEmSteps(int32_t _velocidade_dcmmPorS);
void calculaRampaEmSteps(int32_t _velocidade_dcmmPorS, int32_t _rampa_dcmm);
int32_t braco_getCurrentPositionInDcmm();

void clearAllFaults();
void updateFault(int16_t _faultCode, bool _faultState);
void setFault(int16_t _faultCode);
void clearFault(int16_t _faultCode);

void torre_ligaLuzVermelha();
void torre_ligaLuzVerde();

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void createTasks()
{
    // xTaskCreatePinnedToCore(t_ihm_old, "ihm task", 4096, NULL, PRIORITY_4, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_io, "io task", 2048, NULL, PRIORITY_3, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_intretravamentoIN, "intertravamento in task", 2048, NULL, PRIORITY_2, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_manutencao, "manutencao task", 2048, NULL, PRIORITY_1, NULL, CORE_0);
    // xTaskCreatePinnedToCore(t_receiveStatusImpressoraZebra, "resposta status impressora task", 1024, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreatePinnedToCore(t_eeprom, "eeprom task", 8192, NULL, PRIORITY_1, &h_eeprom, CORE_0);
    xTaskCreatePinnedToCore(t_ihm, "ihm task", 4096, NULL, PRIORITY_3, NULL, CORE_0);
    xTaskCreatePinnedToCore(t_botoesIhm, "botoesIhm task", 4096, NULL, PRIORITY_3, &h_botoesIhm, CORE_0);
    xTaskCreatePinnedToCore(t_emergencia, "emergencia task", 2048, NULL, PRIORITY_1, NULL, CORE_0);
    xTaskCreatePinnedToCore(t_blink, "blink task", 1024, NULL, PRIORITY_1, NULL, CORE_0);

    if (flag_debugEnabled)
        xTaskCreatePinnedToCore(t_debug, "Debug task", 2048, NULL, PRIORITY_1, NULL, CORE_0);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_ihm(void *p)
{
    ihm.configDefaultMsg("   PRINT & APPLY");
    ihm.configDefaultMsg2((String)contadorTotal);
    xSemaphoreTake(mutex_rs485, portMAX_DELAY);
    ihm.setup();
    ihm.desligaLEDvermelho();
    ihm.desligaLEDverde();
    xSemaphoreGive(mutex_rs485);

    delay(3000);

    liberaMenusDaIhm();
    ihm.goToMenu(&menu_contadorDeCiclos);

    while (1)
    {
        xSemaphoreTake(mutex_rs485, portMAX_DELAY);
        ihm.checkAndUpdateScreen();
        xSemaphoreGive(mutex_rs485);
        delay(100);
    }
}

void t_botoesIhm(void *p)
{
    delay(4000); // aguarda o objeto ihm ser inicializado

    while (1)
    {
        delay(100);

        uint16_t bt = 0;

        xSemaphoreTake(mutex_rs485, portMAX_DELAY);
        bt = ihm.requestButtons();
        xSemaphoreGive(mutex_rs485);

        if (bt == TIMEOUT)
        {
            Serial.println("timeout ihm");
        }
        else if (bt == BOTAO_NENHUM)
        {
            // Serial.print("NENHUM...");
        }
        else if (bt == BOTAO_PLAY_PAUSE)
        {
            enviaEvento(EVT_PLAY_PAUSE);
            Serial.println("PLAY/PAUSE");
        }
        else if (bt == BOTAO_HOLD_PLAY_PAUSE)
        {
            enviaEvento(EVT_HOLD_PLAY_PAUSE);
            Serial.println("HOLD PLAY PAUSE");
        }
        else if (flag_habilitaConfiguracaoPelaIhm) // só checa os botoes direcionais se a configuracao estiver liberada.
        {
            if (bt == BOTAO_CIMA)
            {
                Serial.println("CIMA");

                ihm.incrementaParametroAtual();

                Menu *checkMenu = ihm.getMenu();
                if (checkMenu == &menu_produto)
                {
                    // saveProdutoToEEPROM(produto); // to do: salvar produto na eeprom antes de trocar. Tem que ser feito antes de incrementar a variavel
                    loadProdutoFromEEPROM(produto);
                }
                else if (checkMenu == &menu_contadorDeCiclos)
                {
                    contadorDeCiclos = 0;
                }
            }
            else if (bt == BOTAO_ESQUERDA)
            {
                Serial.println("ESQUERDA");
                ihm.goToPreviousMenu();
            }
            else if (bt == BOTAO_BAIXO)
            {
                Serial.println("BAIXO");

                ihm.decrementaParametroAtual();

                Menu *checkMenu = ihm.getMenu();
                if (checkMenu == &menu_produto)
                {
                    // saveProdutoToEEPROM(produto); // to do: salvar produto na eeprom antes de trocar. Tem que ser feito antes de incrementar a variavel
                    loadProdutoFromEEPROM(produto);
                }
                else if (checkMenu == &menu_contadorDeCiclos)
                {
                    contadorDeCiclos = 0;
                }
            }
            else if (bt == BOTAO_DIREITA)
            {
                Serial.println("DIREITA");
                ihm.goToNextMenu();
            }
            else if (bt == BOTAO_HOLD_CIMA)
            {
                Serial.println("HOLD CIMA");
            }
            else if (bt == BOTAO_HOLD_ESQUERDA)
            {
                Serial.println("HOLD ESQUERDA");
            }
            else if (bt == BOTAO_HOLD_DIREITA)
            {
                Serial.println("HOLD DIREITA");
            }
            else if (bt == BOTAO_HOLD_BAIXO)
            {
                Serial.println("HOLD BAIXO");
            }
            else if (bt == BOTAO_HOLD_DIREITA_ESQUERDA)
            {
                Serial.println("HOLD DIREITA E ESQUERDA");

                if (flag_manutencao == false)
                {
                    liberaMenusDeManutencao();
                    ihm.goToMenu(&menu_simulaEtiqueta);
                    ihm.showStatus2msg("MANUTENCAO LIBERADA");
                }
            }
        }
        else
        {
            Serial.print("desabilitado ou erro> bt=");
            Serial.println(bt);
        }
    }
}

void liberaMenusDaIhm()
{
    ihm.addMenuToIndex(&menu_contadorDeCiclos);
    ihm.addMenuToIndex(&menu_produto);
    ihm.addMenuToIndex(&menu_atrasoSensorProduto);
    ihm.addMenuToIndex(&menu_posicaoDeAguardarProduto_dcmm);
    ihm.addMenuToIndex(&menu_distanciaProduto_dcmm);
    ihm.addMenuToIndex(&menu_velocidadeDeTrabalho_dcmm);
}

void liberaMenusDeManutencao()
{
    quantidadeDeMenusDeManutencao = 11; // atualize a quantidade de menus de manutencao, para nao ter erros na funcao bloqueiaMenusDeManutencao()
                                        // essa variavel é necessária porque os menus são removidos um a um.

    ihm.addMenuToIndex(&menu_simulaEtiqueta);
    ihm.addMenuToIndex(&menu_habilitaPortasDeSeguranca);
    ihm.addMenuToIndex(&menu_velocidadeDeReferenciacao_dcmm);
    ihm.addMenuToIndex(&menu_posicaoDePegarEtiqueta_dcmm);
    ihm.addMenuToIndex(&menu_posicaoLimite_dcmm);
    ihm.addMenuToIndex(&menu_tempoFinalizarAplicacao);
    ihm.addMenuToIndex(&menu_posicaoDeRepouso_dcmm);
    ihm.addMenuToIndex(&menu_rampa_dcmm);
    ihm.addMenuToIndex(&menu_velocidadeRebobinador);
    ihm.addMenuToIndex(&menu_aceleracaoRebobinador);
    ihm.addMenuToIndex(&menu_contadorTotal);

    flag_manutencao = true;
}

void bloqueiaMenusDeManutencao()
{
    for (int i = 0; i < quantidadeDeMenusDeManutencao; i++)
    {
        ihm.removeMenuFromIndex();
    }
    flag_manutencao = false;
}

void voltaParaPrimeiroMenu()
{
    ihm.goToMenu(&menu_contadorDeCiclos);
}

void habilitaConfiguracaoPelaIhm()
{
    flag_habilitaConfiguracaoPelaIhm = true;
}

void desabilitaConfiguracaoPelaIhm()
{
    flag_habilitaConfiguracaoPelaIhm = false;
}

void incrementaContadores()
{
    contadorDeCiclos++;
    contadorTotal++;
    //   salvaContadorNaEEPROM();
}

void t_rebobina(void *)
{
    const int16_t interval = 200;
    int16_t numeroDeVoltas = 3;
    int16_t fsm_rebobina = fase1;

    while (1)
    {
        delay(interval);

        if (fsm_rebobina == fase1)
        {
            rebobinador.move(rebobinador_ppr * numeroDeVoltas);
            fsm_rebobina = fase2;
        }
        else if (fsm_rebobina == fase2)
        {
            if (rebobinador.distanceToGo() == 0)
            {
                vTaskDelete(NULL);
            }
        }
    }
}

float dcmm_to_steps(int32_t _dcmm)
{
    float steps = 0;
    steps = _dcmm * resolucao;

    // Serial.print(_dcmm);
    // Serial.print("dcmm = ");
    // Serial.print(steps);
    // Serial.println("p");

    return steps;
}

int32_t steps_to_dcmm(float _steps)
{
    float dcmm = 0;
    dcmm = (float)_steps / resolucao;

    // Serial.print("  p:");
    // Serial.print(_steps);
    // Serial.print("  to dcmm:");
    // Serial.println(dcmm);

    return round(dcmm);
}

void braco_moveTo(int32_t _dcmm)
{
    if (_dcmm <= posicaoLimite_dcmm)
    {
        braco.moveTo(dcmm_to_steps(_dcmm));
    }
    else
    {
        Serial.println("targetPosition = posicao limite");
        braco.moveTo(dcmm_to_steps(posicaoLimite_dcmm));
    }
    // to do: se menor que zero tbm dá erro
}

void braco_move(int32_t _dcmm)
{
    if (braco_getCurrentPositionInDcmm() + _dcmm < posicaoLimite_dcmm)
    {
        braco.move(dcmm_to_steps(_dcmm));
    }
    else
    {
        Serial.println("targetPosition = posicao limite");
        braco.moveTo(dcmm_to_steps(posicaoLimite_dcmm));
    }
    // to do: se menor que zero também da erro.
}

int32_t braco_getCurrentPositionInDcmm()
{
    return steps_to_dcmm(braco.currentPosition());
}

void braco_setup(int32_t _velocidade_dcmmPorS, int32_t _rampa_dcmm)
{
    calculaVelocidadeEmSteps(_velocidade_dcmmPorS);
    calculaRampaEmSteps(_velocidade_dcmmPorS, _rampa_dcmm);
    braco.setMinPulseWidth(3);
}

void calculaVelocidadeEmSteps(int32_t _velocidade_dcmmPorS)
{
    int32_t velocidade_p = dcmm_to_steps(_velocidade_dcmmPorS);
    braco.setMaxSpeed(velocidade_p);
    Serial.print("freq braco: ");
    Serial.println(velocidade_p);
}

void calculaRampaEmSteps(int32_t _velocidade_dcmmPorS, int32_t _rampa_dcmm)
{
    int32_t aceleracao_dcmm = round((float)_velocidade_dcmmPorS * _velocidade_dcmmPorS / (2 * _rampa_dcmm));
    int32_t aceleracao_p = dcmm_to_steps(aceleracao_dcmm);
    braco.setAcceleration(aceleracao_p);
    // Serial.print("  acel: ");
    // Serial.println(aceleracao_p);
}

void rebobinador_setup(int32_t _velocidade_steps, int32_t _aceleracao_steps)
{
    rebobinador.setMaxSpeed(_velocidade_steps);
    rebobinador.setAcceleration(_aceleracao_steps);
    rebobinador.setMinPulseWidth(2);
}

void imprimeEtiqueta()
{
    if (flag_simulaEtiqueta)
    {
        xTaskCreatePinnedToCore(t_simulaPrintEtiqueta, "print task", 1024, NULL, PRIORITY_2, NULL, CORE_0);
    }
    else
    {
        xTaskCreatePinnedToCore(t_printEtiqueta, "print task", 1024, NULL, PRIORITY_2, NULL, CORE_0);
    }
}

void torre_ligaLuzVermelha()
{
    extIOs.desligaOutput(PIN_TORRE_LUMINOSA);
}

void torre_ligaLuzVerde()
{
    extIOs.ligaOutput(PIN_TORRE_LUMINOSA);
}

// simula a impressão de uma etiqueta, para fins de testes do software.
void t_simulaPrintEtiqueta(void *p)
{
    delay(1600);
    enviaEvento(EVT_IMPRESSAO_CONCLUIDA);
    vTaskDelete(NULL);
}

void t_printEtiqueta(void *p)
{
    const uint16_t intervalo_task = 1; // ms

    uint16_t fsm_print = fase1;
    uint32_t timer_duracaoDaImpressao = 0;
    const uint16_t timeout_duracaoDaImpressao = 2500;

    while (1)
    {
        delay(intervalo_task);

        if (fsm_print == fase1)
        {
            if (sinalPrintEnd.checkState() == LOW)
            {
                ligaPrint();
                rebobinador.move(50000);
                timer_duracaoDaImpressao = millis();
                fsm_print = fase2;
            }
            else
            {
                setFault(FALHA_IMPRESSAO);
                enviaEvento(EVT_FALHA);
                desligaPrint();
                rebobinador.stop();
                Serial.println("erro print: impressora desligada ou impressão em andamento");
                vTaskDelete(NULL);
            }
        }
        else if (fsm_print == fase2)
        {
            if (sinalPrintEnd.checkState() == HIGH)
            {
                fsm_print = fase3;
            }
            else if (millis() - timer_duracaoDaImpressao >= timeout_duracaoDaImpressao)
            {
                setFault(FALHA_IMPRESSAO);
                enviaEvento(EVT_FALHA);
                desligaPrint();
                rebobinador.stop();
                Serial.println("erro impressao: impressao nao comecou");
                vTaskDelete(NULL);
            }
        }
        else if (fsm_print == fase3)
        {
            if (sinalPrintEnd.checkState() == LOW)
            {
                desligaPrint();
                enviaEvento(EVT_IMPRESSAO_CONCLUIDA);
                delay(300);
                rebobinador.stop();
                vTaskDelete(NULL);
            }
            else if (millis() - timer_duracaoDaImpressao >= timeout_duracaoDaImpressao)
            {
                setFault(FALHA_IMPRESSAO);
                enviaEvento(EVT_FALHA);
                desligaPrint();
                rebobinador.stop();
                Serial.println("erro impressao: timeout duracao da impressao");
                vTaskDelete(NULL);
            }
        }
    }
}

// chame essa função para tirar eventos da fila de eventos
Evento recebeEventos()
{
    Evento receivedEvent = EVT_NENHUM;
    xQueueReceive(eventQueue, &receivedEvent, 0);
    return receivedEvent;
}

void changeFsmState(Estado estado)
{
    fsm = estado;
    Serial.print("current state: ");
    Serial.println(estado);
    fsm_substate = fase1;
}

void enviaEvento(Evento event)
{
    if (xQueueSend(eventQueue, (void *)&event, 10 / portTICK_PERIOD_MS) == pdFALSE)
    {
        Serial.print("erro enviaEvento: ");
        Serial.println(event);
    }
    // Serial.print("enviou evento: "); Serial.println(event);
}

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

void ligaReprint()
{
    digitalWrite(PIN_PRIN2, LOW);
}

void desligaReprint()
{
    digitalWrite(PIN_PRIN2, HIGH);
}

bool sensorDeAplicacaoDetectouProduto()
{
    return !sensorAplicacao.checkState(); // to do: usar checkSensorPulse
}

bool emCimaDoSensorHome()
{
    return !sensorHome.checkState(); // to do: usar checkSensorPulse
}

void imprimirZebra()
{
    ihm.imprimirRS485();
}

void playZebra()
{
    // ihm.playRS485();
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

    pulsosRampa = resolucao * rampa_dcmm;

    int32_t velocidadeLinearPulsosBracoInit = round(velocidadeReferencia * resolucao);
    int32_t aceleracaoLinearPulsosBracoInit = round(((velocidadeLinearPulsosBracoInit * velocidadeLinearPulsosBracoInit) / (2 * pulsosRampa)));

    // braco.setMaxSpeed(velocidadeLinearPulsosBracoInit); // escolhe velocidade em que o motor vai trabalhar
    // braco.setAcceleration(aceleracaoLinearPulsosBracoInit);
    // braco.setPinsInverted(DIRECAO_HORA); // muda direção do motor

    // rebobinador.setMaxSpeed(velocidadeReferenciaEspatula);
    // rebobinador.setAcceleration(aceleracaoReferenciaEspatula);
    // rebobinador.setPinsInverted(DIRECAO_ANTIHORA);

    braco.setMaxSpeed(1700);
    braco.setAcceleration(5000);
    braco.setPinsInverted();

    rebobinador.setMaxSpeed(3 * rebobinador_ppr);
    rebobinador.setAcceleration(12000);
    // rebobinador nao tem pino de controle de direcao

    velocidadeCiclommps = velocidadeReferencia;
}

void habilitaMotoresEAguardaEstabilizar()
{
    habilitaMotores();
    delay(350);
}

void habilitaMotores()
{
    extIOs.desligaOutput(PIN_ENABLE_MOTORES);
}

void desabilitaMotores()
{
    extIOs.ligaOutput(PIN_ENABLE_MOTORES);
}

void motorRun()
{
    const int32_t velocidadeEspatula = 8000;
    const int32_t aceleracaoEspatula = 80000;

    if (velocidadeCiclommps != velocidadeDeTrabalho_dcmm)
    {
        pulsosRampa = resolucao * rampa_dcmm;

        rebobinador.setMaxSpeed(velocidadeEspatula);
        rebobinador.setAcceleration(aceleracaoEspatula);

        velocidadeLinearPulsos = round(velocidadeDeTrabalho_dcmm * resolucao);
        aceleracaoLinearPulsos = round(((velocidadeLinearPulsos * velocidadeLinearPulsos) / (2 * pulsosRampa)));

        braco.setMaxSpeed(velocidadeLinearPulsos);
        braco.setAcceleration(aceleracaoLinearPulsos);
        velocidadeCiclommps = velocidadeDeTrabalho_dcmm;

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
void t_ihm_old(void *p)
{
    mutex_rs485 = xSemaphoreCreateMutex();

    uint32_t timer_display = 0;
    const uint32_t resetDisplay = 120000;
    String cont_str = "Contador: ";

    cont_str.concat(contadorTotal);
    ihm.configDefaultMsg(cont_str);
    ihm.configDefaultMsg2("PRINT APPLY LINEAR");

    xSemaphoreTake(mutex_rs485, portMAX_DELAY);
    ihm.setup();
    xSemaphoreGive(mutex_rs485);

    delay(3000);

    ihm.addMenuToIndex(&menu_produto);

    ihm.addMenuToIndex(&menu_atrasoSensorProduto);
    ihm.addMenuToIndex(&menu_atrasoImpressaoEtiqueta);
    ihm.addMenuToIndex(&menu_velocidadeDeTrabalho_dcmm);

    ihm.addMenuToIndex(&menu_contadorDeCiclos);

    // ihm.focus(&menu_produto); // Direciona a ihm para iniciar nesse menu

    while (1)
    {
        if (fsm_old.estado == PARADA_EMERGENCIA_OLD || fsm_old.sub_estado == PRONTO_OLD)
        {
            xSemaphoreTake(mutex_rs485, portMAX_DELAY);
            if (checkBotaoCima())
            {
                Menu *checkMenu = ihm.getMenu();
                if (checkMenu == &menu_produto)
                {
                    menu_produto.addVar(MAIS);
                    loadProdutoFromEEPROM(produto);
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
                    loadProdutoFromEEPROM(produto);
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
            // ihm.displayFull();

            flag_restartDisplay = false;
        }

        // ihm.task();

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

// checa qual/quais falhas estão ativas
// se o parametro faultCode foi colocado em zero(ou vazio), checa por qualquer falha
bool checkFault(uint8_t faultCode = 0)
{
    // to do: criar classe FAULT
    if (faultCode == 0)
    {
        return (faultRegister > 0);
    }
    else if (xSemaphoreTake(mutex_fault, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        bool check = (faultRegister & faultCode) != 0;
        xSemaphoreGive(mutex_fault);
        return check;
    }
    else
    {
        Serial.println("erro mtx fault");
    }
    return true;
}

void clearAllFaults()
{
    if (xSemaphoreTake(mutex_fault, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        faultRegister = 0;
        xSemaphoreGive(mutex_fault);
    }
    else
    {
        Serial.println("erro mtx fault");
    }
}

void updateFault(int16_t _faultCode, bool _faultState)
{
    if (xSemaphoreTake(mutex_fault, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        if (_faultState == true)
        {
            faultRegister |= _faultCode; // set byte
        }
        else
        {
            faultRegister &= ~(_faultCode); // clear byte
        }
        xSemaphoreGive(mutex_fault);
    }
    else
    {
        Serial.println("erro mtx fault");
    }
}

void setFault(int16_t _faultCode)
{
    if (xSemaphoreTake(mutex_fault, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        faultRegister |= _faultCode;
        xSemaphoreGive(mutex_fault);
    }
    else
    {
        Serial.println("erro mtx fault");
    }
}

void clearFault(int16_t _faultCode)
{
    if (xSemaphoreTake(mutex_fault, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        faultRegister &= ~(_faultCode); // clear byte
        xSemaphoreGive(mutex_fault);
    }
    else
    {
        Serial.println("erro mtx fault");
    }
}

// apenas a falha de maior prioridade será exibida na tela.
// a prioridade é definida pela ordem dos ifs dessa função aqui.
void imprimeFalhaNaIhm()
{
    String codFalha = "FALHA:";
    if (checkFault(FALHA_EMERGENCIA))
    {
        codFalha.concat("EMERGENCIA");
    }
    else if (checkFault(FALHA_IMPRESSAO))
    {
        codFalha.concat("IMPRESSAO");
    }
    else if (checkFault(FALHA_IMPRESSORA))
    {
        codFalha.concat("IMPRESSORA OFFLINE");
    }
    else if (checkFault(FALHA_SENSORES))
    {
        codFalha.concat("SENSORES");
    }
    else if (checkFault(FALHA_IHM))
    {
        codFalha.concat("IHM");
    }
    else if (checkFault(FALHA_PORTA_ABERTA))
    {
        codFalha.concat("PORTA ABERTA");
    }
    else if (checkFault(FALHA_APLICACAO))
    {
        codFalha.concat("APLICACAO");
    }
    else
    {
        codFalha.concat(faultRegister);
    }

    ihm.showStatus2msg(codFalha);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_eeprom(void *p)
{
    int16_t intervaloEntreBackups = 3000; // ms

    while (1)
    {
        delay(intervaloEntreBackups);
        saveParametersToEEPROM();
    }
}

void saveParametersToEEPROM()
{
    EEPROM.put(EPR_produto, produto);
    EEPROM.put(EPR_contadorTotal, contadorTotal);
    EEPROM.put(EPR_tempoFinalizarAplicacao, tempoFinalizarAplicacao);
    EEPROM.put(EPR_posicaoLimite_dcmm, posicaoLimite_dcmm);
    EEPROM.put(EPR_posicaoDePegarEtiqueta_dcmm, posicaoDePegarEtiqueta_dcmm);
    EEPROM.put(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.put(EPR_velocidadeDeReferenciacao_dcmm, velocidadeDeReferenciacao_dcmm);
    EEPROM.put(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.put(EPR_rampa_dcmm, rampa_dcmm);
    EEPROM.put(EPR_velocidadeRebobinador, velocidadeRebobinador);
    EEPROM.put(EPR_aceleracaoRebobinador, aceleracaoRebobinador);
    EEPROM.put(EPR_habilitaPortasDeSeguranca, habilitaPortasDeSeguranca);
    // EEPROM.put(EPR_startNF, startNF);

    salvaContadorNaEEPROM();

    saveProdutoToEEPROM(produto);

    EEPROM.commit();
}

// carrega os parametros que estão salvos na eeprom. Carrega os parâmetros globais e os de produto.
void loadParametersFromEEPROM()
{
    EEPROM.get(EPR_produto, produto);
    EEPROM.get(EPR_contadorTotal, contadorTotal);
    EEPROM.get(EPR_tempoFinalizarAplicacao, tempoFinalizarAplicacao);
    EEPROM.get(EPR_posicaoLimite_dcmm, posicaoLimite_dcmm);
    EEPROM.get(EPR_posicaoDePegarEtiqueta_dcmm, posicaoDePegarEtiqueta_dcmm);
    EEPROM.get(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.get(EPR_velocidadeDeReferenciacao_dcmm, velocidadeDeReferenciacao_dcmm);
    EEPROM.get(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.get(EPR_rampa_dcmm, rampa_dcmm);
    EEPROM.get(EPR_velocidadeRebobinador, velocidadeRebobinador);
    EEPROM.get(EPR_aceleracaoRebobinador, aceleracaoRebobinador);
    EEPROM.get(EPR_habilitaPortasDeSeguranca, habilitaPortasDeSeguranca);
    // EEPROM.get(EPR_startNF, startNF);
    loadProdutoFromEEPROM(produto);
}

// carrega os parametros de um dos produtos
void loadProdutoFromEEPROM(int16_t _produto)
{
    EEPROM.get(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(atrasoSensorProduto) * EPR_atrasoSensorProduto, atrasoSensorProduto);
    EEPROM.get(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(posicaoDeAguardarProduto_dcmm) * EPR_posicaoDeAguardarProduto_dcmm, posicaoDeAguardarProduto_dcmm);
    EEPROM.get(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(distanciaProduto_dcmm) * EPR_distanciaProduto_dcmm, distanciaProduto_dcmm);
    EEPROM.get(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(velocidadeDeTrabalho_dcmm) * EPR_velocidadeDeTrabalho_dcmm, velocidadeDeTrabalho_dcmm);
}

// salva os parâmetros do _produto na eeprom
void saveProdutoToEEPROM(int16_t _produto)
{
    EEPROM.put(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(atrasoSensorProduto) * EPR_atrasoSensorProduto, atrasoSensorProduto);
    EEPROM.put(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(posicaoDeAguardarProduto_dcmm) * EPR_posicaoDeAguardarProduto_dcmm, posicaoDeAguardarProduto_dcmm);
    EEPROM.put(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(distanciaProduto_dcmm) * EPR_distanciaProduto_dcmm, distanciaProduto_dcmm);
    EEPROM.put(EPR_inicioDaMemoriaDosProdutos + _produto * EPR_quantoOcupaCadaProdutoNaMemoria + sizeof(velocidadeDeTrabalho_dcmm) * EPR_velocidadeDeTrabalho_dcmm, velocidadeDeTrabalho_dcmm);
}

// escreve valores default em todos os produtos
void presetEEPROM()
{
    for (int i = 0; i < EPR_maxProdutos; i++)
    {
        saveProdutoToEEPROM(i);
    }
}

// salva o contador na eeprom. A eeprom não possui um número muito alto de escritas, então é necessário economizar escritas.
// essa função executa a escrita apenas a cada 100 ciclos.
void salvaContadorNaEEPROM()
{
    const uint16_t intervaloEntreBackups = 100; // ciclos
    if ((contadorTotal % intervaloEntreBackups) == 0)
    {
        // Serial.print("save contador: ");Serial.println(contadorTotal);
        EEPROM.put(EPR_contadorTotal, contadorTotal);
    }
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void t_emergencia(void *p)
{
    unsigned int interval = 200;

    while (1)
    {
        delay(interval);

        xSemaphoreTake(mutex_ios, portMAX_DELAY);
        extIOs.updateInputState();
        xSemaphoreGive(mutex_ios);
        if (extIOs.checkInputState(PIN_EMERGENCIA) == LOW)
        {
            setFault(FALHA_EMERGENCIA);
            enviaEvento(EVT_PARADA_EMERGENCIA);
        }
        else if (habilitaPortasDeSeguranca)
        {
            if (extIOs.checkInputState(PIN_SENSOR_DE_PORTAS) == LOW)
            {
                setFault(FALHA_PORTA_ABERTA);
                enviaEvento(EVT_PARADA_EMERGENCIA);
            }
        }
        else if (flag_simulaEtiqueta == false)
        {
            if (sinalImpressoraOnline.checkState() == LOW)
            {
                setFault(FALHA_IMPRESSORA);
            }
        }
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
                    fsm_old.sub_estado = MANUTENCAO_OLD;
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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Thread que controla as entradas e saídas gerais
void t_io(void *p)
{
    while (1)
    {
        if (xSemaphoreTake(mutex_ios, pdMS_TO_TICKS(5))) // mutex output_state
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
            xSemaphoreGive(mutex_ios); // mutex output_state
        }
        delay(3);
    }
}

void resetBits(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mutex_ios, pdMS_TO_TICKS(1))) // mutex output_state
    {
        output_state &= ~bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mutex_ios); // mutex output_state
    }
    else
    {
        Serial.println("mtx reset");
    }
}

void setBits(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mutex_ios, pdMS_TO_TICKS(1))) // mutex output_state
    {
        output_state |= bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mutex_ios); // mutex output_state
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
    if (xSemaphoreTake(mutex_ios, pdMS_TO_TICKS(1))) // Mutex output_state
    {
        output_state &= ~bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mutex_ios); // Mutex output_state
    }
    else
    {
        Serial.println("mtx reset");
    }
}

void desligaOutput(uint8_t posicaoDoBit)
{
    if (xSemaphoreTake(mutex_ios, pdMS_TO_TICKS(1))) // Mutex output_state
    {
        output_state |= bit(posicaoDoBit);
        updateOutput(output_state);
        xSemaphoreGive(mutex_ios); // Mutex output_state
    }
    else
    {
        Serial.println("mtx set");
    }
}

void desligaTodosOutputs()
{
    digitalWrite(PIN_DO1, HIGH);
    digitalWrite(PIN_DO2, HIGH);
    desligaVentilador();
    digitalWrite(PIN_HSDO1, LOW);
    digitalWrite(PIN_HSDO2, LOW);
    digitalWrite(PIN_HSDO3, LOW);
    digitalWrite(PIN_HSDO4, LOW);

    uint8_t output = 0;
    output = bit(DO5) | bit(DO6) | bit(DO7) | bit(DO8);
    extIOs.changeOutputState(output);
    desabilitaMotores();
    torre_ligaLuzVerde();
    extIOs.ligaOutput(RLO1);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void ventiladorSetup()
{
    const uint16_t frequencia = 1000;
    const uint16_t resolucao = 8;

    ledcSetup(VENTILADOR_CANAL, frequencia, resolucao);
    ledcAttachPin(PIN_VENTILADOR, VENTILADOR_CANAL);
}

void ventiladorWrite(uint16_t canal, uint16_t intensidade)
{
    uint16_t dutyCycle = map(intensidade, 0, 100, 0, 255);

    ledcWrite(canal, dutyCycle);
}

void ligaVentilador()
{
    // digitalWrite(PIN_VENTILADOR, HIGH);

    uint16_t dutyCycle = map(potenciaVentilador, 0, 100, 0, 255);

    ledcWrite(VENTILADOR_CANAL, dutyCycle);
}

void desligaVentilador()
{
    ledcWrite(VENTILADOR_CANAL, 0);
    // digitalWrite(PIN_VENTILADOR, LOW);
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
        Serial.print("on: ");
        Serial.print(millis() / 1000);
        Serial.print("  SP: ");
        Serial.print(digitalRead(PIN_SENSOR_PRODUTO));
        Serial.print(" SH: ");
        Serial.print(digitalRead(PIN_SENSOR_HOME));
        Serial.print(" SA: ");
        Serial.print(digitalRead(PIN_SENSOR_APLICACAO));
        Serial.print(" PREND: ");
        Serial.print(digitalRead(PIN_PREND));
        Serial.print(" braco_pos: ");
        Serial.print(braco.currentPosition());

        Serial.println();
        delay(2000);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Functions:
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#endif