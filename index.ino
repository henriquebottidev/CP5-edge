#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "18.209.40.125"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/lamp003/cmd"; // Tópico MQTT de escuta
const char* default_TOPICO_PUBLISH_1 = "/TEF/lamp003/attrs"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_2 = "/TEF/lamp003/attrs/luminosity"; // Tópico MQTT de envio de informações para Broker
// --------------------------------------------------------- //
              // ADICIONADO CP5
const char* TOPICO_PUBLISH_TEMP = "/TEF/lamp003/attrs/t";  // Tópico para temperatura
const char* TOPICO_PUBLISH_HUM = "/TEF/lamp003/attrs/h";   // Tópico para umidade
// --------------------------------------------------------- //

const char* default_ID_MQTT = "fiware_003"; // ID MQTT
const int default_D4 = 2; // Pino do LED onboard
// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "lamp003";

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4;

// ADICIONADO CP5 // -----------------------------
#define DHTPIN 4         // Pino conectado ao DHT
#define DHTTYPE DHT22   // 
DHT dht(DHTPIN, DHTTYPE); // Inicializa o sensor DHT
// --------------------------------------------------------- //

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void setup() {
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    dht.begin(); // ADICIONADO CP5
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH_1, "s|on");
}

void loop() {
    VerificaConexoesWiFIEMQTT();
    EnviaEstadoOutputMQTT();
    handleLuminosity();
    handleTemperatureAndHumidity(); // ADICIONADO CP5
    MQTT.loop();
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garantir que o LED inicie desligado
    digitalWrite(D4, LOW);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Forma o padrão de tópico para comparação
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Compara com o tópico recebido
    if (msg.equals(onTopic)) {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }

    if (msg.equals(offTopic)) {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}

void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

void EnviaEstadoOutputMQTT() {
    if (EstadoSaida == '1') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|on");
        Serial.println("- Led Ligado");
    }

    if (EstadoSaida == '0') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|off");
        Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

void InitOutput() {
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    boolean toggle = false;

    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(D4, toggle);
        delay(200);
    }
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleLuminosity() {
    const int potPin = 34;
    int sensorValue = analogRead(potPin);
    int luminosity = map(sensorValue, 0, 4095, 0, 100);
    String mensagem = String(luminosity);
    Serial.print("Valor da luminosidade: ");
    Serial.println(mensagem.c_str());
    MQTT.publish(TOPICO_PUBLISH_2, mensagem.c_str());
}

unsigned long lastDHTReadTime = 0;
unsigned long intervalDHT = 2000; // 2 segundos de intervalo entre leituras

void handleTemperatureAndHumidity() {
    // Verifica se o intervalo de leitura foi alcançado
    if (millis() - lastDHTReadTime > intervalDHT) {
        lastDHTReadTime = millis();  // Atualiza o tempo da última leitura

        float temp = dht.readTemperature();  // Lê a temperatura
        float hum = dht.readHumidity();  // Lê a umidade

        if (isnan(temp) || isnan(hum)) {
            Serial.println("Falha na leitura do sensor DHT!");  // Mensagem de erro
            return;  // Sai da função se a leitura falhar
        }

        // Publicar temperatura
        String temperaturaStr = String(temp);
        Serial.print("Temperatura: ");
        Serial.println(temperaturaStr.c_str());
        MQTT.publish(TOPICO_PUBLISH_TEMP, temperaturaStr.c_str());  // Publica temperatura via MQTT

        // Publicar umidade
        String umidadeStr = String(hum);
        Serial.print("Umidade: ");
        Serial.println(umidadeStr.c_str());
        MQTT.publish(TOPICO_PUBLISH_HUM, umidadeStr.c_str());  // Publica umidade via MQTT
    }
}