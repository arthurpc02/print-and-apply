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

  sensorDeProdutoOuStart.setup();
  sinalPrintEnd.setup();
  sensorAplicacao.setup();
  sensorHome.setup();
  sunnyVision_A.setup();
  ventiladorSetup();

  extIOs.init();
  desligaTodosOutputs();

  createTasks();

  pinInitialization();
  // ventiladorConfig();
  Serial.print("p/dcmm braco: ");
  Serial.println(resolucao);
  braco_setup(velocidadeDeTrabalho_dcmm, rampaReferenciacao_dcmm);
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
      // vTaskSuspend(h_filaDoSunnyVision);
      desabilitaMotores();
      desligaTodosOutputs();
      vTaskResume(h_eeprom);
      ihm.showStatus2msg("EMERGENCIA OU PORTA");
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
      delay(100); // pra garantir que vai printar EMERGENCIA na ihm antes de desligar a Serial.
      ihm.liquidC.endSerial(); // a serial está sendo desligada durante o emergencia para garantir que o pc não terá concorrência para comunicar com a impressora SATO.
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_PARADA_EMERGENCIA)
      {
        imprimeFalhaNaIhm();
        timer_emergencia = millis();
      }
      else if (millis() - timer_emergencia > timeout_emergencia)
      {
        ihm.desligaLEDvermelho();
        // changeFsmState(ESTADO_TESTE_DE_IMPRESSAO);
        clearAllFaults();
        changeFsmState(ESTADO_STOP);
        ihm.liquidC.init();
        flag_zeraBotoes = true;
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
      delay(500);
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
        if (checkFault(0))
        {
          changeFsmState(ESTADO_FALHA);
          break;
        }
        // vTaskSuspend(h_filaDoSunnyVision);
        ihm.desligaLEDverde();
        delay(1);
        ihm.showStatus2msg("EM PAUSA");
        Serial.println("ESTADO STOP");
        desligaVentilador();
        torre_ligaLuzVerde();
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
        braco_setup(velocidadeDeTrabalho_dcmm, rampaReferenciacao_dcmm);
        rebobinador_setup(velocidadeRebobinador, aceleracaoRebobinador);
        torre_ligaLuzVermelha();

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
          // vTaskResume(h_filaDoSunnyVision);
          changeFsmState(ESTADO_POSICIONANDO);
          // changeFsmState(ESTADO_TESTE_COMUNICACAO);
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
    const int16_t tempoExtraAguardandoEtiqueta = 200; // ms

    if (evento == EVT_PARADA_EMERGENCIA)
    {
      changeFsmState(ESTADO_EMERGENCIA);
      break;
    }
    else if (checkFault(0))
    {
      if (checkFault(FALHA_IMPRESSORA))
      {
        // não faz nada
      }
      else
      {
        changeFsmState(ESTADO_STOP);
        break;
      }
    }

    if (evento == EVT_PLAY_PAUSE)
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
          delay(tempoExtraAguardandoEtiqueta);
          braco_moveTo(posicaoDeAguardarProduto_dcmm);
          fsm_substate = fase4;
        }
      }
      else if (evento == EVT_FALHA)
      {
        setFault(FALHA_IMPRESSAO);
        Serial.println("falha impressora");
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
    else if (checkFault(0))
    {
      if (checkFault(FALHA_IMPRESSORA))
      {
        // não faz nada
      }
      else
      {
        changeFsmState(ESTADO_STOP);
        break;
      }
    }

    if (evento == EVT_PLAY_PAUSE)
    {
      flag_pause = true;
      break;
    }

    if (fsm_substate == fase1)
    {
      if (sensorDeProdutoOuStart.checkPulse() || evento == EVT_HOLD_PLAY_PAUSE)
      {
        preparaAplicacaoDependendoDoProduto();
        timer_atrasoSensorProduto = millis(); //
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
          // to do: provavelmente haverá colisão nesse caso aqui, então o ideal seria nao permitir configurar a distancia do produto menor que a rampa.
          braco.stop();
        }
        else
        {
          // atualiza a distancia do braco para que ele pare bem em cima do produto. (se o parâmetro tiver sido configurado corretamente)
          braco_move(distanciaProduto_dcmm);
        }
        fsm_substate = fase4;
      }
      else if (braco.distanceToGo() == 0)
      {
        setFault(FALHA_APLICACAO);
        Serial.println("erro de aplicação");
        changeFsmState(ESTADO_STOP);
      }
    }
    else if (fsm_substate == fase4)
    {
      if (braco.distanceToGo() == 0)
      {
        timer_finalizaAplicacao = millis();
        fsm_substate = fase5;
        desligaVentilador();
        incrementaContadores();
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
    else if (checkFault(0))
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (evento == EVT_PLAY_PAUSE)
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (fsm_substate == fase1)
    {
      braco_setup(velocidadeDeReferenciacao_dcmm, rampaReferenciacao_dcmm);
      xTaskCreatePinnedToCore(t_rebobina, "task rebobina", 2048, NULL, PRIORITY_1, NULL, CORE_0);
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

    if (evento == EVT_PLAY_PAUSE)
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
    else if (checkFault(0))
    {
      changeFsmState(ESTADO_STOP);
      break;
    }

    if (evento == EVT_PLAY_PAUSE)
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

    if (evento == EVT_PLAY_PAUSE)
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
  case ESTADO_TESTE_COMUNICACAO:
  {
    if (fsm_substate == fase1)
    {
      enviaMensagemDeTesteParaImpressora();
      ihm.showStatus2msg("Enviou msg");
      fsm_substate = fase2;
    }
    else if (fsm_substate == fase2)
    {
      if (evento == EVT_MENSAGEM_ENVIADA)
      {
        Serial.println("fim do teste de mensagem");
        changeFsmState(ESTADO_STOP);
      }
    }
    break;
  }
  }
}
