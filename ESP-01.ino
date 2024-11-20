#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações de rede Wi-Fi
const char* ssid = "     ";        // Substitua pelo seu SSID
const char* password = "      ";  // Substitua pela sua senha Wi-Fi

// Configurações do broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// Tópicos MQTT
const char* topic_etanol = "sensor/etanol";
const char* topic_bomba = "estado/bomba";
const char* topic_combustivel = "tipo/combustivel";

// Configuração do cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Inicializa a comunicação serial
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
  client.setCallback(callback); // Função de callback para tratar mensagens

  // Aguardar conexão MQTT
  connectToMQTT();
}

void loop() {
  // Verifica se está conectado ao broker MQTT, caso contrário, tenta reconectar
  if (!client.connected()) {
    connectToMQTT();
  }

  client.loop();  // Processa as mensagens recebidas

  // Envia dados de etanol, tipo de combustível e estado da bomba periodicamente
  sendData();
}

void sendData() {
  // Envia o valor do etanol para o tópico "sensor/etanol"
  String ethanol = "50";  // Simulando um valor de etanol (este valor deve vir do Arduino)
  client.publish(topic_etanol, ethanol.c_str());

  // Envia o tipo de combustível para o tópico "tipo/combustivel"
  String fuelType = "ETHANOL";  // Exemplo: "ETHANOL", "GASOLINE", "NO FUEL", etc.
  client.publish(topic_combustivel, fuelType.c_str());

  // Envia o estado da bomba para o tópico "estado/bomba"
  String pumpState = "ON";  // Exemplo: "ON", "OFF"
  client.publish(topic_bomba, pumpState.c_str());

  delay(5000);  // Aguarda 5 segundos antes de enviar novamente
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado ao broker MQTT");
      // Inscreve-se nos tópicos necessários
      client.subscribe(topic_bomba);
    } else {
      Serial.print("Falha na conexão. Estado: ");
      Serial.println(client.state());
      delay(2000);  // Tenta novamente em 2 segundos
    }
  }
}

// Função de callback para tratar mensagens recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Se a mensagem for sobre o estado da bomba, aciona a bomba no ESP-01
  if (String(topic) == topic_bomba) {
    if (message == "LIGAR") {
      Serial.println("Bomba ligada!");
      // Aqui você pode controlar a bomba fisicamente com o GPIO do ESP-01 (se for necessário)
    } else if (message == "DESLIGAR") {
      Serial.println("Bomba desligada!");
      // Aqui você pode desligar a bomba fisicamente
    }
  }
}
