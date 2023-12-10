// - - - - - - - - - - - - - - - - - - -
//Variáveis de Controle do Processo
// - - - - - - - - - - - - - - - - - - -
bool ATIVAR = LOW;      // Relês ligam em LOW
bool DESATIVAR = HIGH;  // e desligam em HIGH

bool btn_start = 0;
bool btn_stop = 0;

// Espera um tempo antes de liberar a próxima ação para dar tempo de desligar a resposta do robô
// Grava o momento em que o movimento iniciou e espera o tempo definido em tempoEsperaProxMovimento (em milisegundos)
unsigned int tempoEsperaProxMovimento = 1000;
unsigned long tempoAtivacaoMovimentoAtual = 0;

// Espera um tempo antes de bloquear a leitura para preparar os bits para o próximo movimento do robô
// Grava o momento em que o movimento iniciou e espera o tempo definido em tempoEsperaLeituraBits (em milisegundos)
unsigned int tempoEsperaLeituraBits = 1000;
unsigned long tempoLeituraBitsAtual = 0;

// Espera um tempo antes de enviar as informações atualizadas da Magazine e das cahas da Esteira para o Esp32
// Grava o momento em que o movimento iniciou e espera o tempo definido em tempoEsperaEnviaDados (em milisegundos)
unsigned int tempoEsperaEnviaDados = 500;
unsigned long tempoEnvioDadosAtual = 0;

bool novosDadosMagazine = false;
bool novosDadosEsteira = false;

byte pos = 1;           // Posição Inicial
byte prateleira_atual;  //Prateleira correspondente ao tipo da peça movimentada
byte calha_atual;       //Calha da Esteira correspondente ao tipo da peça movimentada

//Setorizações
byte setorizacao = 0;  // Confirma que os movimentos do robo vão acontecer na ordem certa
byte ciclo_pecas = 0;  // Tipo de peça/prateleira a ser movimentada no momento
//0 = peças vermelhas / prat 1
//1 = peças pretas    / prat 2
//2 = peças metálicas / prat 3
//3 = acabou
bool fim_movimentacao_pecas = false;    //Guarda quando uma movimentação de peças (colocar ou remover) acaba
bool inicio_peca = true;                // Informa se é a primeira peça do grupo a ser movimentada
byte movimento_atual_processo = 0;      // controla em qual tipo de movimento o processo está atualmente
bool libera_mudanca_movimento = false;  // se verdadeiro, libera alterar o tipo de movimento (colocar ou remover peças) quando faz ciclos de movimento
bool interrompeCiclos = false;          // interrompe o funcionamento dos ciclos para finalizar o processo no ciclo atual

void setupControleRobo() {
  pinMode(pino_resposta, INPUT_PULLUP);
  pinMode(pino_hab_movimento, OUTPUT);
  pinMode(pino_bit_5, OUTPUT);
  pinMode(pino_bit_4, OUTPUT);
  pinMode(pino_bit_3, OUTPUT);
  pinMode(pino_bit_2, OUTPUT);
  pinMode(pino_bit_1, OUTPUT);
  pinMode(pino_sinal_esteira, OUTPUT);

  digitalWrite(pino_hab_movimento, DESATIVAR);
  digitalWrite(pino_sinal_esteira, DESATIVAR);

  digitalWrite(pino_bit_5, DESATIVAR);
  digitalWrite(pino_bit_4, DESATIVAR);
  digitalWrite(pino_bit_3, DESATIVAR);
  digitalWrite(pino_bit_2, DESATIVAR);
  digitalWrite(pino_bit_1, DESATIVAR);

  randomSeed(analogRead(0));
};

