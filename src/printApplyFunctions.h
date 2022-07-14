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
    ESTADO_REFERENCIANDO,
    ESTADO_APLICACAO,
    ESTADO_POSICIONANDO,
    ESTADO_FALHA,
    ESTADO_TESTE_DE_IMPRESSAO,
    ESTADO_TESTE_DO_BRACO,
    ESTADO_TESTE_DO_VENTILADOR,
    ESTADO_TESTE_COMUNICACAO,
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
    EVT_MENSAGEM_ENVIADA,
};

// Objetos:
SemaphoreHandle_t mutex_ios;
SemaphoreHandle_t mutex_rs485;
SemaphoreHandle_t mutex_fault; // controla o acesso à variável faultRegister, que é utilizada por várias threads
QueueHandle_t eventQueue;      // os eventos são armazenados em uma fila

AccelStepper braco(AccelStepper::DRIVER, PIN_PUL_BRACO, PIN_DIR_BRACO);
AccelStepper rebobinador(AccelStepper::DRIVER, PIN_PUL_REBOBINADOR, PIN_DIR_REBOBINADOR); // na verdade o DIR do rebobinador não está conectado. Então defini um pino que não está sendo utilizado.

TaskHandle_t h_eeprom;
TaskHandle_t h_botoesIhm;

extern HardwareSerial rs485;

ihmSunnytecMaster ihm{protocoloIhm{PIN_RS485_RX, PIN_RS485_TX, PIN_RS485_EN}};
extendedIOs extIOs = extendedIOs(PIN_IO_CLOCK, PIN_IO_LATCH, PIN_INPUT_DATA, PIN_OUTPUT_DATA);
checkSensorPulse sensorDeProdutoOuStart = checkSensorPulse(PIN_SENSOR_PRODUTO, 1);
checkSensorPulse sinalPrintEnd = checkSensorPulse(PIN_PREND, 1);
checkSensorPulse sensorAplicacao = checkSensorPulse(PIN_SENSOR_APLICACAO, 1);
checkSensorPulse sensorHome = checkSensorPulse(PIN_SENSOR_HOME, 1);
checkSensorPulse sinalImpressoraOnline = checkSensorPulse(PIN_IMPRESSORA_ONLINE, 1);

// Outros:
uint16_t quantidadeDeMenusDeManutencao = 1;

String mensagemTeste = "\eA\eV100\eH200\eP3\eL0403\eXMABCD\eQ2\eZ"; // \e é ESC ou 0x1B
String msgBuffer_out;

int32_t faultRegister = 0; // o fault byte armazena as falhas da máquina como se fosse um registrador.
                           // a vantagem de utilizar o faultRegister, é que é possível ter mais de uma falha ao mesmo tempo.
// Flags:
bool flag_referenciou = false;
bool flag_cicloEmAndamento = false;
bool flag_debugEnabled = true;
bool flag_manutencao = false;
bool flag_habilitaConfiguracaoPelaIhm = true; // se true, todos os botões da ihm serão processados. Se false, apenas play/stop serão processados.

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
int32_t rampaReferenciacao_dcmm = 200; // to do: fazer menu e colocar na eeprom
int32_t flag_simulaEtiqueta = false;
int32_t velocidadeRebobinador = 9600;
int32_t aceleracaoRebobinador = 12000;
int32_t habilitaPortasDeSeguranca = 1;
int32_t potenciaVentilador = 30; // porcentagem
int32_t contadorTotal = 0;       // to do: mudar nome para contadorTotal
int32_t enviaMensagem = 0;
int32_t printTest = 0;

// parâmetros de instalação (só podem ser alterados na compilação do software):
const int32_t tamanhoMaximoDoBraco_dcmm = 4450;
const uint32_t rebobinador_ppr = 3200; // pulsos/revolucao

const float resolucaoNaCalibracao = 20.4803; // steps/dcmm
const uint32_t pprNaCalibracao = 25000;      // pulsos/revolucao

const uint32_t braco_ppr = 3200;                                             // pulsos/revolucao
const float resolucao = braco_ppr * resolucaoNaCalibracao / pprNaCalibracao; // steps/dcmm

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// menus:
Menu menu_contadorDeCiclos = Menu("Contador", READONLY, &contadorDeCiclos);
Menu menu_produto = Menu("Produto", PARAMETRO, &produto, " ", 1u, 1u, (unsigned)(EPR_maxProdutos));
Menu menu_atrasoSensorProduto = Menu("Atraso Produto", PARAMETRO, &atrasoSensorProduto, "ms", 10u, 10u, 5000u, &produto);
Menu menu_posicaoDeAguardarProduto_dcmm = Menu("Pos Aguarda Produto", PARAMETRO, &posicaoDeAguardarProduto_dcmm, "mm", 10, 10, tamanhoMaximoDoBraco_dcmm, &produto, 1);
Menu menu_distanciaProduto_dcmm = Menu("Distancia Produto", PARAMETRO, &distanciaProduto_dcmm, "mm", 10u, 10u, tamanhoMaximoDoBraco_dcmm, &produto, 1);
Menu menu_velocidadeDeTrabalho_dcmm = Menu("Velocidade Aplicacao", PARAMETRO, &velocidadeDeTrabalho_dcmm, "mm/s", 100u, 100u, 15000u, &produto, 1);

