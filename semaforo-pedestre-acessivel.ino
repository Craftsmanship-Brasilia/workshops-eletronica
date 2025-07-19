// Pinos utilizados
const int BUZZER_PIN = 8; // Buzzer adicionado no pino 8
const int BUTTON_PIN = 2;
const int PED_GREEN_PIN = 3;
const int PED_RED_PIN = 4;
const int CAR_RED_PIN = 5;
const int CAR_YELLOW_PIN = 6;
const int CAR_GREEN_PIN = 7;

// Vetor para inicializar os pinos de LEDs
const int LED_PINS[] = {PED_GREEN_PIN, PED_RED_PIN, CAR_RED_PIN, CAR_YELLOW_PIN, CAR_GREEN_PIN};

// Estados do semáforo
const int STATE_CAR_GREEN = 0;
const int STATE_CAR_YELLOW = 1;
const int STATE_CAR_RED = 2;
const int STATE_PEDESTRIAN_GREEN = 3;
const int STATE_BLINK = 4;

// Tempos de transição entre os estados (em milissegundos)
const unsigned long TIME_CAR_GREEN = 45000; // 45 segundos para carros
const unsigned long TIME_CAR_YELLOW = 4000; // 4 segundos para amarelo
const unsigned long TIME_CAR_RED = 2000;   // 2 segundos para transição antes do pedestre
const unsigned long TIME_PEDESTRIAN_GREEN = 20000; // 20 segundos para pedestres
const unsigned long TIME_BLINK = 6000;     // 6 segundos para piscar

// Vetor para armazenar os tempos de cada estado
const unsigned long STATE_TIMES[] = {TIME_CAR_GREEN, TIME_CAR_YELLOW, TIME_CAR_RED, TIME_PEDESTRIAN_GREEN, TIME_BLINK};

// Variáveis globais
int currentState = STATE_CAR_GREEN; // Estado inicial
unsigned long stateStartTime = 0;   // Tempo de início do estado atual
bool pedestrianWaiting = false;    // Indica se há pedestre esperando

void setup() {
  // Configuração dos pinos
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT); // Configurando o buzzer como saída
  for (int i = 0; i < 5; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }

  // Configuração da Serial para debug
  Serial.begin(9600);

  // Configuração da interrupção para o botão
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounceButtonPressed, FALLING);

  // Inicializa o tempo do estado
  stateStartTime = millis();
}

void loop() {
  // Debug: exibe se há pedestre esperando
  Serial.print("Pedestrian Waiting: ");
  Serial.println(pedestrianWaiting);

  // Executa a lógica baseada no estado atual
  switch (currentState) {
    case STATE_CAR_GREEN:
      handleCarGreenState();
      break;
    case STATE_CAR_YELLOW:
      handleCarYellowState();
      break;
    case STATE_CAR_RED:
      handleCarRedState();
      break;
    case STATE_PEDESTRIAN_GREEN:
      handlePedestrianGreenState();
      break;
    case STATE_BLINK:
      handleBlinkState();
      break;
    default:
      Serial.println("Unknown state!");
      break;
  }
}

// Função para lidar com o estado de carro verde
void handleCarGreenState() {
  Serial.println("State: Car Green");
  carGreen();
  pedRed();

  if (isStateTimeOver(TIME_CAR_GREEN) && pedestrianWaiting) {
    transitionToState(STATE_CAR_YELLOW);
  }
}

// Função para lidar com o estado de carro amarelo
void handleCarYellowState() {
  Serial.println("State: Car Yellow");
  carYellow();
  pedRed();

  if (isStateTimeOver(TIME_CAR_YELLOW)) {
    transitionToState(STATE_CAR_RED);
  }
}

// Função para lidar com o estado de carro vermelho
void handleCarRedState() {
  Serial.println("State: Car Red");
  carRed();
  pedRed();

  if (isStateTimeOver(TIME_CAR_RED)) {
    transitionToState(STATE_PEDESTRIAN_GREEN);
  }
}

// Função para lidar com o estado de pedestre verde
void handlePedestrianGreenState() {
  Serial.println("State: Pedestrian Green");
  carRed();
  pedGreen();

  // Cálculo do tempo restante
  unsigned long elapsedTime = millis() - stateStartTime;
  unsigned long remainingTime = TIME_PEDESTRIAN_GREEN - elapsedTime;

  // Lógica para controlar o buzzer com base no tempo restante
  unsigned long buzzerInterval;
  if (remainingTime > TIME_PEDESTRIAN_GREEN / 2) {
    buzzerInterval = 1000; // Buzzer pisca devagar na primeira metade
  } else {
    buzzerInterval = 300; // Buzzer pisca rápido na segunda metade
  }

  // Faz o buzzer piscar
  if ((elapsedTime / buzzerInterval) % 2 == 0) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Transição para o próximo estado
  if (isStateTimeOver(TIME_PEDESTRIAN_GREEN)) {
    digitalWrite(BUZZER_PIN, LOW); // Certifique-se de desligar o buzzer ao sair do estado
    transitionToState(STATE_BLINK);
  }
}

// Função para lidar com o estado de piscar (blink)
void handleBlinkState() {
  Serial.println("State: Blink");
  carRed();

  // Animação Blink
  while (!isStateTimeOver(TIME_BLINK)) {
    digitalWrite(PED_RED_PIN, LOW);
    delay(200);
    digitalWrite(PED_RED_PIN, HIGH);
    delay(200);
  }

  // Transição de volta ao estado inicial
  pedestrianWaiting = false;
  transitionToState(STATE_CAR_GREEN);
}

// Controla a transição de estado
void transitionToState(int newState) {
  currentState = newState;
  stateStartTime = millis();
}

// Verifica se o tempo do estado atual acabou
bool isStateTimeOver(unsigned long stateDuration) {
  return millis() - stateStartTime > stateDuration;
}

// Funções auxiliares para controlar os LEDs
void carGreen() {
  digitalWrite(CAR_GREEN_PIN, HIGH);
  digitalWrite(CAR_YELLOW_PIN, LOW);
  digitalWrite(CAR_RED_PIN, LOW);
}

void carYellow() {
  digitalWrite(CAR_GREEN_PIN, LOW);
  digitalWrite(CAR_YELLOW_PIN, HIGH);
  digitalWrite(CAR_RED_PIN, LOW);
}

void carRed() {
  digitalWrite(CAR_GREEN_PIN, LOW);
  digitalWrite(CAR_YELLOW_PIN, LOW);
  digitalWrite(CAR_RED_PIN, HIGH);
}

void pedGreen() {
  digitalWrite(PED_GREEN_PIN, HIGH);
  digitalWrite(PED_RED_PIN, LOW);
}

void pedRed() {
  digitalWrite(PED_GREEN_PIN, LOW);
  digitalWrite(PED_RED_PIN, HIGH);
}

// Função chamada quando o botão é pressionado
void debounceButtonPressed() {
  static unsigned long lastPressTime = 0;
  if (millis() - lastPressTime >= 100) { // Debouncing
    pedestrianWaiting = true;
    lastPressTime = millis();
  }
}