//Controla os ciclos que o processo vai realizar com base no valor de n_ciclos
void processoControleRobo() {
  if (start) {
    processo_em_andamento = true;
    ciclo_atual = 101;
    movimento_atual_processo = 1;
    start = false;
  }

  if (stop) {
    ciclo_atual = 1;
    interrompeCiclos = true;
    stop = false;
  }

  if (processo_em_andamento) {
    //Sem Ciclos
    if (n_ciclos == 150) {
      processoTipoMovimento(false);

      if (libera_mudanca_movimento) {
        interrompeCiclos = false;
        processo_em_andamento = false;
        libera_mudanca_movimento = false;
        pos = 1;
        setorizacao = 0;
        ciclo_pecas = 0;
        fim_movimentacao_pecas = false;
        inicio_peca = true;
        movimento_atual_processo = 0;
      }
    }

    //Ciclos definidos por n_ciclos
    else if ((n_ciclos > 0 && n_ciclos <= 100) || interrompeCiclos == true) {

      if (ciclo_atual == 101) {
        ciclo_atual = n_ciclos;

        Serial.println("");

        Serial.print("Movimento atual do processo: ");
        Serial.println(movimento_atual_processo);

        Serial.print("Ciclo Atual: ");
        Serial.println(ciclo_atual);

        Serial.println("");
      }

      if (ciclo_atual > 0) {
        switch (movimento_atual_processo) {
          case 1:
            //Tipo_Movimento = 'c';
            processoTipoMovimento(true);
            break;
          case 2:
            movimento_atual_processo = 3;
            fim_movimentacao_pecas = false;
            break;
          case 3:
            //Tipo_Movimento = 'r';
            processoTipoMovimento(true);
            break;
          case 4:
            movimento_atual_processo = 1;
            ciclo_atual--;
            fim_movimentacao_pecas = false;

            Serial.println("");

            Serial.print("Movimento atual do processo: ");
            Serial.println(movimento_atual_processo);

            Serial.print("Ciclo Atual: ");
            Serial.println(ciclo_atual);

            Serial.println("");
            break;
        }
      } else {
        //Fim dos Ciclos - Reseta todas as variáveis de controle
        interrompeCiclos = false;
        processo_em_andamento = false;
        libera_mudanca_movimento = false;

        pos = 1;
        setorizacao = 0;
        ciclo_pecas = 0;
        fim_movimentacao_pecas = false;
        inicio_peca = true;
        movimento_atual_processo = 0;
      }
    }

    //Ciclos infinitos
    else if (n_ciclos == 200 && interrompeCiclos == false) {

      if (ciclo_atual == 101) {
        ciclo_atual = n_ciclos;

        Serial.println("");

        Serial.print("Movimento atual do processo: ");
        Serial.println(movimento_atual_processo);

        Serial.print("Ciclo Atual: ");
        Serial.println(ciclo_atual);

        Serial.println("");
      }

      if (ciclo_atual == 200) {
        switch (movimento_atual_processo) {
          case 1:
            //Tipo_Movimento = 'c';
            processoTipoMovimento(true);
            break;
          case 2:
            movimento_atual_processo = 3;
            fim_movimentacao_pecas = false;
            break;
          case 3:
            //Tipo_Movimento = 'r';
            processoTipoMovimento(true);
            break;
          case 4:
            movimento_atual_processo = 1;
            fim_movimentacao_pecas = false;
            break;

            Serial.println("");

            Serial.print("Movimento atual do processo: ");
            Serial.println(movimento_atual_processo);

            Serial.print("Ciclo Atual: ");
            Serial.println(ciclo_atual);

            Serial.println("");
        }
      }
    }
  }
}

//
void processoTipoMovimento(bool mudar_tipo_movimento) {
  //Muda o tipo de movimento (colocar ou remover peças) se o processo estiver em ciclos
  if (mudar_tipo_movimento && libera_mudanca_movimento) {
    libera_mudanca_movimento = false;

    if (Tipo_Movimento == 'c') {
      Tipo_Movimento = 'r';
    } else {
      Tipo_Movimento = 'c';
    }
  }

  if (Tipo_Sequencia == 's') {
    processoMagazineSequencia(Tipo_Movimento);
  }

  else if (Tipo_Sequencia == 'a') {
    processoMagazineAlternado(Tipo_Movimento);
  }
}

