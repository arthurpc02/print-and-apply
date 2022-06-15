/* Projeto Print Apply - Software desenvolvido para a
placa industrial V2.0 comunicando com a IHM - v1.0 */

// Funções Print & Apply - v1.0

#include <printApplyFunctions.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Print & Apply setup");

  mutex_ios = xSemaphoreCreateMutex();
  mutex_rs485 = xSemaphoreCreateMutex();
  mutex_fault = xSemaphoreCreateMutex();
  eventQueue = xQueueCreate(3, sizeof(Evento));

  EEPROM.begin(EEPROM_SIZE);
  loadParametersFromEEPROM();
  // presetEEPROM();

  desligaTodosOutputs();
  sensorDeProdutoOuStart.setup();
  extIOs.init();

  createTasks();

  pinInitialization();
  // ventiladorConfig();
  // motorSetup();
  Serial.print("p/dcmm braco: "); Serial.println(resolucao);
  braco_setup(velocidadeDeTrabalho_dcmm, rampa_dcmm);
  rebobinador_setup(velocidadeRebobinador, aceleracaoRebobinador);

  Serial.println("End Setup. Print & Apply Linear.");
}

void loop()
{
  braco.run();
  rebobinador.run();

  Evento evento = recebeEventos();

  switch (fsm)
  {
  case ESTADO_EMERGENCIA:
  {
    static uint32_t timer_emergencia = 0;
    const uint32_t timeout_emergencia = 250;

    if (fsm_substate == fase1)
    {
      desabilitaMotores();
      desligaTodosOutputs();
      vTaskResume(h_eeprom);
      ihm.showStatus2msg("BOTAO EMERGENCIA");
      delay(1);
      ihm.ligaLEDvermelho();
      delay(1);
      ihm.desligaLEDverde();
      Serial.println("ESTADO EMERGENCIA");
      // habilitaConfiguracaoPelaIhm();
      timer_emergencia = millis();
      // encoder.clearCount();
      flag_referenciou = false;
      flag_cicloEmAndamento = false;
      fsm_substate = fase2;
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_PARADA_EMERGENCIA)
      {
        timer_emergencia = millis();
      }
      else if (millis() - timer_emergencia > timeout_emergencia)
      {
        ihm.desligaLEDvermelho();
        // falhas.clearAllFaults();
        // changeFsmState(ESTADO_TESTE_DE_IMPRESSAO);
        clearAllFaults();
        changeFsmState(ESTADO_STOP);
      }
    }
    break;
  }
  case ESTADO_STOP:
  {
    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }

    if (fsm_substate == fase1)
    {
      braco.stop();
      fsm_substate = fase2;
    }
    else if (fsm_substate == fase2)
    {
      if (braco.distanceToGo() == 0)
      {
        if (flag_referenciou)
        {
          braco_moveTo(posicaoDeRepouso_dcmm);
        }
        fsm_substate = fase3;
      }
    }
    else if (fsm_substate == fase3)
    {
      if (braco.distanceToGo() == 0)
      {
        ihm.desligaLEDverde();
        delay(1);
        ihm.showStatus2msg("EM PAUSA");
        Serial.println("ESTADO STOP");
        desligaTodosOutputs();
        vTaskResume(h_eeprom);
        flag_cicloEmAndamento = false;
        fsm_substate = fase4;
      }
    }
    else if (fsm_substate == fase4)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        vTaskSuspend(h_eeprom);
        voltaParaPrimeiroMenu();
        habilitaMotoresEAguardaEstabilizar();
        braco_setup(velocidadeDeTrabalho_dcmm, rampa_dcmm);
        rebobinador_setup(velocidadeRebobinador, aceleracaoRebobinador);

        if (flag_manutencao)
        {
          bloqueiaMenusDeManutencao();
        }

        if (flag_referenciou == false)
        {
          changeFsmState(ESTADO_REFERENCIANDO);
        }
        else
        {
          flag_cicloEmAndamento = true;
          changeFsmState(ESTADO_POSICIONANDO);
          // changeFsmState(ESTADO_TESTE_DE_IMPRESSAO);
          // changeFsmState(ESTADO_TESTE_DO_BRACO);
          // changeFsmState(ESTADO_TESTE_DO_VENTILADOR);
          // changeFsmState(ESTADO_APLICACAO);
        }
      }
    }
    break;
  }
  case ESTADO_POSICIONANDO:
  {
    static bool flag_pause = false;

    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      flag_pause = true;
      break;
    }

    if (fsm_substate == fase1)
    {
      if (braco.distanceToGo() == 0)
      {
        braco_setup(velocidadeDeTrabalho_dcmm, rampa_dcmm);
        braco_moveTo(posicaoDePegarEtiqueta_dcmm);
        ligaVentilador();
        fsm_substate = fase2;
      }
    }
    else if (fsm_substate == fase2)
    {
      if (braco.distanceToGo() == 0)
      {
        imprimeEtiqueta();
        fsm_substate = fase3;
      }
    }
    else if (fsm_substate == fase3)
    {
      if (evento == EVT_IMPRESSAO_CONCLUIDA)
      {
        if (flag_pause)
        {
          flag_pause = false;
          changeFsmState(ESTADO_STOP);
        }
        else
        {
          braco_moveTo(posicaoDeAguardarProduto_dcmm);
          fsm_substate = fase4;
        }
      }
      else if (evento == EVT_FALHA)
      {
        setFault(FALHA_IMPRESSAO);
        Serial.println("falha impressora");
        // to do: faultRegister
        changeFsmState(ESTADO_FALHA);
      }
    }
    else if (fsm_substate == fase4)
    {
      if (braco.distanceToGo() == 0)
      {
        ihm.ligaLEDverde();
        delay(1);
        ihm.showStatus2msg("AGUARDANDO PRODUTO");
        changeFsmState(ESTADO_APLICACAO);
      }
    }
    break;
  }
  case ESTADO_APLICACAO:
  {
    static uint32_t timer_atrasoSensorProduto = 0;
    static uint32_t timer_finalizaAplicacao = 0;
    static bool flag_pause = false;

    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      flag_pause = true;
      break;
    }

    if (fsm_substate == fase1)
    {
      if (sensorDeProdutoOuStart.checkPulse())
      {
        timer_atrasoSensorProduto = millis();
        fsm_substate = fase2;
      }
      else if (flag_pause)
      {
        flag_pause = false;
        changeFsmState(ESTADO_STOP);
      }
    }
    else if (fsm_substate == fase2)
    {
      if (millis() - timer_atrasoSensorProduto >= atrasoSensorProduto)
      {
        braco_moveTo(posicaoLimite_dcmm);
        fsm_substate = fase3;
      }
    }
    else if (fsm_substate == fase3)
    {
      if (sensorDeAplicacaoDetectouProduto())
      {
        Serial.println("detectou produto");
        if (distanciaProduto_dcmm < rampa_dcmm)
        {
          braco.stop();
        }
        else
        {
          braco_move(distanciaProduto_dcmm);
        }
        fsm_substate = fase4;
      }
      else if (braco.distanceToGo() == 0)
      {
        setFault(FALHA_APLICACAO);
        changeFsmState(ESTADO_FALHA); // to do:
        Serial.println("erro de aplicação");
      }
    }
    else if (fsm_substate == fase4)
    {
      if (braco.distanceToGo() == 0)
      {
        timer_finalizaAplicacao = millis();
        fsm_substate = fase5;
        desligaVentilador();
      }
    }
    else if (fsm_substate == fase5)
    {
      if (millis() - timer_finalizaAplicacao > tempoFinalizarAplicacao)
      {
        if (flag_pause)
        {
          flag_pause = false;
          changeFsmState(ESTADO_STOP);
        }
        else
        {
          changeFsmState(ESTADO_REFERENCIANDO);
        }
      }
    }
    break;
  }
  case ESTADO_REFERENCIANDO:
  {
    // to do: quando inicia a máquina, tem que rebobinar.

    const int16_t tempoDeEstabilizacaoNaReferenciacao = 200; // ms

    const int32_t distanciaParaSairDoSensor_dcmm = 350;
    static int32_t posicaoZero = 0;

    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (fsm_substate == fase1)
    {
      braco_setup(velocidadeDeReferenciacao_dcmm, rampa_dcmm);
      fsm_substate = fase2;

      if (emCimaDoSensorHome())
      {
        // se já esta no sensor Home, tem que se afastar para fazer a referenciacao
        braco_move(distanciaParaSairDoSensor_dcmm);
      }
    }
    else if (fsm_substate == fase2)
    {
      if (braco.distanceToGo() == 0)
      {
        if (emCimaDoSensorHome())
        {
          // se ainda está no sensor Home, provavelmente o sensor nao esta funcionando ou nao houve movimento.
          setFault(FALHA_SENSORES);
          changeFsmState(ESTADO_FALHA);
        }
        else
        {
          const float mais15porCento = 1.15;
          const int16_t direcaoNegativa = -1;
          braco_move(tamanhoMaximoDoBraco_dcmm * mais15porCento * direcaoNegativa);
          fsm_substate = fase3;
        }
      }
    }
    else if (fsm_substate == fase3)
    {
      if (emCimaDoSensorHome())
      {
        posicaoZero = braco.currentPosition();
        braco.stop();
        fsm_substate = fase4;
      }
      else if (braco.distanceToGo() == 0)
      {
        Serial.println("erro referenciacao: fim da area util.");
      }
    }
    else if (fsm_substate == fase4)
    {
      if (braco.distanceToGo() == 0)
      {
        Serial.print("currt pos: ");
        Serial.println(braco.currentPosition());
        Serial.print(" p0: ");
        Serial.println(posicaoZero);
        braco.setCurrentPosition(braco.currentPosition() - posicaoZero);
        flag_referenciou = true;
        delay(tempoDeEstabilizacaoNaReferenciacao);
        if (flag_cicloEmAndamento)
        {
          changeFsmState(ESTADO_POSICIONANDO);
        }
        else
        {
          changeFsmState(ESTADO_STOP);
        }
      }
    }
    break;
  }
  case ESTADO_TESTE_DO_BRACO:
  {
    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (fsm_substate == fase1)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        braco.move(1000);
      }
    }

    break;
  }
  case ESTADO_TESTE_DE_IMPRESSAO:
  {
    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (fsm_substate == fase1)
    {
      Serial.println("ESTADO TESTE DE IMPRESSAO");
      fsm_substate = fase2;
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        // rebobinador.move(rebobinador_ppr * 1.5);
        imprimeEtiqueta();
        fsm_substate = fase2;
      }
    }
    else if (fsm_substate == fase3)
    {
      if (evento == EVT_IMPRESSAO_CONCLUIDA)
      {
        fsm_substate = fase1;
      }
      else if (evento == EVT_FALHA)
      {
        changeFsmState(ESTADO_FALHA);
        Serial.println("erro impressao");
        fsm_substate = fase1;
      }
    }
    break;
  }
  case ESTADO_TESTE_DO_VENTILADOR:
  {
    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (evento == EVT_PLAY_PAUSE)
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (fsm_substate == fase1)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        ligaVentilador();
        fsm_substate = fase2;
      }
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        desligaVentilador();
        fsm_substate = fase1;
      }
    }
    break;
  }
  case ESTADO_FALHA:
  {
    static uint32_t timer_verificaoDeFalhas = 0;
    const uint16_t tempoVerificacaoDeFalhas = 7000; // ms

    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }

    // to do: sempre passar pelo estado de falha antes de ir para o estado emergencia
    // to do: de 10 em 10 segundos imprime a falha atual na tela.

    if (fsm_substate == fase1)
    {
      vTaskResume(h_eeprom);
      flag_cicloEmAndamento = false;
      flag_referenciou = false;
      delay(1);
      ihm.ligaLEDvermelho();
      delay(1);
      ihm.desligaLEDverde();
      desligaTodosOutputs();
      imprimeFalhaNaIhm();
      timer_verificaoDeFalhas = millis();
      fsm_substate = fase2;
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_HOLD_PLAY_PAUSE)
      {
        ihm.desligaLEDvermelho();
        clearAllFaults();
        changeFsmState(ESTADO_STOP);
      }

      if (millis() - timer_verificaoDeFalhas >= tempoVerificacaoDeFalhas)
      {
        fsm_substate = fase1;
      }
    }
    break;
  }
  }

  switch (fsm_old.estado)
  {
  case ESTADO_DESATIVADO:
  {
    break;
  }
  case PARADA_EMERGENCIA_OLD:
  {
    static uint32_t timer_requestStatusImpressora = 0;

    if (millis() - timer_requestStatusImpressora >= 10000)
    {
      xTaskCreatePinnedToCore(t_requestStatusImpressoraZebra, "request status task", 4096, NULL, PRIORITY_1, NULL, CORE_0);
      timer_requestStatusImpressora = millis();
    }

    if (fsm_old.sub_estado == EMERGENCIA_TOP_OLD)
    {
      if (fsm_emergencia == fase1)
      {
        braco.stop();
        rebobinador.stop();
        motorSetup();
        habilitaMotoresEAguardaEstabilizar();
        ventiladorWrite(VENTILADOR_CANAL, 0);
        fsm_emergencia = fase2;
      }
      else if (fsm_emergencia == fase2)
      {
        setBits(PIN_INTERTRAVAMENTO_OUT);
        setBits(LED_STATUS);
        voltaParaPrimeiroMenu();
        ihm.showStatus2msg(F("-----EMERGENCIA-----"));
        vTaskResume(h_eeprom);
        fsm_emergencia = fase3;
      }
      else if (fsm_emergencia == fase3)
      {
        if (checkSensorEspatulaInit())
        {
          rebobinador.move(-pulsosEspatulaRecuoInit);
          fsm_emergencia = fase4;
        }
        else
        {
          fsm_emergencia = fase5;
        }
      }
      else if (fsm_emergencia == fase4)
      {
        if (checkSensorEspatula())
        {
          rebobinador.stop();
          fsm_emergencia = fase5;
        }
      }
      else if (fsm_emergencia == fase5)
      {
        if (checkSensorHomeInit())
        {
          pulsosBracoEmergencia = posicaoBracoEmergencia * resolucao;
          braco.moveTo(-pulsosBracoEmergencia);
          fsm_emergencia = fase8;
        }
        else
        {
          braco.move(pulsosBracoMaximo);
          fsm_emergencia = fase6;
        }
      }
      else if (fsm_emergencia == fase6)
      {
        if (checkSensorHome())
        {
          posicaoBracoSensor = braco.currentPosition();
          braco.stop();
          fsm_emergencia = fase7;
        }
      }
      else if (fsm_emergencia == fase7)
      {
        if (braco.distanceToGo() == 0)
        {
          posicaoBracoReferencia = braco.currentPosition();
          posicaoBracoCorrecao = posicaoBracoReferencia - posicaoBracoSensor;
          braco.setCurrentPosition(posicaoBracoCorrecao);
          fsm_emergencia = fase8;
        }
      }
      else if (fsm_emergencia == fase8)
      {
        pulsosBracoEmergencia = posicaoBracoEmergencia * resolucao;
        braco.moveTo(-pulsosBracoEmergencia);
        fsm_emergencia = fase9;
      }
      else if (fsm_emergencia == fase9)
      {
        if (braco.distanceToGo() == 0)
        {
          desabilitaMotores();
          fsm_emergencia = fase10;
          flag_manutencao = true;
          fsm_manutencao = fase1;
        }
      }
    }

    if (fsm_old.sub_estado == MANUTENCAO_OLD)
    {
      if (fsm_manutencao == fase1)
      {
        ihm.showStatus2msg(F("-----MANUTENCAO-----"));
        liberaMenusDeManutencao();

        fsm_emergencia = fase1;
        fsm_manutencao = fase2;
      }
    }

    if (flag_emergencia == false) // Condição para sair de Parada de Emergência
    {
      if (flag_intertravamentoIn)
      {
        fsm_old.estado = ERRO_OLD;
        fsm_erro_intertravamento = fase1;
        fsm_erro_aplicacao = fase10;
        fsm_erro_impressora = fase10;
      }

      if (flag_statusImpressora)
      {
        fsm_old.estado = ERRO_OLD;
        fsm_erro_impressora = fase1;
        fsm_erro_aplicacao = fase10;
        fsm_erro_intertravamento = fase10;
      }

      if (fsm_old.sub_estado == MANUTENCAO_OLD)
      {
        bloqueiaMenusDeManutencao();
        fsm_old.sub_estado = EMERGENCIA_TOP_OLD;
        flag_manutencao = false;
      }
      else
      {
        flag_continuo = false;
        flag_manutencao = false;

        fsm_old.estado = ATIVO_OLD;
        fsm_old.sub_estado = REFERENCIANDO_INIT_OLD;

        fsm_emergencia = fase1;
        fsm_manutencao = fase1;

        fsm_referenciando_init = fase1;
      }
    }

    break;
  }
  case ATIVO_OLD:
  {
    static uint32_t timer_atrasoSensorProduto = 0;
    static uint32_t timer_etiqueta = 0;
    static uint32_t timer_ciclo = 0;
    static uint32_t timer_enableMotor = 0;
    static uint32_t timer_finalizarAplicacao = 0;
    static uint32_t timer_reinicio_espatula = 0;

    if (flag_intertravamentoIn)
    {
      fsm_old.estado = ERRO_OLD;
      fsm_erro_intertravamento = fase1;
      fsm_erro_aplicacao = fase10;
      fsm_erro_impressora = fase10;
    }

    if (flag_statusImpressora)
    {
      fsm_old.estado = ERRO_OLD;
      fsm_erro_impressora = fase1;
      fsm_erro_intertravamento = fase10;
      fsm_erro_aplicacao = fase10;
    }

    if (fsm_old.sub_estado == REFERENCIANDO_INIT_OLD)
    {
      if (fsm_referenciando_init == fase1)
      {
        motorSetup();
        habilitaMotoresEAguardaEstabilizar();

        voltaParaPrimeiroMenu();
        ihm.showStatus2msg(F("-REFERENCIANDO INIT-"));

        fsm_referenciando_init = fase2;
        fsm_referenciando_init_espatula = fase2;
        fsm_referenciando_init_motor = fase2;

        timer_enableMotor = millis();
      }

      if (fsm_referenciando_init_motor == fase2)
      {
        if (millis() - timer_enableMotor >= tempoParaEstabilizarMotorBraco)
        {
          fsm_referenciando_init_motor = fase3;
        }
      }
      else if (fsm_referenciando_init_motor == fase3)
      {
        if (checkSensorHomeInit())
        {
          braco.move(-pulsosBracoForaSensor);
          fsm_referenciando_init_motor = fase4;
        }
        else
        {
          braco.move(pulsosBracoMaximo);
          fsm_referenciando_init_motor = fase5;
        }
      }
      else if (fsm_referenciando_init_motor == fase4)
      {
        if (braco.distanceToGo() == 0)
        {
          braco.move(pulsosBracoMaximo);
          fsm_referenciando_init_motor = fase5;
        }
      }
      else if (fsm_referenciando_init_motor == fase5)
      {
        if (checkSensorHome())
        {
          posicaoBracoSensor = braco.currentPosition();
          braco.stop();
          fsm_referenciando_init_motor = fase6;
        }
      }
      else if (fsm_referenciando_init_motor == fase6)
      {
        if (braco.distanceToGo() == 0)
        {
          posicaoBracoReferencia = braco.currentPosition();
          posicaoBracoCorrecao = posicaoBracoReferencia - posicaoBracoSensor;
          braco.setCurrentPosition(posicaoBracoCorrecao);
          fsm_referenciando_init_motor = fase7;
        }
      }

      if (fsm_referenciando_init_espatula == fase2)
      {
        if (checkSensorEspatulaInit())
        {
          rebobinador.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_init_espatula = fase4;
        }
        else
        {
          rebobinador.move(pulsosEspatulaAvancoInit);
          fsm_referenciando_init_espatula = fase3;
        }
      }
      else if (fsm_referenciando_init_espatula == fase3)
      {
        if (rebobinador.distanceToGo() == 0)
        {
          rebobinador.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_init_espatula = fase4;
        }
      }
      else if (fsm_referenciando_init_espatula == fase4)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = rebobinador.currentPosition();
          rebobinador.stop();
          fsm_referenciando_init_espatula = fase5;
        }
      }
      else if (fsm_referenciando_init_espatula == fase5)
      {
        if (rebobinador.distanceToGo() == 0)
        {
          posicaoEspatulaReferencia = rebobinador.currentPosition();
          posicaoEspatulaCorrecao = posicaoEspatulaReferencia - posicaoEspatulaSensor;
          rebobinador.setCurrentPosition(posicaoEspatulaCorrecao);
          fsm_referenciando_init_espatula = fase6;
        }
      }

      else if (fsm_referenciando_init_espatula == fase6 && fsm_referenciando_init_motor == fase7)
      {
        pulsosBracoInicial = posicaoBracoInicial * resolucao;
        braco.moveTo(-pulsosBracoInicial);
        rebobinador.moveTo(pulsosEspatulaAvanco);

        ventiladorWrite(VENTILADOR_CANAL, 100);

        fsm_referenciando_init = fase3;
        fsm_referenciando_init_espatula = fase7;
        fsm_referenciando_init_motor = fase8;

        Serial.println("REFERENCIANDO INIT -- Fase Mista...");
      }

      else if (fsm_referenciando_init == fase3)
      {
        fsm_old.sub_estado = PRONTO_OLD;
        fsm_pronto_init = fase1;
        fsm_pronto_ciclo = fase4;
        fsm_referenciando_init = fase4;
      }
    }

    if (fsm_old.sub_estado == REFERENCIANDO_CICLO_OLD)
    {
      if (fsm_referenciando_ciclo == fase1)
      {
        ihm.showStatus2msg(F("REFERENCIANDO CICLO"));

        fsm_referenciando_ciclo = fase2;
        fsm_referenciando_ciclo_espatula = fase2;
        fsm_referenciando_ciclo_motor = fase2;
        timer_reinicio_espatula = millis();
      }

      if (fsm_referenciando_ciclo_motor == fase2)
      {
        pulsosBracoInicial = posicaoBracoInicial * resolucao;
        braco.moveTo(-pulsosBracoInicial);
        fsm_referenciando_ciclo_motor = fase3;
      }
      else if (fsm_referenciando_ciclo_motor == fase3)
      {
        if (braco.distanceToGo() == 0)
        {
          fsm_referenciando_ciclo_motor = fase4;
        }
      }

      if (fsm_referenciando_ciclo_espatula == fase2)
      {
        if (millis() - timer_reinicio_espatula >= tempoReinicioEspatula)
        {
          fsm_referenciando_ciclo_espatula = fase3;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase3)
      {
        if (checkSensorEspatulaInit())
        {
          rebobinador.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_ciclo_espatula = fase5;
        }
        else
        {
          rebobinador.move(pulsosEspatulaAvancoInit);
          fsm_referenciando_ciclo_espatula = fase4;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase4)
      {
        if (rebobinador.distanceToGo() == 0)
        {
          rebobinador.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_ciclo_espatula = fase5;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase5)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = rebobinador.currentPosition();
          rebobinador.stop();
          fsm_referenciando_ciclo_espatula = fase6;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase6)
      {
        if (rebobinador.distanceToGo() == 0)
        {
          posicaoEspatulaReferencia = rebobinador.currentPosition();
          posicaoEspatulaCorrecao = posicaoEspatulaReferencia - posicaoEspatulaSensor;
          rebobinador.setCurrentPosition(posicaoEspatulaCorrecao);
          fsm_referenciando_ciclo_espatula = fase7;
        }
      }

      else if (fsm_referenciando_ciclo_espatula == fase7 && fsm_referenciando_ciclo_motor == fase4)
      {
        rebobinador.moveTo(pulsosEspatulaAvanco);
        ventiladorWrite(VENTILADOR_CANAL, 100);

        fsm_referenciando_ciclo_espatula = fase8;
        Serial.println("REFERENCIANDO CICLO_OLD -- Fase MISTA...");
      }

      else if (fsm_referenciando_ciclo_espatula == fase8)
      {
        if (rebobinador.distanceToGo() == 0)
        {
          imprimirZebra();
          timer_etiqueta = millis();
          fsm_referenciando_ciclo_espatula = fase9;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase9)
      {
        if (millis() - timer_etiqueta >= atrasoImpressaoEtiqueta)
        {
          rebobinador.moveTo(-pulsosEspatulaRecuo);
          fsm_referenciando_ciclo_espatula = fase10;
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase10)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = rebobinador.currentPosition();
          rebobinador.stop();
          fsm_referenciando_ciclo_espatula = fase11;
          fsm_referenciando_ciclo_motor = fase5;
        }
      }

      else if (fsm_referenciando_ciclo_motor == fase5)
      {
        pulsosBracoAplicacao = posicaoBracoAplicacao * resolucao;
        braco.moveTo(-pulsosBracoAplicacao);
        fsm_referenciando_ciclo_motor = fase6;
      }

      else if (fsm_referenciando_ciclo == fase2)
      {
        if (fsm_referenciando_ciclo_espatula == fase11 && fsm_referenciando_ciclo_motor == fase6)
        {
          if (braco.distanceToGo() == 0 && rebobinador.distanceToGo() == 0)
          {
            fsm_old.sub_estado = PRONTO_OLD;
            fsm_pronto_ciclo = fase1;
            fsm_pronto_init = fase8;
            fsm_referenciando_ciclo = fase1;
            fsm_referenciando_ciclo_espatula = fase12;
            fsm_referenciando_ciclo_motor = fase7;
            Serial.println("REFERENCIANDO CICLO_OLD Motor -- Fim de referencia ciclo...");
          }
        }
      }
    }

    if (fsm_old.sub_estado == PRONTO_OLD)
    {
      if (fsm_pronto_init == fase1)
      {
        resetBits(LED_STATUS);
        vTaskResume(h_eeprom);
        voltaParaPrimeiroMenu();
        ihm.showStatus2msg(F("APERTE O BOTAO START"));
        fsm_pronto_init = fase2;
      }
      else if (fsm_pronto_init == fase2)
      {
        if (checkBotaoStart()) // Inicia o ciclo com o botão START acionado e entra em modo contínuo
        {
          if (rebobinador.distanceToGo() == 0)
          {
            ihm.showStatus2msg(F("APROXIMANDO BRACO..."));
            // imprimirZebra();
            timer_etiqueta = millis();
            fsm_pronto_init = fase3;
          }
        }
        else if (flag_continuo)
        {
          if (rebobinador.distanceToGo() == 0)
          {
            ihm.showStatus2msg(F("APROXIMANDO BRACO..."));
            ventiladorWrite(VENTILADOR_CANAL, 100);
            imprimirZebra();
            timer_etiqueta = millis();
            fsm_pronto_init = fase3;
          }
        }
      }
      else if (fsm_pronto_init == fase3)
      {
        if (millis() - timer_etiqueta >= atrasoImpressaoEtiqueta)
        {
          rebobinador.moveTo(-pulsosEspatulaRecuo);
          fsm_pronto_init = fase4;
        }
      }
      else if (fsm_pronto_init == fase4)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = rebobinador.currentPosition();
          rebobinador.stop();
          fsm_pronto_init = fase5;
        }
      }
      else if (fsm_pronto_init == fase5)
      {
        pulsosBracoAplicacao = posicaoBracoAplicacao * resolucao;
        braco.moveTo(-pulsosBracoAplicacao);

        fsm_pronto_init = fase6;
      }
      else if (fsm_pronto_init == fase6)
      {
        if (braco.distanceToGo() == 0)
        {
          ihm.showStatus2msg(F("AGUARDANDO PRODUTO.."));
          fsm_pronto_init = fase7;
        }
      }
      else if (fsm_pronto_init == fase7)
      {
        motorRun();
        if (checkSensorProduto())
        {
          ihm.showStatus2msg(F("--------CICLO-------"));
          fsm_old.sub_estado = CICLO_OLD;
          fsm_ciclo = fase1;
          fsm_pronto_init = fase8;
        }
      }

      if (fsm_pronto_ciclo == fase1)
      {
        Serial.print("Tempo de Ciclo: ");
        Serial.println(millis() - timer_ciclo);
        fsm_pronto_ciclo = fase2;
      }
      else if (fsm_pronto_ciclo == fase2)
      {
        ihm.showStatus2msg(F("AGUARDANDO PRODUTO.."));
        voltaParaPrimeiroMenu();
        fsm_pronto_ciclo = fase3;
      }
      else if (fsm_pronto_ciclo == fase3)
      {
        motorRun();
        if (checkSensorProduto())
        {
          ihm.showStatus2msg(F("--------CICLO-------"));
          fsm_old.sub_estado = CICLO_OLD;
          fsm_ciclo = fase1;
          fsm_pronto_ciclo = fase4;
        }
      }
    }

    if (fsm_old.sub_estado == CICLO_OLD)
    {
      if (fsm_ciclo == fase1)
      {
        vTaskSuspend(h_eeprom);
        timer_atrasoSensorProduto = millis();
        timer_ciclo = millis();
        fsm_ciclo = fase2;
      }
      else if (fsm_ciclo == fase2)
      {
        if (millis() - timer_atrasoSensorProduto >= atrasoSensorProduto)
        {
          pulsosBracoProduto = posicaoBracoProduto * resolucao;
          braco.moveTo(-pulsosBracoProduto);
          fsm_ciclo = fase3;
        }
      }
      else if (fsm_ciclo == fase3)
      {
        if (checkSensorAplicacao())
        {
          posicaoBracoDeteccaoProduto = braco.currentPosition();
          pulsosBracoEspacamento = distanciaProduto_dcmm * resolucao;
          fsm_ciclo = fase4;
        }
        else if (braco.distanceToGo() == 0)
        {
          fsm_old.estado = ERRO_OLD;
          fsm_erro_aplicacao = fase1;
          fsm_erro_intertravamento = fase10;
          fsm_erro_impressora = fase10;
        }
      }
      else if (fsm_ciclo == fase4)
      {
        if (pulsosBracoEspacamento <= pulsosRampa)
        {
          pulsosBracoEspacamento = pulsosRampa + 1000;
          fsm_ciclo = fase5;
        }
        else
        {
          fsm_ciclo = fase5;
        }
      }
      else if (fsm_ciclo == fase5)
      {
        if (braco.currentPosition() == (posicaoBracoDeteccaoProduto - (pulsosBracoEspacamento - pulsosRampa)))
        {
          braco.stop();
          fsm_ciclo = fase6;
        }
        else if (braco.distanceToGo() == 0)
        {
          fsm_old.estado = ERRO_OLD;
          fsm_erro_aplicacao = fase1;
          fsm_erro_intertravamento = fase10;
          fsm_erro_impressora = fase10;
        }
      }
      else if (fsm_ciclo == fase6)
      {
        if (braco.distanceToGo() == 0)
        {
          // ventiladorWrite(VENTILADOR_CANAL, 25);
          fsm_ciclo = fase7;
        }
      }
      else if (fsm_ciclo == fase7)
      {
        incrementaContadores();
        fsm_ciclo = fase8;
        timer_finalizarAplicacao = millis();
      }
      else if (fsm_ciclo == fase8)
      {
        if (millis() - timer_finalizarAplicacao >= tempoFinalizarAplicacao)
        {
          fsm_old.sub_estado = REFERENCIANDO_CICLO_OLD;
          fsm_referenciando_ciclo = fase1;
          fsm_ciclo = fase9;
        }
      }
    }

    // Condição para sair de ATIVO_OLD:
    if (flag_emergencia == true)
    {
      fsm_old.estado = PARADA_EMERGENCIA_OLD;
      fsm_old.sub_estado = EMERGENCIA_TOP_OLD;
    }
    // Condição para sair de ATIVO_OLD:
    break;
  }
  case ERRO_OLD:
  {
    piscaLedStatus();

    if (fsm_erro_aplicacao == fase1)
    {
      ihm.showStatus2msg(F("PRODUTO N ENCONTRADO"));
      ventiladorWrite(VENTILADOR_CANAL, 0);
      fsm_erro_aplicacao = fase2;
      Serial.println("Erro de Aplicacao...");
    }
    else if (fsm_erro_aplicacao == fase2)
    {
      fsm_old.estado = ATIVO_OLD;
      fsm_old.sub_estado = REFERENCIANDO_INIT_OLD;
      fsm_referenciando_init = fase1;
      fsm_erro_aplicacao = fase4;
      flag_continuo = true;
    }

    if (fsm_erro_impressora == fase1)
    {
      ihm.showStatus2msg(F("IMPRESSORA COM ERRO"));
      ventiladorWrite(VENTILADOR_CANAL, 0);
      fsm_erro_impressora = fase2;
      Serial.println("IMPRESSORA DESABILITADA OU COM ERRO...");
    }
    else if (fsm_erro_impressora == fase2)
    {
      if (flag_statusImpressora == false)
      {
        fsm_old.estado = ATIVO_OLD;
        fsm_old.sub_estado = REFERENCIANDO_INIT_OLD;
        fsm_referenciando_init = fase1;
        fsm_erro_impressora = fase4;
      }
    }

    if (fsm_erro_intertravamento == fase1)
    {
      braco.stop();
      rebobinador.stop();
      desabilitaMotores();
      ventiladorWrite(VENTILADOR_CANAL, 0);
      fsm_erro_intertravamento = fase2;
    }
    else if (fsm_erro_intertravamento == fase2)
    {
      if (flag_intertravamentoIn)
      {
        ihm.showStatus2msg(F("INTERTRAV. DESCONECT"));
        fsm_erro_intertravamento = fase3;
      }
    }
    else if (fsm_erro_intertravamento == fase3)
    {
      if (flag_intertravamentoIn == false)
      {
        fsm_old.estado = ATIVO_OLD;
        fsm_old.sub_estado = REFERENCIANDO_INIT_OLD;
        fsm_referenciando_init = fase1;
        fsm_erro_intertravamento = fase4;
      }
    }

    // Condição para sair do ERRO_OLD:
    if (flag_emergencia == true)
    {
      fsm_old.estado = PARADA_EMERGENCIA_OLD;
      fsm_old.sub_estado = EMERGENCIA_TOP_OLD;
    }
    // Condição para sair do ERRO_OLD:
    break;
  }
  case ESTADO_TESTE_DE_IMPRESSAO:
  {
    static uint32_t fsm_substate = fase1;

    if (fsm_substate == fase1)
    {
      if (sensorDeProdutoOuStart.checkPulse())
      {
        Serial.println("SP");
        xTaskCreatePinnedToCore(t_printEtiqueta, "print task", 1024, NULL, PRIORITY_2, NULL, CORE_0); // createTaskPrint();
        fsm_substate = fase2;
      }
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_IMPRESSAO_CONCLUIDA)
      {
        fsm_substate = fase1;
      }
      else if (evento == EVT_FALHA)
      {
        Serial.println("erro impressao");
        fsm_substate = fase1;
      }
    }
    else if (fsm_substate == fase3)
    {
    }
    break;
  }
  }
}
