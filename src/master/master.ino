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

double temperature;
double humidity;
int counter;

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize");

  //Reset Display
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //Open connection to Display
  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  //Activate Lora
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST_LoRa,DIO0);
  if (!LoRa.begin(BAND))
  {
    Serial.println("LORA initialization failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);

  //Set Pins for BME280 Sensor
  I2Cone.begin(SDA, SCL, 100000); 
  
  if (!bme.begin(0x76, &I2Cone)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();

  if (counter % 20 == 0 || counter == 0)
  {
    LoRa.beginPacket();
    LoRa.print("t:" + String(temperature) + "#h:" + String(humidity));
    LoRa.endPacket();
  }
  
  if (counter == 600)
  {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(5,10);
    display.setTextSize(2);
    display.print("I love you");
  
    display.setTextColor(WHITE);
    display.setCursor(30,35);
    display.setTextSize(3);
    display.print("Baby");
  
    display.display();
  
    delay(5000);
    
    counter = 0;
    return;
  }

  Serial.println("Temperature = " + String(temperature) + "*C");
  Serial.println("Humidity = " + String(humidity) + "%");

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("Temperature (degree)");

  display.setTextColor(WHITE);
  display.setCursor(0,12);
  display.setTextSize(2);
  display.print(String(temperature));

  display.setTextColor(WHITE);
  display.setCursor(0,35);
  display.setTextSize(1);
  display.print("Humidity (percentage)");

  display.setTextColor(WHITE);
  display.setCursor(0,47);
  display.setTextSize(2);
  display.print(String(humidity));
  
  display.display();

  Serial.println();

  counter++;
  delay(1000);
}
