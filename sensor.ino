#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// === PINOS ===
#define DHTPIN 2
#define DHTTYPE DHT22
#define LDR_PIN A0
#define LED_VERDE 11
#define LED_AMARELO 12
#define LED_VERMELHO 13
#define BUZZER 8

// === OBJETOS ===
DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === VARIÁVEIS DE MÉDIA ===
int leituraCount = 0;
float somaTemp = 0, somaUmid = 0, somaLuz = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Iniciado");

  Serial.println("Sistema Iniciado");
  Serial.println("-------------------------");
}

void loop() {
  sensors_event_t tempEvent, umidEvent;
  dht.temperature().getEvent(&tempEvent);
  dht.humidity().getEvent(&umidEvent);

  int valorLDR = analogRead(LDR_PIN);
  float luz = map(valorLDR, 0, 1023, 100, 0); // Inverso para % de luz

  float temp = tempEvent.temperature;
  float umid = umidEvent.relative_humidity;

  if (!isnan(temp) && !isnan(umid)) {
    somaTemp += temp;
    somaUmid += umid;
    somaLuz += luz;
    leituraCount++;

    Serial.println("Leitura feita");
    Serial.print("Temp: ");
    Serial.println(temp);
    Serial.print("Umid: ");
    Serial.println(umid);
    Serial.print("Luz: ");
    Serial.println(luz);
    Serial.print("Contagem de leituras: ");
    Serial.println(leituraCount);
  } else {
    Serial.println("Erro ao ler sensor DHT.");
    return;
  }

  if (leituraCount >= 5) {
    float tempMedia = somaTemp / leituraCount;
    float umidMedia = somaUmid / leituraCount;
    float luzMedia = somaLuz / leituraCount;

    Serial.println("==== MÉDIAS CALCULADAS ====");
    Serial.print("Temp Média: ");
    Serial.println(tempMedia);
    Serial.print("Umid Média: ");
    Serial.println(umidMedia);
    Serial.print("Luz Média: ");
    Serial.println(luzMedia);
    Serial.println("-------------------------");

    Serial.print("Temp: ");
    Serial.print(tempMedia);
    Serial.println(" C");

    Serial.print("Umid: ");
    Serial.print(umidMedia);
    Serial.println(" %");

    Serial.print("Luz: ");
    Serial.println(luzMedia);
    Serial.println("-------------------------");

    // Reset para próxima média
    leituraCount = 0;
    somaTemp = somaUmid = somaLuz = 0;

    // Limpa LEDs e buzzer
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    noTone(BUZZER);

    // === EXIBE LUZ ===
    lcd.clear();
    if (luzMedia <= 30) {
      digitalWrite(LED_VERDE, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Ambiente Escuro");
    } else if (luzMedia <= 70) {
      digitalWrite(LED_AMARELO, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Meia luz");
    } else {
      digitalWrite(LED_VERMELHO, HIGH);
      tone(BUZZER, 1000);
      lcd.setCursor(0, 0);
      lcd.print("Muito Claro");
    }
    lcd.setCursor(0, 1);
    lcd.print("Luz: ");
    lcd.print(luzMedia, 1);
    delay(2000);

    // === EXIBE TEMPERATURA ===
    lcd.clear();
    lcd.setCursor(0, 0);
    if (tempMedia > 30) { // Alterado para 30°C como limite
      digitalWrite(LED_AMARELO, HIGH);
      tone(BUZZER, 1000);
      lcd.print("Temp. Alta");
    } else if (tempMedia < 10) { // Mantido o limite inferior de 10°C
      digitalWrite(LED_AMARELO, HIGH);
      tone(BUZZER, 1000);
      lcd.print("Temp. Baixa");
    } else {
      lcd.print("Temp. OK");
    }
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(tempMedia, 1);
    lcd.print(" C");
    delay(2000);

    // === EXIBE UMIDADE ===
    lcd.clear();
    lcd.setCursor(0, 0);
    if (umidMedia < 30) { // Alterado para 30% como limite de umidade baixa
      digitalWrite(LED_VERMELHO, HIGH);
      tone(BUZZER, 1000);
      lcd.print("Umidade Baixa");
    } else if (umidMedia > 70) {
      digitalWrite(LED_VERMELHO, HIGH);
      tone(BUZZER, 1000);
      lcd.print("Umidade Alta");
    } else {
      lcd.print("Umidade OK");
    }
    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(umidMedia, 1);
    lcd.print(" %");
    delay(2000);

    // Desliga buzzer no final do ciclo
    noTone(BUZZER);
  }

  delay(1000); // Aguarda entre as leituras
}
