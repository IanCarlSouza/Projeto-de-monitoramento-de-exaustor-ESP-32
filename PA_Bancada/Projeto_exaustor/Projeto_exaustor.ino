/*
  Projeto: Monitor de Exautores
  Autor: Ian Carlos de Souza Rivieira
  Copyright (c) 2024, Ian Carlos de Souza Rivieira.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_BMP280.h>

// Configuração da rede WiFi
const char* ssid = "";
const char* password = "";

// Configuração do MQTT
const char* broker = "";
const char* client_id = "ESP32";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

MPU6050 mpu;  // Instância do sensor MPU6050
Adafruit_BMP280 bmp;  // Instância do sensor BMP280

// Pinos dos sensores MPU6050/BMP280
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;

// Pino de leitura de tensão
const int analogPin = 34; // Pino ADC 
const int adc_ref = 4095;
const float v_ref = 3.3;
const float divisor = 3.0;

// Função para conectar ao WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  
  // Aguarda até que o ESP32 se conecte ao Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para conectar ao broker MQTT
bool connectMQTT() {
  int attempts = 0;
  while (!client.connected() && attempts < 5) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect(client_id)) {
      Serial.println("Conectado!");
      return true;
    } else {
      Serial.print("Falha ao conectar, tentativa ");
      Serial.println(attempts + 1);
      delay(5000);
      attempts++;
    }
  }
  Serial.println("Máximo de tentativas alcançado.");
  return false;
}

// Função para leitura da tensão
float leitura_Voltagem() {
  int adcVal = analogRead(analogPin);
  float voltagem = (adcVal * v_ref / adc_ref) * divisor;
  return voltagem;
}

// Função para publicar dados no MQTT separadamente
void publishData() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float temperatura = bmp.readTemperature();  // Temperatura
  float pressao = bmp.readPressure();  // Pressão
  float voltagem = leitura_Voltagem();  // Tensão

  // Publica os dados do MPU6050
  client.publish("scr/ian/vibracao", String(ax).c_str()); // Envia os dados dos eixos do sensor para o MQTT
  client.publish("scr/ian/vibracao", String(ay).c_str());
  client.publish("scr/ian/vibracao", String(az).c_str());
  client.publish("scr/ian/vibracao", String(gx).c_str());
  client.publish("scr/ian/vibracao", String(gy).c_str());
  client.publish("scr/ian/vibracao", String(gz).c_str());

  // Publica os dados do BMP280
  client.publish("scr/ian/temperatura", String(temperatura).c_str()); //Envia os dados de temperatura
  client.publish("scr/ian/pressao", String(pressao).c_str()); //Envia os dados de pressao

  // Publica os dados de corrente
  client.publish("scr/ian/corrente", String(voltagem, 2).c_str()); //Envia os dados da corrente
}

// Função pra reconexão MQTT
void ensureMQTTConnection() {
  if (!client.connected()) {
    if (connectMQTT()) {
      Serial.println("Reconectado ao MQTT.");
    } else {
      Serial.println("Falha ao reconectar ao MQTT.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Conecta o WiFi
  connectWiFi();

  // Configura o cliente MQTT
  client.setServer(broker, mqttPort);

  // Inicializa o MPU6050
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado!");
  } else {
    Serial.println("Falha ao conectar MPU6050");
  }

  // Inicializa o BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("Falha ao inicializar o BMP280!");
  } else {
    Serial.println("BMP280 conectado!");
  }

  // Conecta o broker MQTT
  connectMQTT();
}

void loop() { 
  // Verifica a conexão MQTT
  ensureMQTTConnection();

  // Publicação dos dados
  publishData();

  delay(1000);
}
