#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Parametros e variaveis de conexão
const char* ssid = "adcione o nome da sua rede"; //Rede  
const char* password = "Adcione a senha da sua rede"; //Senha
const char* mqtt_server = "IP/endereço servidor"; //servidor mqtt

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi(){
  //Conectar
  WiFi.begin(ssid, password);

  //loop de conexão
  Serial.println("");
  Serial.print("Procurando...");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");

  }
  Serial.println("");
  Serial.println("Conectado!");

  //IP
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char*topic, byte* payload, unsigned int length){
  Serial.print("Chegou, caralho![");
  Serial.print(topic);
  Serial.print("]");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect(){
  // loop para reconexão
  while (!client.connected()){
    Serial.print("Aguarde a conexão do MQTT...");
    // ID Aleatorio
    String clientId = "ESP8266Client-";
    clientId +=String(random(0xfff), HEX);
    // Reconectando
    if(client.connect(clientId.c_str())){
      Serial.print("Conectado!");
      //mensagem ao se conectar
      client.publish("outTopic", "Hello, World!");
      client.subscribe("inTopic");
    }
    else{
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println("Tentando denovo em 5 segundos...");
      delay(5000);
      }
  }
}
void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void loop(){
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  
  //Informaçoes do sensor para o MQTT
  unsigned long now = millis();
  if (now - lastMsg > 2000){
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "Mensagem #%ld", value); //mensagem que sera enviada para o broker
    Serial.print("Mensagem publicada: ");
    Serial.println(msg);
    client.publish("Mensagem", msg);
  }
}