//Encher e Esvazair Magazine com as peças na sequência
void processoMagazineSequencia(char tipo_movimento) {
  if (fim_movimentacao_pecas == false) {

    if (Hab_V && ciclo_pecas == 0) {
      if (tipo_movimento == 'c') {
        magazineSequenciaColocar('V');
      } else if (tipo_movimento == 'r') {
        magazineSequenciaRemover('V');
      }
    } else if (!Hab_V && ciclo_pecas == 0) {
      ciclo_pecas = 1;
    }

    if (Hab_P && ciclo_pecas == 1) {
      if (tipo_movimento == 'c') {
        magazineSequenciaColocar('P');
      } else if (tipo_movimento == 'r') {
        magazineSequenciaRemover('P');
      }
    } else if (!Hab_P && ciclo_pecas == 1) {
      ciclo_pecas = 2;
    }

    if (Hab_M && ciclo_pecas == 2) {
      if (tipo_movimento == 'c') {
        magazineSequenciaColocar('M');
      } else if (tipo_movimento == 'r') {
        magazineSequenciaRemover('M');
      }
    } else if (!Hab_M && ciclo_pecas == 2) {
      ciclo_pecas = 3;
    }

    if (ciclo_pecas == 3) {
      ciclo_pecas = 0;
      fim_movimentacao_pecas = true;
      Serial.println("Magazine no fim da Sequência: ");
      for (int i = 0; i < 3; i++) {
        Serial.println(magazine[i]);
      }
      movimento_atual_processo++;
      libera_mudanca_movimento = true;
    }
  }
}

void magazineSequenciaColocar(char tipo_peca) {
  if (inicio_peca) {
    pos = 1;
    setorizacao = 0;
    inicio_peca = false;
  }

  if (pos <= Qtde) {
    switch (tipo_peca) {
      case 'V':
        prateleira_atual = Prat_V;
        calha_atual = 2;
        break;
      case 'P':
        prateleira_atual = Prat_P;
        calha_atual = 1;
        break;
      case 'M':
        prateleira_atual = Prat_M;
        calha_atual = 0;
        break;
    }

    if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
      // Se a peça não estiver na calha da Esteira e já estiver na magazine,
      // então não é necessário realizar esse movimento; pula para a próxima peça
      if ((calhasEsteira[calha_atual][pos - 1] == false) && (magazine[prateleira_atual][pos - 1] == tipo_peca)) {
        setorizacao = 4;
      }

      // //Se a peça não estiver na calha da Esteira e na magazine tem peça diferente, ou não tem peça, impede o processo com uma mensagem de erro
      // else if ((calhasEsteira[calha_atual][pos - 1] == false) && (magazine[prateleira_atual][pos - 1] != tipo_peca)) {
      //   tratamentoErro('M');
      // }

      else {
        if (verificaMovimentoCalhas(calha_atual, pos)) {
          Retira_Peca_Esteira(calha_atual);
          calhasEsteira[calha_atual][pos - 1] = false;
          novosDadosEsteira = true;

          Serial.print("Calha ");
          Serial.print(calha_atual);
          Serial.print(pos);
          Serial.print(" atualizada: ");
          Serial.println(calhasEsteira[calha_atual][pos - 1]);
          Serial.println("");

          Serial.print("Calha: ");
          Serial.print(tipo_peca);
          Serial.print(", ");
          Serial.println(pos);

          setorizacao = 1;
          tempoAtivacaoMovimentoAtual = millis();
        }
      }
    }

    if (setorizacao == 2 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
      if (verificaMovimentoMagazine(tipo_peca, pos, true)) {

        Coloca_Peca_Magazine(prateleira_atual, pos);
        magazine[prateleira_atual][pos - 1] = tipo_peca;
        novosDadosMagazine = true;

        setorizacao = 3;
        tempoAtivacaoMovimentoAtual = millis();
      } else {
        Serial.println("Erro Mag Colocar");
      }
    }

    if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {
      habilitaMovimento();
      tempoLeituraBitsAtual = millis();
      tempoEnvioDadosAtual = millis();

      if (setorizacao == 1) {
        Serial.print("Realizando movimento: Calha da Esteira - Tipo da Peça e Posição: ");
      } else if (setorizacao == 3) {
        Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
        //enviarDadosMagazine(prateleira_atual, (pos - 1));
      }
      Serial.print(tipo_peca);
      Serial.println(pos);

      setorizacao++;
    }

    if (robo_em_movimento) {
      Serial.println("Aguardando resposta o robo............");
    }

    if (novosDadosMagazine == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
      novosDadosMagazine = false;
      enviarDadosMagazine(prateleira_atual, (pos - 1));
    }

    if (novosDadosEsteira == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
      novosDadosEsteira = false;
      enviarDadosEsteira(calha_atual, (pos - 1));
    }

    if (setorizacao == 4 && robo_em_movimento == false) {
      pos++;
      setorizacao = 0;
    }

    //Espera a resposta do Robô, que encia o sinal quando termina o movimento
    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      observadorRespostaRobo();
    }

    //testar tempo com robo
    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      digitalWrite(pino_hab_movimento, DESATIVAR);
    }
  } else {
    inicio_peca = true;
    ciclo_pecas++;
  }
}

