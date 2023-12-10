// - - - - - - - - - - - - - - - - - - -
//Mapeamento de Hardware
// - - - - - - - - - - - - - - - - - - -
#define pino_resposta 19
#define pino_hab_movimento 13
#define pino_bit_5 12
#define pino_bit_4 11
#define pino_bit_3 10
#define pino_bit_2 9
#define pino_bit_1 8
#define pino_sinal_esteira 7

// - - - - - - - - - - - - - - - - - - -
//Funções
// - - - - - - - - - - - - - - - - - - -
// Controle Robô
void setupControleRobo();
void processoControleRobo();
void processoTipoMovimento(bool);

void processoMagazineSequencia(char);
void magazineSequenciaColocar(char);
void magazineSequenciaRemover(char);

void processoMagazineAlternado(char);
void magazineAlternado(char, char);

void Coloca_Peca_Magazine(byte, byte);
void Retira_Peca_Magazine(byte, byte);
void Retira_Peca_Esteira(byte);
void Libera_Coloca_Peca_Esteira();

void habilitaMovimento();
void observadorRespostaRobo();
byte aleatorio(byte, bool);
bool verificaMovimentoMagazine(char, byte, bool);
bool verificaMovimentoBase(char, byte, bool);
void tratamentoErro(char);

// Comunicação com Esp32
void setupComunicacaoComEsp32();
void observadorComunicacaoEsp32();
void receberDadosConfigs();
void receberDadosMagazine();
void enviarDadosMagazine(byte, byte);
void enviarDadosProcesso();
void observadorMudancaProcesso();

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

//Operação / Tipo de Movimento a ser realizado
//'c' = Encher Magazine, Colocar Peças
//'r' = Esvaziar Magazine, Retirar Peças
char Tipo_Movimento = 'c';

//Tipo de sequência / Tipo_Sequencia de Retidada e Posicionamento de Peças
//'s' = Sequência
//'a' = Alternado
char Tipo_Sequencia = 's';

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
  "VVVV",
  "PPPP",
  "MMMM"
};

// //Estado das Calhas de peças da Esteira
// bool calhasEsteira[4][3] = {
//   { false, false, false }, // Calha de Peças Metálicas
//   { false, false, false }, // Calha de Peças Pretas 
//   { false, false, false }, // Calha de Peças Vermelhas
//   { false, false, false }  // Calha de Peças de Refugo
// };

//Estado das Calhas de peças da Esteira
bool calhasEsteira[3][4] = {
  { false, false, false, false }, // Calha de Peças Metálicas
  { false, false, false, false }, // Calha de Peças Pretas 
  { false, false, false, false }, // Calha de Peças Vermelhas
};

//Informa se o processo de movimentação de peças está em execução
bool processo_em_andamento = false;

//Robô em movimento
// 'true' se uma função de ativar um movimento no robô foi acionada
// 'false' quando o robô enviar um sinal dizendo que terminou o movimento, e então libera o próximo
bool robo_em_movimento = false;

// - - - - - - - - - - - - - - - - - - -
// Configurações Iniciais
// - - - - - - - - - - - - - - - - - - -
void setup() {
  setupComunicacaoComEsp32();
  setupControleRobo();
  Serial.println("inicio");
}

// - - - - - - - - - - - - - - - - - - -
// Programa Principal
// - - - - - - - - - - - - - - - - - - -
void loop() {
  observadorComunicacaoEsp32();
  processoControleRobo();
}