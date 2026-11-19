#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

const uint8_t PIN_TRIG = 5;
const uint8_t PIN_ECHO = 18;
const uint8_t PIN_DHT = 15;
const uint8_t PIN_LED_VERDE = 4;
const uint8_t PIN_LED_AMARELO = 25;
const uint8_t PIN_LED_VERMELHO = 26;
const uint8_t PIN_BUZZER = 27;

#define TIPO_DHT DHT22

const float LIMIAR_CRITICO_CM = 10.0;
const float LIMIAR_ATENCAO_CM = 20.0;
const float ALTURA_INSTALACAO_CM = 60.0;

const unsigned long INTERVALO_LEITURA_MS = 1000;
const unsigned long INTERVALO_DHT_MS = 2000;
const unsigned long INTERVALO_NUVEM_MS = 15000;
const unsigned long INTERVALO_PING_MS = 60;
const unsigned long TIMEOUT_ECHO_US = 30000;
const unsigned long TIMEOUT_WIFI_MS = 10000;
const unsigned int FREQUENCIA_ALARME_HZ = 2000;
const unsigned long PERIODO_ALARME_MS = 300;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";

// Cole aqui a Write API Key do seu canal (nao versione a chave real)
#define THINGSPEAK_API_KEY "SEU_WRITE_KEY"
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

DHT dht(PIN_DHT, TIPO_DHT);

enum Status { NORMAL, ATENCAO, CRITICO };

Status statusAtual = NORMAL;
unsigned long ultimaLeitura = 0;
unsigned long ultimaLeituraDht = 0;
unsigned long ultimoEnvioNuvem = 0;
unsigned long seq = 0;
bool buzzerLigado = false;
float temperatura = NAN;
float umidade = NAN;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  dht.begin();
  aplicarStatus(NORMAL);
  Serial.println(F("{\"event\":\"boot\",\"device\":\"EDGE-AV-BRASIL-01\",\"baud\":9600}"));
  conectarWifi();
}

void loop() {
  atualizarBuzzer();

  if (millis() - ultimaLeituraDht >= INTERVALO_DHT_MS) {
    ultimaLeituraDht = millis();
    lerClima();
  }

  if (millis() - ultimaLeitura < INTERVALO_LEITURA_MS) return;
  ultimaLeitura = millis();

  float distancia = lerDistanciaCm();
  if (distancia < 0.0f) {
    enviarSemLeitura();
    return;
  }

  statusAtual = classificar(distancia);
  aplicarStatus(statusAtual);
  enviarTelemetria(distancia, statusAtual);
  atualizarNuvem(distancia);
}

void conectarWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < TIMEOUT_WIFI_MS) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("# wifi conectado em "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("# wifi sem conexao - segue so na borda"));
  }
}

void lerClima() {
  float leituraTemperatura = dht.readTemperature();
  float leituraUmidade = dht.readHumidity();

  if (!isnan(leituraTemperatura)) temperatura = leituraTemperatura;
  if (!isnan(leituraUmidade)) umidade = leituraUmidade;
}

float medirPulsoCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracao = pulseIn(PIN_ECHO, HIGH, TIMEOUT_ECHO_US);
  if (duracao == 0) return -1.0f;

  return (duracao * 0.0343f) / 2.0f;
}

float lerDistanciaCm() {
  float validas[3];
  uint8_t total = 0;

  for (uint8_t i = 0; i < 5 && total < 3; i++) {
    float amostra = medirPulsoCm();
    if (amostra > 1.0f && amostra < 400.0f) validas[total++] = amostra;
    delay(INTERVALO_PING_MS);
  }

  if (total == 0) return -1.0f;
  if (total == 1) return validas[0];
  if (total == 2) return (validas[0] + validas[1]) / 2.0f;

  float a = validas[0], b = validas[1], c = validas[2];
  if ((a >= b && a <= c) || (a >= c && a <= b)) return a;
  if ((b >= a && b <= c) || (b >= c && b <= a)) return b;
  return c;
}