// menus de manutencao:
// to do: menu de falhas.
Menu menu_simulaEtiqueta = Menu("Simula Etiqueta", PARAMETRO, &flag_simulaEtiqueta, " ", 1u, 0u, 1u, NULL);
Menu menu_habilitaPortasDeSeguranca = Menu("Portas de Seguranca", PARAMETRO, &habilitaPortasDeSeguranca, " ", 1u, 0u, 1u, NULL);
Menu menu_tempoFinalizarAplicacao = Menu("Finalizar Aplicacao", PARAMETRO, &tempoFinalizarAplicacao, "ms", 10u, 20u, 500u);
Menu menu_potenciaVentilador = Menu("Potencia Ventilador", PARAMETRO, &potenciaVentilador, "%", 1u, 10u, 100u);
Menu menu_posicaoDePegarEtiqueta_dcmm = Menu("Pos Pega Etiqueta", PARAMETRO, &posicaoDePegarEtiqueta_dcmm, "mm", 5, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_posicaoLimite_dcmm = Menu("Pos Limite", PARAMETRO, &posicaoLimite_dcmm, "mm", 10, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_posicaoDeRepouso_dcmm = Menu("Pos Repouso", PARAMETRO, &posicaoDeRepouso_dcmm, "mm", 10, 20, tamanhoMaximoDoBraco_dcmm, NULL, 1);
Menu menu_velocidadeDeReferenciacao_dcmm = Menu("Veloc Referenciacao", PARAMETRO, &velocidadeDeReferenciacao_dcmm, "mm/s", 100u, 100u, 15000u, NULL, 1);
Menu menu_rampa_dcmm = Menu("Rampa", PARAMETRO, &rampa_dcmm, "mm", 5u, 10u, 500u, NULL, 1);
Menu menu_rampaReferenciacao_dcmm = Menu("Rampa Ref", PARAMETRO, &rampaReferenciacao_dcmm, "mm", 5u, 10u, 200u, NULL, 1);
Menu menu_contadorTotal = Menu("Contador Total", READONLY, &contadorTotal, " ");
Menu menu_velocidadeRebobinador = Menu("Veloc Rebobinador", PARAMETRO, &velocidadeRebobinador, "pulsos", 100u, 1000u, 50000u, NULL);
Menu menu_aceleracaoRebobinador = Menu("Acel Rebobinador", PARAMETRO, &aceleracaoRebobinador, "pulsos", 100u, 1000u, 50000u, NULL);
Menu menu_enviaMensagem = Menu("Envia Mensagem", PARAMETRO, &enviaMensagem, " ", 1u, 0u, 1u, NULL);
Menu menu_printTest = Menu("Print Test", PARAMETRO, &printTest, " ", 1u, 0u, 1u, NULL);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Prototypes:
void createTasks();

void habilitaMotoresEAguardaEstabilizar();
void desabilitaMotores();
void habilitaMotores();

void saveParametersToEEPROM();
void saveProdutoToEEPROM(int16_t _produto);
void loadParametersFromEEPROM();
void loadProdutoFromEEPROM(int16_t);
void presetEEPROM();
void salvaContadorNaEEPROM();
void t_eeprom(void *p);

void t_emergencia(void *p);

void desligaTodosOutputs();

void ventiladorSetup();
void ligaVentilador();
void desligaVentilador();

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
void bloqueiaMenusDeManutencao();
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

void enviaMensagemDeTesteParaImpressora();
void t_enviaMensagem(void *p);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void createTasks()
{
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
                else if (checkMenu == &menu_enviaMensagem)
                {
                    enviaMensagemDeTesteParaImpressora();
                    enviaMensagem = 0;
                }
                else if (checkMenu == &menu_printTest)
                {
                    xTaskCreatePinnedToCore(t_printEtiqueta, "print task", 1024, NULL, PRIORITY_2, NULL, CORE_0);
                    printTest = 0;
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
                else if (checkMenu == &menu_enviaMensagem)
                {
                    enviaMensagemDeTesteParaImpressora();
                    enviaMensagem = 0;
                }
                else if (checkMenu == &menu_printTest)
                {
                    xTaskCreatePinnedToCore(t_printEtiqueta, "print task", 1024, NULL, PRIORITY_2, NULL, CORE_0);
                    printTest = 0;
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

void enviaMensagemDeTesteParaImpressora()
{
    msgBuffer_out = "\eA\eV100\eH200\eP3\eL0403\eXMABCD\eQ2\eZ"; // mensagem simples, texto = ABCD e quantidade = 2.
    xTaskCreatePinnedToCore(t_enviaMensagem, "msg task", 1024, NULL, PRIORITY_2, NULL, CORE_0);
}

void t_enviaMensagem(void *p)
{
    xSemaphoreTake(mutex_rs485, portMAX_DELAY);
    ihm.liquidC.envio485(msgBuffer_out); // to do: separar rs485 da lib da ihm, para que o software principal possa utilizar o RS485 separado.
    xSemaphoreGive(mutex_rs485);

    enviaEvento(EVT_MENSAGEM_ENVIADA);
    Serial.println("msg de teste enviada.");
    // to do: checar confirmação da impressora

    vTaskDelete(NULL);
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
    quantidadeDeMenusDeManutencao = 15; // atualize a quantidade de menus de manutencao, para nao ter erros na funcao bloqueiaMenusDeManutencao()
                                        // essa variavel é necessária porque os menus são removidos um a um.

    ihm.addMenuToIndex(&menu_simulaEtiqueta);
    ihm.addMenuToIndex(&menu_habilitaPortasDeSeguranca);
    ihm.addMenuToIndex(&menu_velocidadeDeReferenciacao_dcmm);
    ihm.addMenuToIndex(&menu_posicaoDePegarEtiqueta_dcmm);
    ihm.addMenuToIndex(&menu_posicaoLimite_dcmm);
    ihm.addMenuToIndex(&menu_tempoFinalizarAplicacao);
    ihm.addMenuToIndex(&menu_potenciaVentilador);
    ihm.addMenuToIndex(&menu_posicaoDeRepouso_dcmm);
    ihm.addMenuToIndex(&menu_rampa_dcmm);
    ihm.addMenuToIndex(&menu_rampaReferenciacao_dcmm);
    ihm.addMenuToIndex(&menu_velocidadeRebobinador);
    ihm.addMenuToIndex(&menu_aceleracaoRebobinador);
    ihm.addMenuToIndex(&menu_enviaMensagem);
    ihm.addMenuToIndex(&menu_printTest);
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
    salvaContadorNaEEPROM(); // to do: reativar
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
                habilitaMotores();
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
    EEPROM.put(EPR_tempoFinalizarAplicacao, tempoFinalizarAplicacao);
    EEPROM.put(EPR_posicaoLimite_dcmm, posicaoLimite_dcmm);
    EEPROM.put(EPR_posicaoDePegarEtiqueta_dcmm, posicaoDePegarEtiqueta_dcmm);
    EEPROM.put(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.put(EPR_velocidadeDeReferenciacao_dcmm, velocidadeDeReferenciacao_dcmm);
    EEPROM.put(EPR_posicaoDeRepouso_dcmm, posicaoDeRepouso_dcmm);
    EEPROM.put(EPR_rampa_dcmm, rampa_dcmm);
    EEPROM.put(EPR_rampaReferenciacao_dcmm, rampaReferenciacao_dcmm);
    EEPROM.put(EPR_velocidadeRebobinador, velocidadeRebobinador);
    EEPROM.put(EPR_aceleracaoRebobinador, aceleracaoRebobinador);
    EEPROM.put(EPR_habilitaPortasDeSeguranca, habilitaPortasDeSeguranca);
    EEPROM.put(EPR_potenciaVentilador, potenciaVentilador);
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
    EEPROM.get(EPR_rampaReferenciacao_dcmm, rampaReferenciacao_dcmm);
    EEPROM.get(EPR_velocidadeRebobinador, velocidadeRebobinador);
    EEPROM.get(EPR_aceleracaoRebobinador, aceleracaoRebobinador);
    EEPROM.get(EPR_habilitaPortasDeSeguranca, habilitaPortasDeSeguranca);
    EEPROM.get(EPR_potenciaVentilador, potenciaVentilador);
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
        Serial.print("save contador: ");
        Serial.println(contadorTotal);
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
        } // to do: trocar "else if" por vários if's independentes

        if (flag_simulaEtiqueta == false)
            updateFault(FALHA_IMPRESSORA, !sinalImpressoraOnline.checkState());
        else
            updateFault(FALHA_IMPRESSORA, false);
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

void ligaVentilador()
{
    uint16_t dutyCycle = map(potenciaVentilador, 0, 100, 0, 255);

    ledcWrite(VENTILADOR_CANAL, dutyCycle);
}

void desligaVentilador()
{
    ledcWrite(VENTILADOR_CANAL, 0);
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
        Serial.print(" ONLINE: ");
        Serial.print(sinalImpressoraOnline.checkState());
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