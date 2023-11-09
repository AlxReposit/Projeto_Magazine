// - - - - - - - - - - - - - - - - - - -
//Mapeamento de Hardware
// - - - - - - - - - - - - - - - - - - -
#define pino_resposta 19
#define pino_hab_movimento 12
#define pino_bit_6 11
#define pino_bit_5 10
#define pino_bit_4 9
#define pino_bit_3 8
#define pino_bit_2 7
#define pino_bit_1 6
#define pino_start 18

// - - - - - - - - - - - - - - - - - - -
//Funções
// - - - - - - - - - - - - - - - - - - -
void movimentos_em_ciclo();
void processoMagazineRobo(bool);
void processoMagazineSequencia(char);
void magazineSequenciaColocar(char);
void magazineSequenciaRemover(char);
void processoMagazineAlternado(char);
void magazineAlternado(char, char);
void Pega_Peca_Base(char, byte);
void Leva_Peca_Magazine(byte, byte);
void Pega_Peca_Magazine(byte, byte);
void Leva_Peca_Base(char, byte);
void habilitaMovimento();
void observadorRespostaRobo();
byte aleatorio(byte, bool);
bool verificaMovimentoMagazine(char, byte, bool);
bool verificaMovimentoBase(char, byte, bool);
void tratamentoErro(char);
void enviarDadosMagazine(byte, byte);

// - - - - - - - - - - - - - - - - - - -
//Variáveis do Processo
// - - - - - - - - - - - - - - - - - - -

//Tipo de Peça
bool Hab_V = 1;
bool Hab_P = 1;
bool Hab_M = 1;

//Tipo de Peça em cada Prateleira
byte Prat_V = 0;
byte Prat_P = 1;
byte Prat_M = 2;

//Quantidade de Peças a serem Separadas
const byte Qtde = 4;

//Operação
//'c' = Encher Magazine, Colocar Peças
//'r' = Esvaziar Magazine, Retirar Peças
char Operacao = 'c';

//Ordem de Retidada e Posicionamento de Peças
//'s' = Sequência
//'a' = Alternado
char Ordem = 's';

//Número de Ciclos de movimentos que o robô vai realizar
//entre 1 e 100 => ciclos definidos pelo número
//150  => sem ciclos (apenas o movimento de colocar ou de remover peças)
//200 => ciclos infinitos
//Máximo de ciclos: 100
byte n_ciclos = 0;

//guarda quantos ciclos faltam até acabar o processo
//101 => não inicializado (guarda o valor de n_ciclos no início do processo)
//0 => fim dos ciclos
byte ciclo_atual = 0;

// - - - - - - - - - - - - - - - - - - -
//Variáveis de Controle do Processo
// - - - - - - - - - - - - - - - - - - -

//Estado da Magazine
char magazine[3][5] = {
  "----",
  "----",
  "----"
};

//Estado da Base de peças do Robô
bool baseRobo[3][4] = {
  { true, true, true, true },
  { true, true, true, true },
  { true, true, true, true }
};

/*//Estado da Magazine
char magazine[3][5] = {
  "VVVV",
  "PPPP",
  "MMMM"
};

//Estado da Base de peças do Robô
bool baseRobo[3][4] = {
  { false, false, false, false },
  { false, false, false, false },
  { false, false, false, false }
};*/

//Controle de posições aleatórias
bool posicoes[3][4] = {
  { false, false, false, false },
  { false, false, false, false },
  { false, false, false, false }
};

bool ATIVAR = HIGH;    // Relês ligam em LOW
bool DESATIVAR = LOW;  // e desligam em HIGH

//Informa se o processo de movimentação de peças está em execução
bool processo_em_andamento = false;

//Robô em movimento
// 'true' se uma função de ativar um movimento no robô foi acionada
// 'false' quando o robô enviar um sinal dizendo que terminou o movimento, e então libera o próximo
bool robo_em_movimento = false;

// Espera um tempo antes de liberar a próxima ação para dar tempo de desligar a resposta do robô
// Grava o momento em que o movimento iniciou e espera o tempo definido em tempoEsperaProxMovimento (em milisegundos)
unsigned int tempoEsperaProxMovimento = 200;
unsigned long tempoAtivacaoMovimentoAtual = 0;