void magazineSequenciaRemover(char tipo_peca) {
  if (inicio_peca) {
    pos = Qtde;
    setorizacao = 0;
    inicio_peca = false;
  }

  if (pos > 0) {

    switch (tipo_peca) {
      case 'V':
        prateleira_atual = Prat_V;
        calha_atual = 2;
        break;
      case 'P':
        prateleira_atual = Prat_P;
        calha_atual = 1;
        break;
      case 'M':
        prateleira_atual = Prat_M;
        calha_atual = 0;
        break;
    }

    if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
      // Se a peça não estiver na magazine e já estiver na calha,
      // então não é necessário realizar esse movimento; pula para a próxima peça
      if ((calhasEsteira[calha_atual][pos - 1] == true) && (magazine[prateleira_atual][pos - 1] == '-')) {
        setorizacao = 4;
      } else {
        if (verificaMovimentoMagazine(tipo_peca, pos, false)) {

          Retira_Peca_Magazine(prateleira_atual, pos);
          magazine[prateleira_atual][pos - 1] = '-';
          novosDadosMagazine = true;

          setorizacao = 1;
          tempoAtivacaoMovimentoAtual = millis();
        }
      }
    }

    if (setorizacao == 2 && robo_em_movimento == false && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
      Libera_Coloca_Peca_Esteira();
      robo_em_movimento = true;
      calhasEsteira[calha_atual][pos - 1] = true;
      novosDadosEsteira = true;

      Serial.print("Calha ");
      Serial.print(calha_atual);
      Serial.print(pos);
      Serial.print(" atualizada: ");
      Serial.println(calhasEsteira[calha_atual][pos - 1]);
      Serial.println("");

      Serial.print("Calha: ");
      Serial.print(tipo_peca);
      Serial.print(", ");
      Serial.println(pos);

      setorizacao = 3;
      tempoAtivacaoMovimentoAtual = millis();
    }

    if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {

      if (setorizacao == 1) {
        habilitaMovimento();
        tempoLeituraBitsAtual = millis();
        tempoEnvioDadosAtual = millis();

        Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
      } else if (setorizacao == 3) {
        Serial.print("Realizando movimento: Calha da Esteira - Tipo da Peça e Posição: ");
      }
      Serial.print(tipo_peca);
      Serial.println(pos);

      setorizacao++;
    }

    if (robo_em_movimento) {
      Serial.println("Aguardando resposta o robo............");
    }

    if (novosDadosMagazine == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
      novosDadosMagazine = false;
      enviarDadosMagazine(prateleira_atual, (pos - 1));
    }

    if (novosDadosEsteira == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
      novosDadosEsteira = false;
      enviarDadosEsteira(calha_atual, (pos - 1));
    }

    if (setorizacao == 4 && robo_em_movimento == false) {
      pos--;
      setorizacao = 0;
    }

    //Espera a resposta do Robô, que envia o sinal quando termina o movimento
    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      observadorRespostaRobo();
    }

    //testar tempo com robo
    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      digitalWrite(pino_hab_movimento, DESATIVAR);
    }
  } else {
    inicio_peca = true;
    ciclo_pecas++;
  }
}

