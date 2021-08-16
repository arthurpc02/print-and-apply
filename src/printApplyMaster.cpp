// Projeto Print & Apply Linear - Software desenvolvido para a placa industrial v2.0 comunicando com a IHM - v1.0

// Funções Print & Apply - v1.0

#include <printApplyFunctions.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Projeto Print & Apply - ESP_32 v_1.0");

  mtx_ios = xSemaphoreCreateMutex();

  EEPROM.begin(EEPROM_SIZE);
  restoreBackupParameters();
  // presetEEPROM();

  desligaTodosOutput();
  enableOutput();

  createTasks();

  pin_mode();
  ventiladorConfig();
  motorSetup();

  Serial.println("End Setup. Print & Apply Linear.");
}

void loop()
{
  motor.run();
  motor_espatula.run();

  // Ciclo:

  switch (fsm.estado)
  {
  case PARADA_EMERGENCIA:
  {
    if (fsm.sub_estado == EMERGENCIA)
    {
      if (fsm_emergencia == fase1)
      {
        motor.stop();
        motor_espatula.stop();
        motorDisable();
        ventiladorWrite(VENTILADOR_CANAL, 0);

        fsm_emergencia = fase2;
        fsm_manutencao = fase1;
      }
      else if (fsm_emergencia == fase2)
      {
        setBits(PIN_INTERTRAVAMENTO_OUT);
        setBits(LED_STATUS);

        fsm_emergencia = fase3;
      }
      else if (fsm_emergencia == fase3)
      {
        ihm.focus(&menu_produto);
        ihm.showStatus2msg(F("-----EMERGENCIA-----"));
        vTaskResume(h_eeprom);
        fsm_emergencia = fase4;
      }
    }

    if (fsm.sub_estado == MANUTENCAO)
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
        fsm.estado = ERRO;
        fsm_erro_intertravamento = fase1;
        fsm_erro_aplicacao = fase10;
        fsm_erro_impressora = fase10;
      }

      if (flag_statusImpressora)
      {
        fsm.estado = ERRO;
        fsm_erro_impressora = fase1;
        fsm_erro_aplicacao = fase10;
        fsm_erro_intertravamento = fase10;
      }

      if (fsm.sub_estado == MANUTENCAO)
      {
        bloqueiaMenusDeManutencao();
        fsm.sub_estado = EMERGENCIA;
      }
      else
      {
        flag_continuo = false;

        fsm.estado = ATIVO;
        fsm.sub_estado = REFERENCIANDO_INIT;

        fsm_emergencia = fase1;
        fsm_manutencao = fase1;

        fsm_referenciando_init = fase1;
      }
    }

    break;
  }

  case ATIVO:
  {
    static uint32_t timer_atrasoSensorProduto = 0;
    static uint32_t timer_etiqueta = 0;
    static uint32_t timer_ciclo = 0;
    static uint32_t timer_enableMotor = 0;
    static uint32_t timer_finalizarAplicacao = 0;
    static uint32_t timer_reinicio_espatula = 0;

    if (flag_intertravamentoIn)
    {
      fsm.estado = ERRO;
      fsm_erro_intertravamento = fase1;
      fsm_erro_aplicacao = fase10;
      fsm_erro_impressora = fase10;
    }

    if (flag_statusImpressora)
    {
      fsm.estado = ERRO;
      fsm_erro_impressora = fase1;
      fsm_erro_intertravamento = fase10;
      fsm_erro_aplicacao = fase10;
    }

    if (fsm.sub_estado == REFERENCIANDO_INIT)
    {
      if (fsm_referenciando_init == fase1)
      {
        motorSetup();
        motorEnable();

        ihm.focus(&menu_contador);
        ihm.showStatus2msg(F("-REFERENCIANDO INIT-"));

        fsm_referenciando_init = fase2;
        fsm_referenciando_init_espatula = fase2;
        fsm_referenciando_init_motor = fase2;

        timer_enableMotor = millis();

        Serial.println("REFERENCIANDO INIT -- Fase 1...");
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
          motor.move(-pulsosBracoForaSensor);
          fsm_referenciando_init_motor = fase4;
          Serial.println("REFERENCIANDO INIT -- Fase 3 - Motor 1...");
        }
        else
        {
          motor.move(pulsosBracoMaximo);
          fsm_referenciando_init_motor = fase5;
          Serial.println("REFERENCIANDO INIT -- Fase 3 - Motor 2...");
        }
      }
      else if (fsm_referenciando_init_motor == fase4)
      {
        if (motor.distanceToGo() == 0)
        {
          motor.move(pulsosBracoMaximo);
          fsm_referenciando_init_motor = fase5;
          Serial.println("REFERENCIANDO INIT -- Fase 4 - Motor...");
        }
      }
      else if (fsm_referenciando_init_motor == fase5)
      {
        if (checkSensorHome())
        {
          posicaoBracoSensor = motor.currentPosition();
          motor.stop();
          fsm_referenciando_init_motor = fase6;
          Serial.println("REFERENCIANDO INIT -- Fase 5 - Motor...");
        }
      }
      else if (fsm_referenciando_init_motor == fase6)
      {
        if (motor.distanceToGo() == 0)
        {
          posicaoBracoReferencia = motor.currentPosition();
          posicaoBracoCorrecao = posicaoBracoReferencia - posicaoBracoSensor;
          motor.setCurrentPosition(posicaoBracoCorrecao);
          fsm_referenciando_init_motor = fase7;
          Serial.print("Posicao 0: ");
          Serial.println(posicaoBracoCorrecao);
          Serial.println("REFERENCIANDO INIT -- Fase 6 - Motor...");
        }
      }

      if (fsm_referenciando_init_espatula == fase2)
      {
        if (checkSensorEspatulaInit())
        {
          motor_espatula.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_init_espatula = fase4;
          Serial.println("REFERENCIANDO INIT -- Fase 2-1...");
        }
        else
        {
          motor_espatula.move(pulsosEspatulaAvancoInit);
          fsm_referenciando_init_espatula = fase3;
          Serial.println("REFERENCIANDO INIT -- Fase 2-2...");
        }
      }
      else if (fsm_referenciando_init_espatula == fase3)
      {
        if (motor_espatula.distanceToGo() == 0)
        {
          motor_espatula.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_init_espatula = fase4;
          Serial.println("REFERENCIANDO INIT -- Fase 3...");
        }
      }
      else if (fsm_referenciando_init_espatula == fase4)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = motor_espatula.currentPosition();
          motor_espatula.stop();
          fsm_referenciando_init_espatula = fase5;
          Serial.println("REFERENCIANDO INIT -- Fase 4...");
        }
      }
      else if (fsm_referenciando_init_espatula == fase5)
      {
        if (motor_espatula.distanceToGo() == 0)
        {
          posicaoEspatulaReferencia = motor_espatula.currentPosition();
          posicaoEspatulaCorrecao = posicaoEspatulaReferencia - posicaoEspatulaSensor;
          motor_espatula.setCurrentPosition(posicaoEspatulaCorrecao);
          fsm_referenciando_init_espatula = fase6;
          Serial.println("REFERENCIANDO INIT -- Fase 5...");
        }
      }

      else if (fsm_referenciando_init_espatula == fase6 && fsm_referenciando_init_motor == fase7)
      {
        pulsosBracoInicial = posicaoBracoInicial * resolucao;
        motor.moveTo(-pulsosBracoInicial);
        motor_espatula.moveTo(pulsosEspatulaAvanco);

        ventiladorWrite(VENTILADOR_CANAL, 50);

        fsm_referenciando_init = fase3;
        fsm_referenciando_init_espatula = fase7;
        fsm_referenciando_init_motor = fase8;

        Serial.println("REFERENCIANDO INIT -- Fase Mista...");
      }

      else if (fsm_referenciando_init == fase3)
      {
        fsm.sub_estado = PRONTO;
        fsm_pronto_init = fase1;
        fsm_pronto_ciclo = fase4;
        fsm_referenciando_init = fase4;
        Serial.println("REFERENCIANDO INIT -- End ref...");
      }
    }

    if (fsm.sub_estado == REFERENCIANDO_CICLO)
    {
      if (fsm_referenciando_ciclo == fase1)
      {
        ihm.showStatus2msg(F("REFERENCIANDO CICLO"));

        fsm_referenciando_ciclo = fase2;
        fsm_referenciando_ciclo_espatula = fase2;
        fsm_referenciando_ciclo_motor = fase2;
        Serial.println("REFERENCIANDO CICLO -- Fase 1...");
      }

      if (fsm_referenciando_ciclo_motor == fase2)
      {
        motor.move(pulsosBracoMaximo);
        fsm_referenciando_ciclo_motor = fase3;
        Serial.println("REFERENCIANDO CICLO Motor -- Fase 2 - Motor 2...");
      }
      else if (fsm_referenciando_ciclo_motor == fase3)
      {
        if (checkSensorHome())
        {
          posicaoBracoSensor = motor.currentPosition();
          motor.stop();
          fsm_referenciando_ciclo_motor = fase4;
          Serial.println("REFERENCIANDO CICLO Motor -- Fase 3...");
        }
      }
      else if (fsm_referenciando_ciclo_motor == fase4)
      {
        if (motor.distanceToGo() == 0)
        {
          posicaoBracoReferencia = motor.currentPosition();
          posicaoBracoCorrecao = posicaoBracoReferencia - posicaoBracoSensor;
          motor.setCurrentPosition(posicaoBracoCorrecao);
          fsm_referenciando_ciclo_motor = fase5;
          Serial.println("REFERENCIANDO CICLO Motor -- Fase 4...");
        }
      }

      if (fsm_referenciando_ciclo_espatula == fase2)
      {
        if (millis() - timer_reinicio_espatula >= tempoReinicioEspatula)
        {
          fsm_referenciando_ciclo_espatula = fase3;
        }
      }
      if (fsm_referenciando_ciclo_espatula == fase3)
      {
        if (checkSensorEspatulaInit())
        {
          motor_espatula.move(-pulsosEspatulaRecuoInit);
          fsm_referenciando_ciclo_espatula = fase5;
          Serial.println("REFERENCIANDO CICLO -- Fase 3 - 1...");
        }
        else
        {
          motor_espatula.move(pulsosEspatulaAvancoInit);
          fsm_referenciando_ciclo_espatula = fase4;
          Serial.println("REFERENCIANDO CICLO -- Fase 3 - 2...");
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase4)
      {
        if (motor_espatula.distanceToGo() == 0)
        {
          motor_espatula.move(-(pulsosEspatulaRecuoInit));
          fsm_referenciando_ciclo_espatula = fase5;
          Serial.println("REFERENCIANDO CICLO -- Fase 3...");
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase5)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = motor_espatula.currentPosition();
          motor_espatula.stop();
          fsm_referenciando_ciclo_espatula = fase6;
          Serial.println("REFERENCIANDO CICLO -- Fase 4...");
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase6)
      {
        if (motor_espatula.distanceToGo() == 0)
        {
          posicaoEspatulaReferencia = motor_espatula.currentPosition();
          posicaoEspatulaCorrecao = posicaoEspatulaReferencia - posicaoEspatulaSensor;
          motor_espatula.setCurrentPosition(posicaoEspatulaCorrecao);
          fsm_referenciando_ciclo_espatula = fase7;
          Serial.println("REFERENCIANDO CICLO -- Fase 5...");
        }
      }

      else if (fsm_referenciando_ciclo_espatula == fase7 && fsm_referenciando_ciclo_motor == fase5)
      {
        pulsosBracoInicial = posicaoBracoInicial * resolucao;
        motor.moveTo(-pulsosBracoInicial);
        motor_espatula.moveTo(pulsosEspatulaAvanco);

        ventiladorWrite(VENTILADOR_CANAL, 100);
        imprimirZebra();
        fsm_referenciando_ciclo_espatula = fase8;
        Serial.println("REFERENCIANDO CICLO -- Fase MISTA...");
      }

      else if (fsm_referenciando_ciclo_espatula == fase8)
      {
        if (motor_espatula.distanceToGo() == 0)
        {
          timer_etiqueta = millis();
          fsm_referenciando_ciclo_espatula = fase9;
          Serial.println("REFERENCIANDO CICLO - FASE 7 ...");
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase9)
      {
        if (millis() - timer_etiqueta >= atrasoImpressaoEtiqueta)
        {
          motor_espatula.moveTo(-pulsosEspatulaRecuo);
          fsm_referenciando_ciclo_espatula = fase10;
          Serial.println("REFERENCIANDO CICLO -- Fase 8...");
        }
      }
      else if (fsm_referenciando_ciclo_espatula == fase10)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = motor_espatula.currentPosition();
          motor_espatula.stop();
          fsm_referenciando_ciclo_espatula = fase11;
          fsm_referenciando_ciclo_motor = fase6;
          Serial.println("REFERENCIANDO CICLO -- Fase 9...");
        }
      }

      else if (fsm_referenciando_ciclo_motor == fase6)
      {
        pulsosBracoAplicacao = posicaoBracoAplicacao * resolucao;
        motor.moveTo(-pulsosBracoAplicacao);
        fsm_referenciando_ciclo_motor = fase7;
        Serial.println("REFERENCIANDO CICLO Motor -- Fase 6...");
      }

      else if (fsm_referenciando_ciclo == fase2)
      {
        if (fsm_referenciando_ciclo_espatula == fase11 && fsm_referenciando_ciclo_motor == fase7)
        {
          if (motor.distanceToGo() == 0 && motor_espatula.distanceToGo() == 0)
          {
            fsm.sub_estado = PRONTO;
            fsm_pronto_ciclo = fase1;
            fsm_referenciando_ciclo = fase1;
            fsm_referenciando_ciclo_espatula = fase12;
            fsm_referenciando_ciclo_motor = fase8;
            Serial.println("REFERENCIANDO CICLO Motor -- Fim de referencia ciclo...");
          }
        }
      }
    }

    if (fsm.sub_estado == PRONTO)
    {
      if (fsm_pronto_init == fase1)
      {
        resetBits(LED_STATUS);
        vTaskResume(h_eeprom);
        ihm.focus(&menu_contador);
        ihm.showStatus2msg(F("APERTE O BOTAO START"));
        fsm_pronto_init = fase2;
        Serial.println("PRONTO INIT -- FASE 1...");
      }
      else if (fsm_pronto_init == fase2)
      {
        if (checkBotaoStart()) // Inicia o ciclo com o botão START acionado e entra em modo contínuo
        {
          if (motor_espatula.distanceToGo() == 0)
          {
            ihm.showStatus2msg(F("APROXIMANDO BRACO..."));
            ventiladorWrite(VENTILADOR_CANAL, 100);
            imprimirZebra();
            timer_etiqueta = millis();
            fsm_pronto_init = fase3;
            Serial.println("PRONTO INIT -- FASE 2...");
          }
        }
        else if (flag_continuo)
        {
          if (motor_espatula.distanceToGo() == 0)
          {
            ihm.showStatus2msg(F("APROXIMANDO BRACO..."));
            ventiladorWrite(VENTILADOR_CANAL, 100);
            imprimirZebra();
            timer_etiqueta = millis();
            fsm_pronto_init = fase3;
            Serial.println("PRONTO INIT -- FASE 2 CONTINUO...");
          }
        }
      }
      else if (fsm_pronto_init == fase3)
      {
        if (millis() - timer_etiqueta >= atrasoImpressaoEtiqueta)
        {
          motor_espatula.moveTo(-pulsosEspatulaRecuo);
          fsm_pronto_init = fase4;
          Serial.println("PRONTO INIT -- FASE 3...");
        }
      }
      else if (fsm_pronto_init == fase4)
      {
        if (checkSensorEspatula())
        {
          posicaoEspatulaSensor = motor_espatula.currentPosition();
          motor_espatula.stop();
          fsm_pronto_init = fase5;
          Serial.println("PRONTO INIT -- FASE 4...");
        }
      }
      else if (fsm_pronto_init == fase5)
      {
        pulsosBracoAplicacao = posicaoBracoAplicacao * resolucao;
        motor.moveTo(-pulsosBracoAplicacao);

        fsm_pronto_init = fase6;
        Serial.println("PRONTO INIT -- FASE 5...");
      }
      else if (fsm_pronto_init == fase6)
      {
        if (motor.distanceToGo() == 0)
        {
          ihm.showStatus2msg(F("AGUARDANDO PRODUTO.."));
          fsm_pronto_init = fase7;
          Serial.println("PRONTO INIT -- FASE 6...");
        }
      }
      else if (fsm_pronto_init == fase7)
      {
        motorRun();
        if (checkSensorProduto())
        {
          ihm.showStatus2msg(F("--------CICLO-------"));
          fsm.sub_estado = CICLO;
          fsm_ciclo = fase1;
          fsm_pronto_init = fase8;
          Serial.println("PRONTO INIT -- FASE 7...");
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
        ihm.focus(&menu_contador);
        fsm_pronto_ciclo = fase3;
      }
      else if (fsm_pronto_ciclo == fase3)
      {
        motorRun();
        if (checkSensorProduto())
        {
          ihm.showStatus2msg(F("--------CICLO-------"));
          fsm.sub_estado = CICLO;
          fsm_ciclo = fase1;
          fsm_pronto_ciclo = fase4;
          Serial.println("PRONTO CICLO -- FASE 3...");
        }
      }
    }

    if (fsm.sub_estado == CICLO)
    {
      if (fsm_ciclo == fase1)
      {
        vTaskSuspend(h_eeprom);
        timer_atrasoSensorProduto = millis();
        timer_ciclo = millis();
        fsm_ciclo = fase2;
        Serial.println("CICLO FASE 1...");
      }
      else if (fsm_ciclo == fase2)
      {
        if (millis() - timer_atrasoSensorProduto >= atrasoSensorProduto)
        {
          pulsosBracoProduto = posicaoBracoProduto * resolucao;
          motor.moveTo(-pulsosBracoProduto);
          fsm_ciclo = fase3;
          Serial.println("CICLO FASE 2...");
        }
      }
      else if (fsm_ciclo == fase3)
      {
        if (checkSensorAplicacao())
        {
          posicaoBracoDeteccaoProduto = motor.currentPosition();
          pulsosBracoEspacamento = espacamentoProdutomm * resolucao;
          fsm_ciclo = fase4;
          Serial.println("CICLO FASE 3...");
        }
        else if (motor.distanceToGo() == 0)
        {
          fsm.estado = ERRO;
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

          Serial.println("CICLO FASE 4...");
          fsm_ciclo = fase5;
        }
        else
        {
          Serial.println("CICLO FASE 4...");
          fsm_ciclo = fase5;
        }
      }
      else if (fsm_ciclo == fase5)
      {
        if (motor.currentPosition() == (posicaoBracoDeteccaoProduto - (pulsosBracoEspacamento - pulsosRampa)))
        {
          motor.stop();
          fsm_ciclo = fase6;
          Serial.println("CICLO FASE 5...");
        }
        else if (motor.distanceToGo() == 0)
        {
          fsm.estado = ERRO;
          fsm_erro_aplicacao = fase1;
          fsm_erro_intertravamento = fase10;
          fsm_erro_impressora = fase10;
        }
      }
      else if (fsm_ciclo == fase6)
      {
        if (motor.distanceToGo() == 0)
        {
          ventiladorWrite(VENTILADOR_CANAL, 25);
          fsm_ciclo = fase7;
          Serial.println("CICLO FASE 6...");
        }
      }
      else if (fsm_ciclo == fase7)
      {
        incrementaContador();
        fsm_ciclo = fase8;
        Serial.println("CICLO FASE 7...");
        timer_finalizarAplicacao = millis();
      }
      else if (fsm_ciclo == fase8)
      {
        if (millis() - timer_finalizarAplicacao >= tempoFinalizarAplicacao)
        {
          fsm.sub_estado = REFERENCIANDO_CICLO;
          fsm_referenciando_ciclo = fase1;
          fsm_ciclo = fase9;
          Serial.println("CICLO FASE 8...");
        }
      }
    }

    // Condição para sair de ATIVO:
    if (flag_emergencia == true)
    {
      fsm.estado = PARADA_EMERGENCIA;
      fsm.sub_estado = EMERGENCIA;
    }
    // Condição para sair de ATIVO:
    break;
  }

  case ERRO:
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
      fsm.estado = ATIVO;
      fsm.sub_estado = REFERENCIANDO_INIT;
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
        fsm.estado = ATIVO;
        fsm.sub_estado = REFERENCIANDO_INIT;
        fsm_referenciando_init = fase1;
        fsm_erro_impressora = fase4;
      }
    }

    if (fsm_erro_intertravamento == fase1)
    {
      motor.stop();
      motor_espatula.stop();
      motorDisable();
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
        fsm.estado = ATIVO;
        fsm.sub_estado = REFERENCIANDO_INIT;
        fsm_referenciando_init = fase1;
        fsm_erro_intertravamento = fase4;
      }
    }

    // Condição para sair do ERRO:
    if (flag_emergencia == true)
    {
      fsm.estado = PARADA_EMERGENCIA;
      fsm.sub_estado = EMERGENCIA;
    }
    // Condição para sair do ERRO:
    break;
  }
  }
}