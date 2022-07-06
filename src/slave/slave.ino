//Libraries for LoRa
#include <SPI.h>
#include "LoRa.h"

//Libraries for BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//Libraries for OLED Display SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Prepare BME280 Sensor
#define SDA 21
#define SCL 22

Adafruit_BME280 bme;
//Prepare a second I2C wire for Sensor
TwoWire I2Cone = TwoWire(1);

//Prepare OLED Display SSD1306
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Prepare LoRa
#define BAND    868E6

//Identifier
#define MODULE_IDENTIFIER "SENSOR1"

//Sensor config
#define ROLLING_AVERAGE_DATAPOINT_NUMBER 100

double temperature;
double averageTemperature;
double humidity;

unsigned int loopCounter;

void displaySmallText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(1);
  display.print(text);
}

void displayNormalText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(2);
  display.print(text);
}

void displayBigText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(3);
  display.print(text);
}

void resetOledDisplay() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
}

void initializeOledDisplay() {
  Serial.println(F("Initialize SSD1306 display"));
  //Open connection to Display
  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
}

void initializeLoRa() {
  display.clearDisplay();
  displayNormalText(5, 10, "Initialize");
  displayNormalText(5, 35, "LoRa");
  display.display();

  Serial.println(F("Initialize LoRa"));
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST_LoRa,DIO0);
  if (!LoRa.begin(BAND))
  {
    Serial.println(F("LoRa initialization failed!"));
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
}

void initializeTemperatureSensor() {
  //Set Pins for BME280 Sensor
  I2Cone.begin(SDA, SCL, 100000); 
  
  if (!bme.begin(0x76, &I2Cone)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1);
  }

  averageTemperature = bme.readTemperature();
}

double calculateAverageTemperature (double currentTemperature) {
  averageTemperature -= averageTemperature / ROLLING_AVERAGE_DATAPOINT_NUMBER;
  averageTemperature += currentTemperature / ROLLING_AVERAGE_DATAPOINT_NUMBER;
}

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize");

  resetOledDisplay();
  initializeOledDisplay();
  initializeLoRa();
  initializeTemperatureSensor();
}

void loop() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();

  calculateAverageTemperature(temperature);

  if (loopCounter % 20 == 0 || loopCounter == 0)
  {
    LoRa.beginPacket();
    LoRa.print(String(MODULE_IDENTIFIER) + "#t:" + String(temperature) + "#h:" + String(humidity));
    LoRa.endPacket();
  }

  Serial.println("Temperature = " + String(temperature) + "*C");
  Serial.println("Humidity = " + String(humidity) + "%");

  display.clearDisplay();
  displaySmallText(0, 0, "Temperature (degree)");
  displayNormalText(0, 12, String(temperature));
  displaySmallText(0, 35, "Humidity (percentage)");
  displayNormalText(0, 47, String(humidity));  
  display.display();

  Serial.println();

  loopCounter++;
  delay(1000);
}