//Encher e Esvazair Magazine com as peças alternando as colunas e posições
void processoMagazineAlternado(char tipo_movimento) {
  if (ciclo_pecas == 0 && fim_movimentacao_pecas == false) {
    if (tipo_movimento == 'c') pos = 1;
    else if (tipo_movimento == 'r') pos = 4;
    ciclo_pecas = 1;
  }

  if (Hab_V && ciclo_pecas == 1) {
    magazineAlternado('V', tipo_movimento);
  } else if (!Hab_V && ciclo_pecas == 1) {
    ciclo_pecas = 2;
  }

  if (Hab_P && ciclo_pecas == 2) {
    magazineAlternado('P', tipo_movimento);
  } else if (!Hab_P && ciclo_pecas == 2) {
    ciclo_pecas = 3;
  }

  if (Hab_M && ciclo_pecas == 3) {
    magazineAlternado('M', tipo_movimento);
  } else if (!Hab_M && ciclo_pecas == 3) {
    ciclo_pecas = 4;
  }

  if (ciclo_pecas == 4) {
    if (tipo_movimento == 'c') pos++;
    else if (tipo_movimento == 'r') pos--;
    ciclo_pecas = 1;

    if (pos < 1 || pos > 4) {
      ciclo_pecas = 0;  //9
      fim_movimentacao_pecas = true;
      movimento_atual_processo++;
      libera_mudanca_movimento = true;

      Serial.print("ciclo_pecas ... ");
      Serial.println(ciclo_pecas);
      Serial.print("movimento_atual_processo ... ");
      Serial.println(movimento_atual_processo);
    }
  }
}

