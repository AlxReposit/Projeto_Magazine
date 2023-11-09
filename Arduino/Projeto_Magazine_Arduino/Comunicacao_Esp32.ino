bool enviandoDadosMagazine = false;

void receberDadosConfigs();
void receberDadosMagazine();
void enviarDadosMagazine();
// envia a mensagem como <{----{----{----> (todos de uma vez)
// < = início da mensagem
// { = inicio da próxima prateleira
// - = tipo da peça:
//   M = metálica
//   P = preta
//   V = vermalha
//   - ou qualquer outro = vazio
// > = fim da mensagem

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

//char msg[] = "<{----{----{---->";

char magazine[3][5] = {
  "VVVV",
  "----",
  "MMMM"
};

double tempo_atual = -15000;

void setup() {
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {

  /*if(millis() > (tempo_atual + 15000)){
    enviarDadosMagazine(0,3);
    tempo_atual = millis();
    }*/

  receberDadosMagazine();

  /*magazine[0] = digitalRead(13);
    magazine[1] = digitalRead(12);
    magazine[2] = digitalRead(11);
    magazine[3] = digitalRead(10);
    magazine[4] = digitalRead(9);
    magazine[5] = digitalRead(8);
  */
}

void atualizaMagazine() {
}

void atualizaMagazine(int prat, int pos, char valor) {
  magazine[prat][pos] = valor;
}

//Gera uma mensagem com as informações da magazine toda e envia para o Esp32
void enviarDadosMagazine() {
  enviandoDadosMagazine = true;
  String msg = String();

  msg = "<";
  for (int prat = 0; prat < 3; prat++) {
    for (int pos = -1; pos < 4; pos++) {
      if (pos != -1) {
        msg += magazine[prat][pos];
      } else {
        msg += '{';
      }
    }
  }

  msg += '>';
  Serial.println(msg);
  enviandoDadosMagazine = false;
}

//Gera uma mensagem apenas as informações da magazine que mudaram e envia para o Esp32
void enviarDadosMagazine(byte prateleira, byte posicao) {
  enviandoDadosMagazine = true;
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

  enviandoDadosMagazine = false;
}

//salvar em outro lugar
void receberDadosMagazine() {
  static char inicioLeitura = '<';
  static char fimLeitura = '>';
  static char marcadorPrateleira = '{';
  static char marcadorPosicao = '[';
  static char marcadorPeca = '(';
  static byte setorizacao = 0;
  static bool leituraEmProgresso = false;
  static byte pos = 0;
  static byte prat = 0;
  static String pos_c = "";
  static String prat_c = "";
  static char peca = '-';
  char leituraSerialArduino;

  if (Serial.available() > 0) {
    leituraSerialArduino = Serial.read();

    if (leituraEmProgresso == true) {
      if (leituraSerialArduino != fimLeitura) {

        if (leituraSerialArduino == marcadorPrateleira) {
          setorizacao = 1;
        } else if (leituraSerialArduino == marcadorPosicao) {
          setorizacao = 2;
        } else if (leituraSerialArduino == marcadorPeca) {
          setorizacao = 3;
        } else if (setorizacao == 1) {
          prat_c += leituraSerialArduino;
        } else if (setorizacao == 2) {
          prat = prat_c.toInt();
          pos_c += leituraSerialArduino;
        } else if (setorizacao == 3) {
          pos = pos_c.toInt();
          magazine[prat][pos] = leituraSerialArduino;

          Serial.print("prateleira: ");
          Serial.println(prat);
          Serial.print("posição: ");
          Serial.println(pos);
          Serial.print("peça: ");
          Serial.println(magazine[prat][pos]);

          prat_c = "";
          pos_c = "";
        } /*
        {
          if (leituraSerialArduino == marcadorPrateleira) {
            setorizacao = 1;
          } else if (leituraSerialArduino == marcadorPosicao) {
            setorizacao = 2;
          } else if (leituraSerialArduino == marcadorPeca) {
            setorizacao = 3;
          } else if (setorizacao == 1) {
            prat = (byte)leituraSerialArduino - '0';
          } else if (setorizacao == 2) {
            pos = byte(leituraSerialArduino - '0');
          } else if (setorizacao == 3) {
            magazine[prat][pos] = leituraSerialArduino;
            Serial.print("prateleira: ");
            Serial.println(prat);
            Serial.print("posição: ");
            Serial.println(pos);
            Serial.print("peça: ");
            Serial.println(magazine[prat][pos]);
          }
        }<{23[34(M>*/

      } else if (leituraSerialArduino == fimLeitura) {
        //chegou no fim da leitura
        leituraEmProgresso = false;
        pos = 0;
      }
    } else if (leituraSerialArduino == inicioLeitura) {
      leituraEmProgresso = true;
    }
  } else {
  }
}

//testar
void receberDadosConfigs() {
  static char inicioLeitura = '[';
  static char fimLeitura = ']';
  static char separador = ',';
  static byte setorizacao = 0;
  static bool leituraEmProgresso = false;
  static String num_ciclos = "";
  char leituraSerialArduino;

  if (Serial.available() > 0) {
    leituraSerialArduino = Serial.read();

    if (leituraEmProgresso == true) {
      if (leituraSerialArduino != fimLeitura) {

        if (leituraSerialArduino == separador) {
          setorizacao++;
        }

        switch (setorizacao) {
          case 0:  // Habilita Peça Vermelha
            Hab_V = byte(leituraSerialArduino);
            break;
          case 1:  // Habilita Peça Preta
            Hab_P = byte(leituraSerialArduino);
            break;
          case 2:  // Habilita Peça Metálica
            Hab_M = byte(leituraSerialArduino);
            break;
          case 3:  // Prateleira para Peças Vermelhas
            Prat_V = byte(leituraSerialArduino);
            break;
          case 4:  // Prateleira para Peças Pretas
            Prat_P = byte(leituraSerialArduino);
            break;
          case 5:  // Prateleira para Peças Metálicas
            Prat_M = byte(leituraSerialArduino);
            break;
          case 6:  // Operação
            Operacao = leituraSerialArduino;
            break;
          case 7:  // Ordem
            Ordem = leituraSerialArduino;
            break;
          case 8:  // Ciclos
            num_ciclos += leituraSerialArduino;
            break;
        }

      } else {
        //chegou no fim da leitura
        n_ciclos = num_ciclos.toInt();
        leituraEmProgresso = false;
      }
    } else if (leituraSerialArduino == inicioLeitura) {
      leituraEmProgresso = true;
    }
  } else {
  }
}

//testar
void receberDadosStartStop() {
  static bool leituraEmProgresso = false;
  bool salvarStart = 0;
  bool salvarStop = 0;
  char leituraSerialArduino;

  if (Serial.available() > 0) {
    leituraSerialArduino = Serial.read();

    if (leituraEmProgresso == true) {
      if (salvarStart) {
        start = leituraSerialArduino;
      }
      if (salvarStop) {
        stop = leituraSerialArduino;
      }

      if (leituraSerialArduino == '!') {
        salvarStart = 1
      }
      if (leituraSerialArduino == '?') {
        salvarStop = 1;
      }
    } else {
      //chegou no fim da leitura
      leituraEmProgresso = false;
    }
  } else if (leituraSerialArduino == '!' || leituraSerialArduino == '?') {
    leituraEmProgresso = true;
  }
}






/*

Receber Config Esp

bool Hab_V = 1;
bool Hab_P = 1;
bool Hab_M = 1;

byte Prat_V = 1;
byte Prat_P = 2;
byte Prat_M = 3;

//Operação
//'c' = Encher Magazine, Colocar Peças
//'r' = Esvaziar Magazine, Retirar Peças
char Operacao = 'e';

//Ordem de Retidada e Posicionamento de Peças
//'s' = Sequência
//'a' = Alternado
char Ordem = 'a';

int n_ciclos = 0;

void receberDadosConfigsEsp32();

void setup() {
  Serial.begin(9600);
  Serial.println("inicio");
}

void loop() {
  receberDadosConfigsEsp32();
}


void receberDadosConfigsEsp32() {
  static const char inicioLeitura = '[';
  static const char fimLeitura = ']';
  static const char separador = ',';
  static bool leituraEmProgresso = false;
  static byte setorizacao = 0;
  static char str_ciclos[4] = "---";
  static char *ponteiro_ciclos;
  char leituraSerial;
  //[1,1,1,0,1,2,c,s,5]
  //[0,0,0,2,1,0,r,a,5]

  if (Serial.available() > 0) {
    leituraSerial = Serial.read();

    if (leituraEmProgresso == true) {
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
              Operacao = leituraSerial;
              break;
            case 7:
              Ordem = leituraSerial;
              break;
            case 8:
              char tempLeitura[2];
              //tempLeitura[0] = leituraSerial;
              //tempLeitura[0] = '\0';
              //strcat(str_ciclos, tempLeitura);
              if (ponteiro_ciclos) {
                *ponteiro_ciclos = leituraSerial;
                ponteiro_ciclos++;
              }
              break;
          }
        }
      } else if (leituraSerial == fimLeitura) {
        n_ciclos = atoi(str_ciclos);
        leituraEmProgresso = false;

        //char *concat = "Habilita V: ";
        // strcat(concat, Hab_V);

        Serial.println(" ");
        Serial.println(" ");
        Serial.print("Habilita V: ");
        Serial.println(Hab_V);
        Serial.print("Habilita P: ");
        Serial.println(Hab_P);
        Serial.print("Habilita M: ");
        Serial.println(Hab_M);

        Serial.println(" ");
        Serial.print("Prat V: ");
        Serial.println(Prat_V);
        Serial.print("Prat P: ");
        Serial.println(Prat_P);
        Serial.print("Prat M: ");
        Serial.println(Prat_M);

        Serial.println(" ");
        Serial.print("Operacao: ");
        Serial.println(Operacao);
        Serial.print("Ordem: ");
        Serial.println(Ordem);
        Serial.print("Ciclos: ");
        Serial.println(n_ciclos);
        Serial.print("strCiclos: ");
        Serial.println(str_ciclos);
      }
    } else if (leituraSerial == inicioLeitura) {
      leituraEmProgresso = true;
      setorizacao = 0;
      strcpy(str_ciclos, "   ");
      ponteiro_ciclos = str_ciclos;
    }
  }
}