// Espera um tempo antes de bloquear a leitura para preparar os bits para o próximo movimento do robô
// Grava o momento em que o movimento iniciou e espera o tempo definido em tempoEsperaLeituraBits (em milisegundos)
unsigned int tempoEsperaLeituraBits = 200;
unsigned long tempoLeituraBitsAtual = 0;

byte pos = 1;  // Posição Inicial

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

// - - - - - - - - - - - - - - - - - - -
// Configurações Iniciais
// - - - - - - - - - - - - - - - - - - -
void setup() {
  pinMode(pino_resposta, INPUT_PULLUP);
  pinMode(pino_hab_movimento, OUTPUT);
  pinMode(pino_bit_6, OUTPUT);
  pinMode(pino_bit_5, OUTPUT);
  pinMode(pino_bit_4, OUTPUT);
  pinMode(pino_bit_3, OUTPUT);
  pinMode(pino_bit_2, OUTPUT);
  pinMode(pino_bit_1, OUTPUT);
  pinMode(pino_start, INPUT_PULLUP);

  digitalWrite(pino_hab_movimento, DESATIVAR);
  digitalWrite(pino_bit_6, DESATIVAR);
  digitalWrite(pino_bit_5, DESATIVAR);
  digitalWrite(pino_bit_4, DESATIVAR);
  digitalWrite(pino_bit_3, DESATIVAR);
  digitalWrite(pino_bit_2, DESATIVAR);
  digitalWrite(pino_bit_1, DESATIVAR);

  randomSeed(analogRead(0));

  Serial.begin(9600);

  n_ciclos = 1;
  ciclo_atual = 101;
  movimento_atual_processo = 1;
}

// - - - - - - - - - - - - - - - - - - -
// Programa Principal
// - - - - - - - - - - - - - - - - - - -
void loop() {
  movimentos_em_ciclo();
}

//Controla os ciclos que o processo vai realizar com base no valor de n_ciclos
//pensar em outro nome
void movimentos_em_ciclo() {
  //Sem Ciclos
  if (n_ciclos == 150) {
    processoMagazineRobo(false);
  }

  //Ciclos definidos por n_ciclos
  else if (n_ciclos > 0 && n_ciclos <= 100) {

    if (ciclo_atual == 101) {
      ciclo_atual = n_ciclos;

      Serial.println("");

      Serial.print("Movimento atual do processo: ");
      Serial.println(movimento_atual_processo);

      Serial.print("Ciclo Atual: ");
      Serial.println(ciclo_atual);

      Serial.println("");

      delay(1000);
    }

    if (ciclo_atual > 0) {
      switch (movimento_atual_processo) {
        case 1:
          //Operacao = 'c';
          processoMagazineRobo(true);
          break;
        case 2:
          movimento_atual_processo = 3;
          fim_movimentacao_pecas = false;
          break;
        case 3:
          //Operacao = 'r';
          processoMagazineRobo(true);
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

          delay(1500);
          break;
      }
    } else {
      //fim dos ciclos
    }
  }

  //Ciclos infinitos
  else if (n_ciclos == 200) {

    if (ciclo_atual == 101) {
      ciclo_atual = n_ciclos;

      Serial.println("");

      Serial.print("Movimento atual do processo: ");
      Serial.println(movimento_atual_processo);

      Serial.print("Ciclo Atual: ");
      Serial.println(ciclo_atual);

      Serial.println("");

      delay(1500);
    }

    if (ciclo_atual == 200) {
      switch (movimento_atual_processo) {
        case 1:
          //Operacao = 'c';
          processoMagazineRobo(true);
          break;
        case 2:
          movimento_atual_processo = 3;
          fim_movimentacao_pecas = false;
          break;
        case 3:
          //Operacao = 'r';
          processoMagazineRobo(true);
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

          delay(1500);
      }
    }
  }
}