void magazineAlternado(char tipo_peca, char tipo_movimento) {
  if (inicio_peca) {
    setorizacao = 0;
    inicio_peca = false;
  }

  static byte pos_aleatoria;

  switch (tipo_peca) {
    case 'V':
      prateleira_atual = Prat_V;
      calha_atual = 2;
      break;
    case 'P':
      prateleira_atual = Prat_P;
      calha_atual = 1;
      break;
    case 'M':
      prateleira_atual = Prat_M;
      calha_atual = 0;
      break;
  }

  if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
    if (tipo_movimento == 'c') {

      if (verificaMovimentoCalhas(calha_atual, pos)) {
        Retira_Peca_Esteira(calha_atual);
        calhasEsteira[calha_atual][pos - 1] = false;
        novosDadosEsteira = true;

        Serial.print("Calha ");
        Serial.print(calha_atual);
        Serial.print(pos);
        Serial.print(" atualizada: ");
        Serial.println(calhasEsteira[calha_atual][pos - 1]);
        Serial.println("");

        Serial.print("Calha: ");
        Serial.print(tipo_peca);
        Serial.print(", ");
        Serial.println(pos);
      }

    } else if (tipo_movimento == 'r') {
      pos_aleatoria = aleatorio(prateleira_atual, false);

      if (verificaMovimentoMagazine(tipo_peca, pos_aleatoria, false)) {
        Retira_Peca_Magazine(prateleira_atual, pos_aleatoria);
        magazine[prateleira_atual][pos_aleatoria - 1] = '-';
        novosDadosMagazine = true;
      }

      Serial.print("posicao mag: ");
      Serial.print(tipo_peca);
      Serial.print(", ");
      Serial.println(pos_aleatoria);

      Serial.println("pos_aleatoria ................................");
      Serial.println(pos_aleatoria);
    }

    setorizacao = 1;
    tempoAtivacaoMovimentoAtual = millis();
  }

  if (setorizacao == 2 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
    if (tipo_movimento == 'c') {
      pos_aleatoria = aleatorio(prateleira_atual, true);

      if (verificaMovimentoMagazine(tipo_peca, pos_aleatoria, true)) {
        Coloca_Peca_Magazine(prateleira_atual, pos_aleatoria);
        magazine[prateleira_atual][pos_aleatoria - 1] = tipo_peca;
        novosDadosMagazine = true;
      }

      Serial.print("posicao mag: ");
      Serial.print(tipo_peca);
      Serial.print(", ");
      Serial.println(pos_aleatoria);

      setorizacao = 3;
      tempoAtivacaoMovimentoAtual = millis();

    } else if (tipo_movimento == 'r' && robo_em_movimento == false) {
      Libera_Coloca_Peca_Esteira();
      calhasEsteira[calha_atual][pos - 1] = true;
      novosDadosEsteira = true;

      Serial.print("Calha ");
      Serial.print(calha_atual);
      Serial.print(pos);
      Serial.print(" atualizada: ");
      Serial.println(calhasEsteira[calha_atual][pos - 1]);
      Serial.println("");

      Serial.print("Calha: ");
      Serial.print(tipo_peca);
      Serial.print(", ");
      Serial.println(pos);

      setorizacao = 3;
      tempoAtivacaoMovimentoAtual = millis();
    }
  }

  if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {
    habilitaMovimento();
    tempoLeituraBitsAtual = millis();
    tempoEnvioDadosAtual = millis();

    if (setorizacao == 1) {
      Serial.print("Realizando movimento: Calha da Esteira - Tipo da Peça e Posição: ");
    } else if (setorizacao == 3) {
      Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
      //enviarDadosMagazine(prateleira_atual, (pos_aleatoria - 1));
    }
    Serial.print(tipo_peca);
    Serial.println(pos);

    setorizacao++;
  }

  if (robo_em_movimento) {
    Serial.println("Aguardando resposta o robo............");
  }

  if (novosDadosMagazine == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
    novosDadosMagazine = false;
    enviarDadosMagazine(prateleira_atual, (pos - 1));
  }

  if (novosDadosEsteira == true && (millis() >= (tempoEsperaEnviaDados + tempoEnvioDadosAtual))) {
    novosDadosEsteira = false;
    enviarDadosEsteira(calha_atual, (pos - 1));
  }

  if (setorizacao == 4 && robo_em_movimento == false) {
    ciclo_pecas++;
    setorizacao = 0;
    inicio_peca = true;  //fim do loop e inicio de outra coluna de peças
  }

  //Espera a resposta do Robô, que encia o sinal quando termina o movimento
  if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
    observadorRespostaRobo();
  }

  //testar tempo com robo
  if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
    digitalWrite(pino_hab_movimento, DESATIVAR);
    Serial.println("Desativa Hab_movimento - Fim Leitura Bits");
  }
}

//
void Coloca_Peca_Magazine(byte prat, byte pos) {
  switch (prat) {
    case 0:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;

    case 1:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }

      break;

    case 2:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;
  }
}

//
void Retira_Peca_Magazine(byte prat, byte pos) {
  switch (prat) {
    case 0:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;

    case 1:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }

      break;

    case 2:
      switch (pos) {
        case 1:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:

          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;
  }
}

