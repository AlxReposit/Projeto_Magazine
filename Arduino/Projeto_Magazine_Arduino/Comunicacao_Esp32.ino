char leituraSerial;
bool start;
bool stop;

// envia a mensagem como <{1[1(-> (uma posição de cada vez)
// < = início da mensagem
// { = índice da prateleira
// [ = índice da posição
// ( = tipo da peça:
//   M = metálica
//   P = preta
//   V = vermalha
//   - ou qualquer outro = vazio
// > = fim da mensagem

void setupComunicacaoComEsp32() {
  Serial.begin(9600);
  enviarDadosProcesso();
  //receberDadosMagazine();
}

void observadorComunicacaoEsp32() {
  if (Serial.available() > 0) {
    leituraSerial = Serial.read();
    receberDadosConfigsEsp32();
    receberDadosStartStop();
  }

  observadorMudancaProcesso();
}

//Gera uma mensagem apenas com as informações da magazine que mudaram e envia para o Esp32
void enviarDadosMagazine(byte prateleira, byte posicao) {
  Serial.print('<');
  Serial.print(prateleira);
  Serial.print(';');
  Serial.print(posicao);
  Serial.print(';');
  Serial.print(magazine[prateleira][posicao]);
  Serial.println('>');

  // Serial.print('<');
  // Serial.print('{');
  // Serial.print(prateleira);
  // Serial.print('[');
  // Serial.print(posicao);
  // Serial.print('(');
  // Serial.print(magazine[prateleira][posicao]);
  // Serial.println('>');
}

//Gera uma mensagem apenas com as informações das calhas da Esterira que mudaram e envia para o Esp32
void enviarDadosEsteira(byte calha, byte posicao) {
  Serial.print('{');
  Serial.print(calha);
  Serial.print(';');
  Serial.print(posicao);
  Serial.print(';');
  Serial.print(calhasEsteira[calha][posicao]);
  Serial.println('}');
}

//Envia uma mensagem com as informações do processo para o Esp32
void enviarDadosProcesso() {
  Serial.print('[');
  Serial.print(processo_em_andamento);
  Serial.print(',');
  Serial.print(robo_em_movimento);
  Serial.print(',');
  Serial.print(ciclo_atual);
  Serial.println(']');
}

void observadorMudancaProcesso(){
  static bool temp_processo = processo_em_andamento;
  static bool temp_robo = robo_em_movimento;
  static byte temp_ciclo = ciclo_atual;

  if(temp_processo != processo_em_andamento){
    enviarDadosProcesso();
    temp_processo = processo_em_andamento;
  }

  if(temp_robo != robo_em_movimento){
    enviarDadosProcesso();
    temp_robo = robo_em_movimento;
  }

  if(temp_ciclo != ciclo_atual){
    enviarDadosProcesso();
    temp_ciclo = ciclo_atual;
  }
}

//OK
void receberDadosConfigsEsp32() {
  static const char inicioLeitura = '[';
  static const char fimLeitura = ']';
  static const char separador = ',';
  static bool leituraConfigsEmProgresso = false;
  static byte setorizacao = 0;
  static char str_ciclos[4] = "---";
  static char *ponteiro_ciclos;

  //[1,1,1,0,1,2,c,s,5]
  //[0,0,0,2,1,0,r,a,5]

  if (leituraConfigsEmProgresso == true) {
    if (leituraSerial != fimLeitura) {

      if (leituraSerial == separador) {
        setorizacao++;
      } else {
        switch (setorizacao) {
          case 0:
            Hab_V = byte(leituraSerial - '0');
            break;
          case 1:
            Hab_P = byte(leituraSerial - '0');
            break;
          case 2:
            Hab_M = byte(leituraSerial - '0');
            break;
          case 3:
            Prat_V = byte(leituraSerial - '0');
            break;
          case 4:
            Prat_P = byte(leituraSerial - '0');
            break;
          case 5:
            Prat_M = byte(leituraSerial - '0');
            break;
          case 6:
            Tipo_Movimento = leituraSerial;
            break;
          case 7:
            Tipo_Sequencia = leituraSerial;
            break;
          case 8:
            if (ponteiro_ciclos) {
              *ponteiro_ciclos = leituraSerial;
              ponteiro_ciclos++;
            }
            break;
        }
      }
    } else if (leituraSerial == fimLeitura) {
      n_ciclos = atoi(str_ciclos);
      leituraConfigsEmProgresso = false;

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
    }
  } else if (leituraSerial == inicioLeitura) {
    leituraConfigsEmProgresso = true;
    setorizacao = 0;
    strcpy(str_ciclos, "   ");
    ponteiro_ciclos = str_ciclos;
  }
}

//
void receberDadosStartStop() {
  if (leituraSerial == '!') {
    start = true;
  } else if (leituraSerial == '?') {
    stop = true;
  }
}