//pensar em outro nome
void processoMagazineRobo(bool mudar_tipo_movimento) {
  //Muda o tipo de movimento (colocar ou remover peças) se o processo estiver em ciclos
  if (mudar_tipo_movimento && libera_mudanca_movimento) {
    if (Operacao == 'c') {
      Operacao = 'r';
    } else {
      Operacao = 'c';
    }

    libera_mudanca_movimento = false;
  }

  if (Ordem == 's') {
    processoMagazineSequencia(Operacao);
  }

  else if (Ordem == 'a') {
    processoMagazineAlternado(Operacao);
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
      delay(1000);
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

    static byte prateleira_atual;

    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      observadorRespostaRobo();
    }

    if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {

      if (verificaMovimentoBase(tipo_peca, pos, false)) {
        Pega_Peca_Base(tipo_peca, pos);

        switch (tipo_peca) {
          case 'V':
            baseRobo[0][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[0][pos - 1]);
            Serial.println("");

            break;
          case 'P':
            baseRobo[1][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[1][pos - 1]);
            Serial.println("");

            break;
          case 'M':
            baseRobo[2][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[2][pos - 1]);
            Serial.println("");

            break;
        }

        setorizacao = 1;
        tempoAtivacaoMovimentoAtual = millis();
      } else {
        Serial.println("erro base colocar");
      }
    }

    if (setorizacao == 2 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {

      if (verificaMovimentoMagazine(tipo_peca, pos, true)) {
        switch (tipo_peca) {
          case 'V':
            Leva_Peca_Magazine(Prat_V, pos);
            magazine[Prat_V][pos - 1] = 'V';
            prateleira_atual = Prat_V;
            //Serial.println(magazine[Prat_V][pos - 1]);

            break;
          case 'P':
            Leva_Peca_Magazine(Prat_P, pos);
            magazine[Prat_P][pos - 1] = 'P';
            prateleira_atual = Prat_P;
            //Serial.println(magazine[Prat_P][pos - 1]);

            break;
          case 'M':
            Leva_Peca_Magazine(Prat_M, pos);
            magazine[Prat_M][pos - 1] = 'M';
            prateleira_atual = Prat_M;
            //Serial.println(magazine[Prat_M][pos - 1]);

            break;
        }
        setorizacao = 3;
        tempoAtivacaoMovimentoAtual = millis();
      } else {
        Serial.println("erro mag colocar");
      }
    }

    if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {
      habilitaMovimento();
      tempoLeituraBitsAtual = millis();

      if (setorizacao == 1) {
        Serial.print("Realizando movimento: Base do Robô - Tipo da Peça e Posição: ");
      } else if (setorizacao == 3) {
        Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
        enviarDadosMagazine(prateleira_atual, (pos - 1));
      }
      Serial.print(tipo_peca);
      Serial.println(pos);

      setorizacao++;
    }

    if (robo_em_movimento) {
      Serial.println("Aguardando resposta o robo............");
    }

    if (setorizacao == 4) {
      pos++;
      setorizacao = 0;
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

    static byte prateleira_atual;

    if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
      observadorRespostaRobo();
    }

    if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {

      if (verificaMovimentoMagazine(tipo_peca, pos, false)) {

        switch (tipo_peca) {
          case 'V':
            Pega_Peca_Magazine(Prat_V, pos);
            magazine[Prat_V][pos - 1] = '-';
            prateleira_atual = Prat_V;

            //Serial.println(magazine[Prat_V][pos - 1]);
            break;
          case 'P':
            Pega_Peca_Magazine(Prat_P, pos);
            magazine[Prat_P][pos - 1] = '-';
            prateleira_atual = Prat_P;

            //Serial.println(magazine[Prat_P][pos - 1]);
            break;
          case 'M':
            Pega_Peca_Magazine(Prat_M, pos);
            magazine[Prat_M][pos - 1] = '-';
            prateleira_atual = Prat_M;

            //Serial.println(magazine[Prat_M][pos - 1]);
            break;
        }
        setorizacao = 1;
        tempoAtivacaoMovimentoAtual = millis();
      }
    }

    if (setorizacao == 2 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {

      if (verificaMovimentoBase(tipo_peca, pos, true)) {
        Leva_Peca_Base(tipo_peca, pos);

        switch (tipo_peca) {
          case 'V':
            baseRobo[0][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[0][pos - 1]);
            Serial.println("");

            break;
          case 'P':
            baseRobo[1][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[1][pos - 1]);
            Serial.println("");

            break;
          case 'M':
            baseRobo[2][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[2][pos - 1]);
            Serial.println("");

            break;
        }
        setorizacao = 3;
        tempoAtivacaoMovimentoAtual = millis();
      } else {
        Serial.println("erro aqui..................................");
      }
    }

    if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {
      habilitaMovimento();
      tempoLeituraBitsAtual = millis();

      if (setorizacao == 1) {
        Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
      } else if (setorizacao == 3) {
        Serial.print("Realizando movimento: Base do Robô - Tipo da Peça e Posição: ");
        enviarDadosMagazine(prateleira_atual, (pos - 1));
      }
      Serial.print(tipo_peca);
      Serial.println(pos);

      setorizacao++;
    }

    if (robo_em_movimento) {
      Serial.println("Aguardando resposta o robo............");
    }

    if (setorizacao == 4) {
      pos--;
      setorizacao = 0;
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

  if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
    observadorRespostaRobo();
  }

  static byte prateleira_atual;

  if (setorizacao == 0 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
    if (tipo_movimento == 'c') {

      if (verificaMovimentoBase(tipo_peca, pos, false)) {
        Pega_Peca_Base(tipo_peca, pos);

        switch (tipo_peca) {  //COLOCAR EM FUNÇÃO <---------------------------------
          case 'V':
            baseRobo[0][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[0][pos - 1]);
            Serial.println("");

            prateleira_atual = Prat_V;

            break;
          case 'P':
            baseRobo[1][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[1][pos - 1]);
            Serial.println("");

            prateleira_atual = Prat_P;

            break;
          case 'M':
            baseRobo[2][pos - 1] = false;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[2][pos - 1]);
            Serial.println("");

            prateleira_atual = Prat_M;

            break;
        }

        Serial.print("posicao base: ");
        Serial.print(tipo_peca);
        Serial.print(", ");
        Serial.println(pos);
      }

    } else if (tipo_movimento == 'r') {

      if (verificaMovimentoMagazine(tipo_peca, pos, false)) {

        byte pos_aleatoria;

        switch (tipo_peca) {
          case 'V':
            pos_aleatoria = aleatorio(Prat_V, false);
            Pega_Peca_Magazine(Prat_V, pos_aleatoria);
            magazine[Prat_V][pos_aleatoria - 1] = '-';
            prateleira_atual = Prat_V;
            break;
          case 'P':
            pos_aleatoria = aleatorio(Prat_P, false);
            Pega_Peca_Magazine(Prat_P, pos_aleatoria);
            magazine[Prat_P][pos_aleatoria - 1] = '-';
            prateleira_atual = Prat_P;
            break;
          case 'M':
            pos_aleatoria = aleatorio(Prat_M, false);
            Pega_Peca_Magazine(Prat_M, pos_aleatoria);
            magazine[Prat_M][pos_aleatoria - 1] = '-';
            prateleira_atual = Prat_M;
            break;
        }

        Serial.print("posicao mag: ");
        Serial.print(tipo_peca);
        Serial.print(", ");
        Serial.println(pos_aleatoria);

        Serial.println("pos_aleatoria ................................");
        Serial.println(pos_aleatoria);
      }
    }

    setorizacao = 1;
    tempoAtivacaoMovimentoAtual = millis();
  }

  if (setorizacao == 2 && (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits))) {
    if (tipo_movimento == 'c') {

      if (verificaMovimentoMagazine(tipo_peca, pos, true)) {

        byte pos_aleatoria;

        switch (tipo_peca) {
          case 'V':
            pos_aleatoria = aleatorio(Prat_V, true);
            Leva_Peca_Magazine(Prat_V, pos_aleatoria);
            magazine[Prat_V][pos_aleatoria - 1] = 'V';
            break;
          case 'P':
            pos_aleatoria = aleatorio(Prat_P, true);
            Leva_Peca_Magazine(Prat_P, pos_aleatoria);
            magazine[Prat_P][pos_aleatoria - 1] = 'P';
            break;
          case 'M':
            pos_aleatoria = aleatorio(Prat_M, true);
            Leva_Peca_Magazine(Prat_M, pos_aleatoria);
            magazine[Prat_M][pos_aleatoria - 1] = 'M';
            break;
        }

        Serial.print("posicao mag: ");
        Serial.print(tipo_peca);
        Serial.print(", ");
        Serial.println(pos_aleatoria);
      }

    } else if (tipo_movimento == 'r') {

      if (verificaMovimentoBase(tipo_peca, pos, 1)) {

        Leva_Peca_Base(tipo_peca, pos);

        switch (tipo_peca) {  //COLOCAR EM FUNÇÃO <---------------------------------
          case 'V':
            baseRobo[0][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[0][pos - 1]);
            Serial.println("");

            break;
          case 'P':
            baseRobo[1][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[1][pos - 1]);
            Serial.println("");

            break;
          case 'M':
            baseRobo[2][pos - 1] = true;

            Serial.print("Base ");
            Serial.print(tipo_peca);
            Serial.print(pos);
            Serial.print(" atualizada: ");
            Serial.println(baseRobo[2][pos - 1]);
            Serial.println("");

            break;
        }

        Serial.print("posicao base: ");
        Serial.print(tipo_peca);
        Serial.print(", ");
        Serial.println(pos);
      }
    }

    setorizacao = 3;
    tempoAtivacaoMovimentoAtual = millis();
  }

  if (robo_em_movimento == false && (millis() >= (tempoEsperaProxMovimento + tempoAtivacaoMovimentoAtual)) && (setorizacao == 1 || setorizacao == 3)) {
    habilitaMovimento();
    tempoLeituraBitsAtual = millis();

    if (setorizacao == 1) {
      Serial.print("Realizando movimento: Base do Robô - Tipo da Peça e Posição: ");
    } else if (setorizacao == 3) {
      Serial.print("Realizando movimento: Magazine - Tipo da Peça e Posição: ");
      enviarDadosMagazine(prateleira_atual, (pos - 1));
    }
    Serial.print(tipo_peca);
    Serial.println(pos);

    setorizacao++;
  }

  if (robo_em_movimento) {
    Serial.println("Aguardando resposta o robo............");
  }

  if (setorizacao == 4) {
    ciclo_pecas++;
    setorizacao = 0;
    inicio_peca = true;  //fim do loop e inicio de outra coluna de peças

    /*Serial.print("ciclo_pecas ... ");
    Serial.println(ciclo_pecas);
    Serial.print("movimento_atual_processo ... ");
    Serial.println(movimento_atual_processo);
    */
  }

  //testar tempo com robo
  if (millis() >= (tempoLeituraBitsAtual + tempoEsperaLeituraBits)) {
    digitalWrite(pino_hab_movimento, DESATIVAR);

    Serial.println("Desativa Hab_movimento - Fim Leitura Bits");
  }
}