//
void Retira_Peca_Esteira(byte calha) {
  switch (calha) {
    case 1:
      digitalWrite(pino_bit_5, ATIVAR);
      digitalWrite(pino_bit_4, ATIVAR);
      digitalWrite(pino_bit_3, DESATIVAR);
      digitalWrite(pino_bit_2, DESATIVAR);
      digitalWrite(pino_bit_1, DESATIVAR);
      break;
    case 2:
      digitalWrite(pino_bit_5, ATIVAR);
      digitalWrite(pino_bit_4, ATIVAR);
      digitalWrite(pino_bit_3, DESATIVAR);
      digitalWrite(pino_bit_2, DESATIVAR);
      digitalWrite(pino_bit_1, ATIVAR);
      break;
    case 3:
      digitalWrite(pino_bit_5, ATIVAR);
      digitalWrite(pino_bit_4, ATIVAR);
      digitalWrite(pino_bit_3, DESATIVAR);
      digitalWrite(pino_bit_2, ATIVAR);
      digitalWrite(pino_bit_1, DESATIVAR);
      break;
    case 4:
      digitalWrite(pino_bit_5, ATIVAR);
      digitalWrite(pino_bit_4, ATIVAR);
      digitalWrite(pino_bit_3, DESATIVAR);
      digitalWrite(pino_bit_2, ATIVAR);
      digitalWrite(pino_bit_1, ATIVAR);
      break;
  }
}

//
void Libera_Coloca_Peca_Esteira() {
  digitalWrite(pino_bit_5, DESATIVAR);
  digitalWrite(pino_bit_4, DESATIVAR);
  digitalWrite(pino_bit_3, DESATIVAR);
  digitalWrite(pino_bit_2, DESATIVAR);
  digitalWrite(pino_bit_1, DESATIVAR);
  digitalWrite(pino_sinal_esteira, ATIVAR);
}

//Garante que o robô não vai colocar peças onde a posição já está ocupada, ou pegar uma peça em uma posição vazia, na magazine
bool verificaMovimentoMagazine(char tipoPeca, byte pos, bool colocarPeca) {
  byte pratel = 0;

  switch (tipoPeca) {
    case 'V':
      pratel = Prat_V;
      break;
    case 'P':
      pratel = Prat_P;
      break;
    case 'M':
      pratel = Prat_M;
      break;
  }

  Serial.println("");

  Serial.print("Prateleira: ");
  Serial.println(pratel);

  Serial.print("Posição: ");
  Serial.println(pos);

  Serial.println("Verificação da Magazine");
  for (int i = 0; i < 3; i++) {
    Serial.println(magazine[i]);
  }

  Serial.println("");

  bool verif = ((magazine[pratel][pos - 1] == '-') == colocarPeca);

  Serial.print("Posição selecionada: ");
  Serial.println(magazine[pratel][pos - 1]);

  Serial.print("Resultado da Verificação: ");
  Serial.println(verif);

  //delay(1000);

  if ((magazine[pratel][pos - 1] == '-') == colocarPeca) {
    return true;
  } else {
    tratamentoErro('M');
    return false;
  }
}

//Garante que o robô não vai pegar uma peça em uma posição vazia, nas calhas da Esteira
bool verificaMovimentoCalhas(byte calha, byte pos) {
  Serial.println("");

  Serial.print("Calha: ");
  Serial.println(calha);

  Serial.print("Posição ");
  Serial.println(pos);

  Serial.println("Verificação das Calhas da Esteira");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      Serial.print(calhasEsteira[i][j]);
    }
    Serial.println("");
  }

  Serial.println("");

  if (calhasEsteira[calha][pos - 1] == true) {
    return true;
  } else {
    tratamentoErro('M');
    return false;
  }
}

