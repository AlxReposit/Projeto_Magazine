// - - - - - - - - - - - - - - - - - - -
// Inclusões e Definiçoes de informações
// para preparar a biblioteca Firebase_ESP_Client
// - - - - - - - - - - - - - - - - - - -
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define dados do Wifi */
#define WIFI_SSID "Galaxy a."
#define WIFI_PASSWORD "123123098"

//#define WIFI_SSID "Rede Longarez"
//#define WIFI_PASSWORD "LauraLePedroA"

/** 2. Define a API Key
 * Obtida com a criação de um projeto Firebasee e habilidando a autenticação.
 * Configurações do Projeto -> Geral -> Web API Key
 */
#define API_KEY "AIzaSyCS2IckjSds29v8UUomI1aYt5mPhFlWB9Y"

/* 3. Informações para Autenticação de Email e Senha do usuário do ESP32 no projeto */
#define AUTH_EMAIL "esp32@magazine.com"
#define AUTH_PASSWORD "TW9uaXRvck1hZ2F6aW5lRXNw"

/* 4. URL do RTDB */
#define DATABASE_URL "https://teste-app-monitor-default-rtdb.firebaseio.com"

#define Tx2 17  // Comunicação Serial com Arduino
#define Rx2 16  // nos pinos G16 e G17

/* 5. Objetos Firebase Data, para receber e enviar informações entre ESP32 e RTDB */
FirebaseData fbdo;

// Firebase Data object para a função Stream
FirebaseData stream_start;
FirebaseData stream_stop;
FirebaseData stream_configs;

/* 7. Objeto FirebaseAuth para autenticação de usuário */
FirebaseAuth auth;

/* 8. Objeto FirebaseConfig para definir as configurações da conexão */
FirebaseConfig config;

bool signupOK = false;  //Verifica se o login foi realizado

// - - - - - - - - - - - - - - - - - - -
//Variáveis do Processo
// - - - - - - - - - - - - - - - - - - -
//Monitora o estado da Magazine
char magazine[3][5] = {
  "----",
  "----",
  "----"
};

//Tipo de Peça habilitada para movimentação
bool Hab_V = 0;
bool Hab_P = 0;
bool Hab_M = 0;

//Tipo de Peça em cada Prateleira
byte Prat_V = 0;
byte Prat_P = 1;
byte Prat_M = 2;

char Prat_0[2] = "V";
char Prat_1[2] = "P";
char Prat_2[2] = "M";

//Operação
//'c' = Encher Magazine, Colocar Peças
//'r' = Esvaziar Magazine, Retirar Peças
char Tipo_Movimento[2] = "c";

//Ordem de Retidada e Posicionamento de Peças
//'s' = Sequência
//'a' = Alternado
char Tipo_Sequencia[2] = "s";

//Número de Ciclos de movimentos que o robô vai realizar
//entre 1 e 10 => ciclos definidos pelo número
//150  => sem ciclos (apenas o movimento de colocar ou de remover peças)
//200 => ciclos infinitos
//Máximo de ciclos: 100
byte n_ciclos = 1;

bool start = false;  // inicia processo
bool stop = false;   // zera ciclos

//Variáveis de monitoramento do processo:
bool processo_em_andamento;
bool robo_em_movimento;
byte ciclo_atual;

unsigned long tempoEnvioDadosStream = 0;  //Tempo de espera da leitura de Stream
unsigned long tempoEsperaStream = 5000;   //para não acabar com meus dados móveis

// Sempre que recebe novas configurações, fica true e
// espera o processo atual acabar para enviar configs para o Arduino
bool novosDadosConfigs = false;

bool reconectarFirebase = false;

// - - - - - - - - - - - - - - - - - - -
//Funções
// - - - - - - - - - - - - - - - - - - -
//Firebase
void WiFiConectadoRoteador(WiFiEvent_t, WiFiEventInfo_t);
void WiFiConectadoIP(WiFiEvent_t, WiFiEventInfo_t);
void WiFiDesconectado(WiFiEvent_t, WiFiEventInfo_t);
void conectarWifi();
void conectarFirebase();
void streamFirebase();
void receberDadosConfigsRTDB();
void enviarDadosMagazineRTDB(const byte, const byte, const char *);
void enviarDadosConfigsRTDB();
void enviarDadosProcessoRTDB();

//Comunicação com Arduino
void observadorComunicacaoArduino();
void receberDadosMagazine();
void receberDadosProcesso();
void conversaoPrateleiras();
void enviarDadosConfigsArduino();
void enviarStartStopArduino(bool);
void enviarStartArduino();

// - - - - - - - - - - - - - - - - - - -
// Configurações Iniciais
// - - - - - - - - - - - - - - - - - - -
void setup() {
  //Serial.begin(115200, SERIAL_8N1, Rx0, Tx0);
  Serial.begin(115200);                       // Serial PC
  Serial2.begin(9600, SERIAL_8N1, Rx2, Tx2);  // Serial Arduino

  conectarWifi();

  while (!reconectarFirebase) {
    Serial.print('.');
    delay(1000);
  }

  reconectarFirebase = false;

  conectarFirebase();

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  Serial.print("User UID: ");
  Serial.print(auth.token.uid.c_str());

  receberDadosConfigsRTDB();
}

// - - - - - - - - - - - - - - - - - - -
// Programa Principal
// - - - - - - - - - - - - - - - - - - -
void loop() {
  // if (reconectarFirebase) {
  //   reconectarFirebase = false;
  //   conectarFirebase();

  //   // Getting the user UID might take a few seconds
  //   Serial.println("Getting User UID");
  //   while ((auth.token.uid) == "") {
  //     Serial.print('.');
  //     delay(1000);
  //   }

  //   // Print user UID
  //   Serial.print("User UID: ");
  //   Serial.print(auth.token.uid.c_str());

  //   receberDadosConfigsRTDB();
  // }


  /*if (Firebase.failed()) {
    Serial.print("setting number failed:");
    Serial.println(Firebase.error());
    ESP.reset();
    return;
  }*/

  observadorComunicacaoArduino();

  if ((millis() - tempoEnvioDadosStream) > tempoEsperaStream) {
    tempoEnvioDadosStream = millis();
    streamFirebase();
  }
}

/*
Função para bloquear salvar as configurações caso a pessoa escolha colocar peças na magazine se ela estiver cheia ou
retirar peças se ela estiver vazia. Verificar essas condições para as peças habilitadas
*/