//
void Pega_Peca_Base(char tipo_peca, byte pos) {
  switch (tipo_peca) {
    case 'V':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;

    case 'P':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }

      break;

    case 'M':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
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
void Leva_Peca_Magazine(byte prat, byte pos) {
  switch (prat) {
    case 0:
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
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
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
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
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
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
void Pega_Peca_Magazine(byte prat, byte pos) {
  switch (prat) {
    case 0:
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;

    case 1:
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, DESATIVAR);
          digitalWrite(pino_bit_5, ATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }

      break;

    case 2:
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;
  }
}

//
void Leva_Peca_Base(char tipo_peca, byte pos) {
  switch (tipo_peca) {
    case 'V':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, DESATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;

    case 'P':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, DESATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }

      break;

    case 'M':
      switch (pos) {
        case 1:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 2:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, DESATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;

        case 3:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, DESATIVAR);
          break;

        case 4:
          digitalWrite(pino_bit_6, ATIVAR);
          digitalWrite(pino_bit_5, DESATIVAR);
          digitalWrite(pino_bit_4, ATIVAR);
          digitalWrite(pino_bit_3, ATIVAR);
          digitalWrite(pino_bit_2, ATIVAR);
          digitalWrite(pino_bit_1, ATIVAR);
          break;
      }
      break;
  }
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


  delay(1000);

  if ((magazine[pratel][pos - 1] == '-') == colocarPeca) {
    return true;
  } else {
    tratamentoErro('M');
    Serial.println("erro aqui ...............................................");
    return false;
  }
}