//Função que mostra possíveis erros no processo
void tratamentoErro(char erro) {
  switch (erro) {
    case 'M':
      Serial.println("ERRO - MOVIMENTO ERRADO");
      processo_em_andamento = false;
      ciclo_atual = 120;
      movimento_atual_processo = 1;

      Serial.println("----------------------------------------------------");

      Serial.println("Verificação da Magazine");
      for (int i = 0; i < 3; i++) {
        Serial.println(magazine[i]);
      }

      Serial.println("");

      // Serial.println("Verificação da Base do Robô");
      // for (int i = 0; i < 3; i++) {
      //   for (int j = 0; j < 4; j++) {
      //     Serial.print(baseRobo[i][j]);
      //   }
      //   Serial.println("");
      // }

      Serial.println("Verificação da Calha da Esteira");
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
          Serial.print(calhasEsteira[i][j]);
        }
        Serial.println("");
      }

      Serial.println("----------------------------------------------------");

      Serial.println("CONFIGURACOES");
      Serial.print("Hab_V: ");
      Serial.println(Hab_V);
      Serial.print("Hab_P: ");
      Serial.println(Hab_P);
      Serial.print("Hab_M: ");
      Serial.println(Hab_M);

      Serial.print("Prat_V: ");
      Serial.println(Prat_V);
      Serial.print("Prat_P: ");
      Serial.println(Prat_P);
      Serial.print("Prat_M: ");
      Serial.println(Prat_M);

      Serial.print("Tipo_Movimento: ");
      Serial.println(Tipo_Movimento);
      Serial.print("Tipo_Sequencia: ");
      Serial.println(Tipo_Sequencia);
      Serial.print("n_ciclos: ");
      Serial.println(n_ciclos);

      Serial.println("----------------------------------------------------");

      Serial.println("VARIAVEIS DE CONTROLE");
      Serial.print("Tipo_Movimento: ");
      Serial.println(Tipo_Movimento);

      Serial.println("");
      Serial.print("Posição Inicial pos: ");
      Serial.println(pos);

      Serial.println("");
      Serial.println("Setorizações");
      Serial.print("setorizacao: ");
      Serial.println(setorizacao);
      Serial.print("ciclo_pecas: ");
      Serial.println(ciclo_pecas);

      Serial.println("");

      Serial.print("fim_movimentacao_pecas: ");
      Serial.println(fim_movimentacao_pecas);
      Serial.print("inicio_peca: ");
      Serial.println(inicio_peca);

      Serial.print("movimento_atual_processo: ");
      Serial.println(movimento_atual_processo);

      Serial.print("libera_mudanca_movimento: ");
      Serial.println(libera_mudanca_movimento);
      Serial.print("interrompeCiclos: ");
      Serial.println(interrompeCiclos);
      Serial.println("----------------------------------------------------");

      break;
    default:
      Serial.println("ERRO NÃO IDENTIFICADO");
      break;
  }
}

//Permite que o Robô leia as suas entradas e realize o movimento escolhido
void habilitaMovimento() {
  robo_em_movimento = true;
  digitalWrite(pino_hab_movimento, ATIVAR);

  Serial.println("Habilita Movimento");
}

//Verifica se o Robô finalizou o movimento por enviar o sinal de resposta
void observadorRespostaRobo() {
  if (digitalRead(pino_resposta) == LOW) {
    robo_em_movimento = false;
  }
}

//
byte aleatorio(byte prateleira, bool colocar) {
  bool verificacao = false;
  byte minimo = 1;
  byte maximo = 4;
  byte numero = random(minimo, (maximo + 1));

  Serial.println("aleatorio");


  if (((magazine[prateleira][0] == '-') == colocar) || ((magazine[prateleira][1] == '-') == colocar) || ((magazine[prateleira][2] == '-') == colocar) || ((magazine[prateleira][3] == '-') == colocar)) {

    Serial.println("dentro if");

    while (!verificacao) {
      if ((magazine[prateleira][(numero - 1)] == '-') == colocar) {
        verificacao = true;
      } else {
        numero = random(minimo, (maximo + 1));
      }

      bool vereif = (magazine[prateleira][(numero - 1)] == '-') == colocar;

      Serial.println("continua loop");
      Serial.println("numero");
      Serial.println(numero);
      Serial.println("vereif");
      Serial.println(vereif);
    }

    //posicoes[prateleira][(numero - 1)] = colocar;
    Serial.println("saiu do loop");

    return numero;
  } else {
    return false;
  }
}