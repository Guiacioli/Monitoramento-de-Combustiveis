#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações de rede Wi-Fi
const char* ssid = "wifi_ssid";
const char* password = "wifi_senha";

// Configurações do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// Tópicos MQTT
const char* topic_etanol = "sensor/etanol";
const char* topic_bomba = "estado/bomba";
const char* topic_combustivel = "tipo/combustivel";

// Inicializa o display no endereço 0x27, com 16 colunas e 4 linhas
LiquidCrystal_I2C lcd(0x27, 16, 4);

int analyzer = 0, comb1 = 0;
const int relayPin = 7; // Pino digital para o relé

// Limites de combustível
const int ethanolLowerThreshold = 80; // Limite para etanol puro
const int gasolineUpperThreshold = 40; // Limite para gasolina pura
const int adulterationLowerThreshold = 41; // Limite inferior para adulteração
const int adulterationUpperThreshold = 79; // Limite superior para adulteração

unsigned long pumpTimer = 0; // Temporizador para desligar a bomba após 5 segundos
bool pumpActive = true; // Estado da bomba
String fuelType = "NO FUEL"; // Tipo de combustível

// Variáveis para a conexão MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void analy() {
  analyzer++;
}

void setup() {
  // Inicia a comunicação Serial
  Serial.begin(9600);
  
  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado!");

  // Configura o servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Define a função de callback

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Liga a bomba inicialmente (se o relé for acionado com LOW)

  attachInterrupt(0, analy, FALLING);

  lcd.init();  // Inicializa o LCD
  lcd.setBacklight(HIGH);  // Ativa a retroiluminação

  // Exibe "GUILHERME ACIOLI" na primeira linha
  lcd.setCursor(0, 0);
  lcd.print("GUILHERME ACIOLI");
  delay(4000);  // Aguarda por 4 segundos

  lcd.clear();  // Limpa o display

  // Exibe "ETHANOL SENSOR" na terceira linha
  lcd.setCursor(0, 2);
  lcd.print("ETHANOL SENSOR");
  delay(2000);  // Aguarda por 2 segundos
  lcd.clear();

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 31250;  // Ajusta a temporização
  TCCR1B |= (1 << WGM12);  // Configura o timer
  TCCR1B |= (1 << CS12);   // Inicia o timer
  TIMSK1 |= (1 << OCIE1A); // Habilita a interrupção do timer
  interrupts();

  lcd.clear();
}

ISR(TIMER1_COMPA_vect) {  // Interrupção do timer
  comb1 = analyzer * 2 - 50;  // Calcula o percentual de etanol
  analyzer = 0;

  // Limita o valor do percentual de etanol para não ultrapassar 100% nem ser negativo
  if (comb1 > 100) {
    comb1 = 100;
  } else if (comb1 < 0) {
    comb1 = 0;
  }
}

void loop() {
  // Verifica a conexão com o broker MQTT
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Exibe o título na primeira linha
  lcd.setCursor(3, 0);
  lcd.print("FUEL ANALYZER");

  // Exibe o percentual de etanol na segunda linha
  lcd.setCursor(0, 1);
  lcd.print("ETHANOL= ");
  if (comb1 > 2) {
    lcd.print(comb1);  // Exibe o valor do etanol
    lcd.print("%       ");
  } else {
    lcd.print("NO FUEL");  // Caso não haja combustível
  }

  // Determina o tipo de combustível e controla a bomba
  lcd.setCursor(0, 2);
  if (comb1 <= gasolineUpperThreshold && comb1 >= 2) {
    fuelType = "GASOLINE";
    lcd.print("FUEL: GASOLINE   ");
    pumpActive = true;  // Bomba ligada para gasolina
  } else if (comb1 >= ethanolLowerThreshold && comb1 <= 100) {
    fuelType = "ETHANOL";
    lcd.print("FUEL: ETHANOL    ");
    pumpActive = true;  // Bomba ligada para etanol
  } else if (comb1 >= adulterationLowerThreshold && comb1 <= adulterationUpperThreshold) {
    fuelType = "ADULTERATED";
    lcd.print("FUEL: ADULTERATED");
    pumpActive = true;  // Bomba ligada para combustível adulterado

    // Desliga a bomba após 5 segundos
    if (millis() - pumpTimer > 5000) {
      pumpActive = false; // Desliga a bomba após 5 segundos
    }
  } else {
    fuelType = "NO FUEL";
    lcd.print("FUEL: NO FUEL    ");
    pumpActive = false; // Sem combustível
  }

  // Atualiza o estado do relé (bomba)
  if (pumpActive) {
    digitalWrite(relayPin, LOW);  // Liga a bomba
  } else {
    digitalWrite(relayPin, HIGH);  // Desliga a bomba
  }

  // Envia as informações completas para o ESP-01
  Serial.print("sensor/etanol=");
  Serial.println(comb1);  // Envia o percentual de etanol para o ESP-01

  Serial.print("estado/bomba=");
  Serial.println(pumpActive ? "ON" : "OFF");  // Envia o estado da bomba para o ESP-01

  Serial.print("tipo/combustivel=");
  Serial.println(fuelType);  // Envia o tipo de combustível para o ESP-01

  // Exibe o estado da bomba na quarta linha
  lcd.setCursor(0, 3);
  lcd.print(pumpActive ? "PUMP ON         " : "PUMP OFF        ");

  delay(500);  // Aguarda 500ms antes de atualizar
}

// Função para conectar ao MQTT
void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado ao broker MQTT");
      // Se conectar, inscreve-se nos tópicos necessários
      client.subscribe("estado/bomba");
    } else {
      Serial.print("Falha na conexão. Estado: ");
      Serial.println(client.state());
      delay(2000);  // Tenta novamente em 2 segundos
    }
  }
}

// Função para tratar os comandos recebidos via MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "estado/bomba") {
    if (message == "LIGAR") {
      digitalWrite(relayPin, LOW);  // Liga a bomba
    } else if (message == "DESLIGAR") {
      digitalWrite(relayPin, HIGH);  // Desliga a bomba
    }
  }
}