//Garante que o robô não vai colocar peças onde a posição já está ocupada, ou pegar uma peça em uma posição vazia, na base de peças
bool verificaMovimentoBase(char tipoPeca, byte pos, bool colocarPeca) {
  byte coluna = 0;

  switch (tipoPeca) {
    case 'V':
      coluna = 0;
      break;
    case 'P':
      coluna = 1;
      break;
    case 'M':
      coluna = 2;
      break;
  }

  //Verifica se tem peças nas posições abaixo da posição em que se quer guardar a peça
  if (colocarPeca) {
    bool verificacao = false;

    if (pos == 4) {
      verificacao = true;
    } else {
      for (int i = pos; i < 4; i++) {
        if (baseRobo[coluna][i] == true) {
          verificacao = true;
        } else {
          tratamentoErro('M');
          return false;
        }
      }
    }


    if ((baseRobo[coluna][pos - 1] == false) && verificacao == true) {
      return true;
    } else {
      tratamentoErro('M');
      return false;
    }

  } else {
    //Verifica se não tem peças nas posições anteriores à posição em que o robô vai buscar
    bool verificacao = false;

    Serial.println("");

    Serial.print("Coluna: ");
    Serial.println(coluna);

    Serial.print("Posição ");
    Serial.println(pos);

    Serial.println("Verificação da Base do Robô");
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 4; j++) {
        Serial.print(baseRobo[i][j]);
      }
      Serial.println("");
    }

    Serial.println("");

    delay(1000);

    if (pos > 1) {
      for (int i = (pos - 1); i >= 1; i--) {

        Serial.print("Base ");
        Serial.print(tipoPeca);
        Serial.print(i);
        Serial.print(": ");
        Serial.println(baseRobo[coluna][i - 1]);

        if (baseRobo[coluna][i - 1] == false) {
          verificacao = true;
        } else {
          tratamentoErro('M');
          verificacao = false;
          Serial.println("erroloop");
          return false;
        }
      }
    } else {
      verificacao = true;
    }

    if ((baseRobo[coluna][pos - 1] == true) && verificacao == true) {
      return true;
    } else {
      tratamentoErro('M');
      return false;
    }
  }
}