Status classificar(float distanciaCm) {
  if (distanciaCm < LIMIAR_CRITICO_CM) return CRITICO;
  if (distanciaCm <= LIMIAR_ATENCAO_CM) return ATENCAO;
  return NORMAL;
}

void aplicarStatus(Status status) {
  digitalWrite(PIN_LED_VERDE, status == NORMAL);
  digitalWrite(PIN_LED_AMARELO, status == ATENCAO);
  digitalWrite(PIN_LED_VERMELHO, status == CRITICO);
}

void atualizarBuzzer() {
  bool ligar = statusAtual == CRITICO && (millis() % (2 * PERIODO_ALARME_MS)) < PERIODO_ALARME_MS;
  if (ligar == buzzerLigado) return;

  buzzerLigado = ligar;
  if (ligar) tone(PIN_BUZZER, FREQUENCIA_ALARME_HZ);
  else noTone(PIN_BUZZER);
}

const char* nomeStatus(Status status) {
  switch (status) {
    case CRITICO: return "CRITICO";
    case ATENCAO: return "ATENCAO";
    default: return "NORMAL";
  }
}

void imprimirClima() {
  Serial.print(F(",\"temp_c\":"));
  if (isnan(temperatura)) Serial.print(F("null"));
  else Serial.print(temperatura, 1);

  Serial.print(F(",\"hum_pct\":"));
  if (isnan(umidade)) Serial.print(F("null"));
  else Serial.print(umidade, 1);
}

void enviarTelemetria(float distanciaCm, Status status) {
  seq++;
  Serial.print(F("{\"seq\":"));
  Serial.print(seq);
  Serial.print(F(",\"dist_cm\":"));
  Serial.print(distanciaCm, 1);
  imprimirClima();
  Serial.print(F(",\"status\":\""));
  Serial.print(nomeStatus(status));
  Serial.print(F("\",\"alert\":"));
  Serial.print(status == NORMAL ? F("false") : F("true"));
  Serial.print(F(",\"uptime_ms\":"));
  Serial.print(millis());
  Serial.println(F("}"));
}

void enviarSemLeitura() {
  seq++;
  Serial.print(F("{\"seq\":"));
  Serial.print(seq);
  Serial.print(F(",\"dist_cm\":null"));
  imprimirClima();
  Serial.print(F(",\"status\":\"SEM_LEITURA\",\"alert\":false,\"uptime_ms\":"));
  Serial.print(millis());
  Serial.println(F("}"));
}

void atualizarNuvem(float distanciaCm) {
  if (strcmp(THINGSPEAK_API_KEY, "SEU_WRITE_KEY") == 0) return;
  if (millis() - ultimoEnvioNuvem < INTERVALO_NUVEM_MS) return;
  ultimoEnvioNuvem = millis();

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    Serial.println(F("# nuvem: wifi desconectado, tentando reconectar"));
    return;
  }

  float nivel = ALTURA_INSTALACAO_CM - distanciaCm;
  if (nivel < 0.0f) nivel = 0.0f;

  String corpo = "api_key=";
  corpo += THINGSPEAK_API_KEY;
  corpo += "&field1=";
  corpo += String(distanciaCm, 1);
  corpo += "&field2=";
  corpo += String(nivel, 1);
  corpo += "&field3=";
  corpo += String(static_cast<int>(statusAtual));
  corpo += "&field4=";
  corpo += String(statusAtual == NORMAL ? 0 : 1);
  if (!isnan(temperatura)) {
    corpo += "&field5=";
    corpo += String(temperatura, 1);
  }
  if (!isnan(umidade)) {
    corpo += "&field6=";
    corpo += String(umidade, 1);
  }

  WiFiClient cliente;
  HTTPClient http;
  http.setTimeout(4000);

  if (!http.begin(cliente, THINGSPEAK_URL)) {
    Serial.println(F("# nuvem: falha ao iniciar HTTP"));
    return;
  }

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int codigo = http.POST(corpo);
  http.end();

  Serial.print(F("# nuvem: HTTP "));
  Serial.println(codigo);
}