//Função que mostra possíveis erros no processo
void tratamentoErro(char erro) {
  switch (erro) {
    case 'M':
      Serial.println("ERRO - MOVIMENTO ERRADO");
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

//Retorna um número aleatório entre 1 e 4, excluindo as posições que já estão ocupadas
/*byte aleatorio(byte prateleira, bool encher) {
  bool verificacao = false;
  byte minimo = 1;
  byte maximo = 4;
  byte numero = random(minimo, (maximo + 1));

  if ((posicoes[prateleira][0] != encher) || (posicoes[prateleira][1] != encher) || (posicoes[prateleira][2] != encher) || (posicoes[prateleira][3] != encher)) {

    while (!verificacao) {
      if (posicoes[prateleira][(numero - 1)] == encher) {
        numero = random(minimo, (maximo + 1));
      } else {
        verificacao = true;
      }
      Serial.println("loop");
    }

    posicoes[prateleira][(numero - 1)] = encher;

    return numero;
  } else {
    return 0;
  }
}*/

byte aleatorio(byte prateleira, bool colocar) {
  bool verificacao = false;
  byte minimo = 1;
  byte maximo = 4;
  byte numero = random(minimo, (maximo + 1));

  if (((magazine[prateleira][0] == '-') == colocar)
    || (magazine[prateleira][1] == '-') == colocar)
    || (magazine[prateleira][2] == '-') == colocar) 
    || (magazine[prateleira][3] == '-') == colocar)) {

      while (!verificacao) {
        if (posicoes[prateleira][(numero - 1)] == colocar) {
          numero = random(minimo, (maximo + 1));
        } else {
          verificacao = true;
        }
        Serial.println("loop");
      }

      posicoes[prateleira][(numero - 1)] = colocar;

      return numero;
    }
  else {
    return 0;
  }
}

//Gera uma mensagem apenas as informações da magazine que mudaram e envia para o Esp32
//mudar para tab da Comunicação com Esp
void enviarDadosMagazine(byte prateleira, byte posicao) {
  //enviandoDadosMagazine = true;
  /*String msg = String();
    msg = "<";
    msg += prateleira;
    msg += posicao;
    msg += magazine[prateleira][posicao];
    msg += '>';
    Serial.println(msg);*/

  Serial.print('<');
  Serial.print('{');
  Serial.print(prateleira);
  Serial.print('[');
  Serial.print(posicao);
  Serial.print('(');
  Serial.print(magazine[prateleira][posicao]);
  Serial.println('>');

  //enviandoDadosMagazine = false;
}
