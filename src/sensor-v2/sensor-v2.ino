//Bitmaps
#include "bitmaps.h"

//Libraries for LoRa
#include <SPI.h>
#include "LoRa.h"

//Libraries for BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//Display
#include "display.h"

//Configuration
#include "configuration.h"

//Prepare BME280 Sensor
#define SDA 21
#define SCL 22

Adafruit_BME280 bme;
//Prepare a second I2C wire for Sensor
TwoWire I2Cone = TwoWire(1);

//Prepare LoRa
#define BAND    868E6

//Sensor config
#define ROLLING_AVERAGE_DATAPOINT_NUMBER 20

//Deep Sleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

esp_sleep_wakeup_cause_t wakeup_reason;

RTC_DATA_ATTR unsigned int bootCount = 0;
RTC_DATA_ATTR unsigned int loopCounter;

float temperature;
float humidity;
float pressure;

RTC_DATA_ATTR float averageTemperature;
RTC_DATA_ATTR float averageHumidity;

RTC_DATA_ATTR float lastSentTemperature;
RTC_DATA_ATTR float lastSentHumidity;

char temperatureRounded[4];

String moduleUniqueidentifier;
float temperatureCompensation;

void initializeLoRa(bool displayOutput, int delayDuration) {

  if (displayOutput) {
    displayClear();
    displaySmallText(0, 0, "Initialize");
    displayLargeText(0, 20, "LoRa");
    displayDraw();
  }

  Serial.println(F("Initialize LoRa"));
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(18, 14, 26);

  if (!LoRa.begin(BAND))
  {
    Serial.println(F("LoRa initialization failed!"));
    while (1);
  }

  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN); //Supported values are 2 to 20 for PA_OUTPUT_PA_BOOST_PIN

  LoRa.setPreambleLength(8); //ranges from 6-65535
  LoRa.setSyncWord(0x91); //ranges from 0-0xFF, default 0x34, see API docs

  //setLoraProfileLongRange();
  setLoraProfileUltraLongRange();

  //Debugging
  //LoRa.dumpRegisters(Serial);

  delay(delayDuration);
}

void setLoraProfileLongRange() {
  //Lora Send Configuration examples
  //https://github.com/dragino/Arduino-Profile-Examples/blob/master/libraries/Dragino/examples/LoRa/LoRa_Simple_Client_Arduino/LoRa_Simple_Client_Arduino.ino
  //Documentation Source: https://josefmtd.com/2018/08/14/spreading-factor-bandwidth-coding-rate-and-bit-rate-in-lora-english/
  LoRa.setSignalBandwidth(31.25E3);
  //LoRa modulation also adds a forward error correction (FEC) in every data transmission.
  //This implementation is done by encoding 4-bit data with redundancies into 5-bit, 6-bit, 7-bit, or even 8-bit.
  //Using this redundancy will allow the LoRa signal to endure short interferences.
  //The Coding Rate (CR) value need to be adjusted according to conditions of the channel used for data transmission.
  //If there are too many interference in the channel, then it’s recommended to increase the value of CR.
  //However, the rise in CR value will also increase the duration for the transmission
  LoRa.setCodingRate4(8); //ranges from 5-8, default 5
  //The value of Spreading Factor (SF) determines how many chips used to represent a symbol.
  //The higher the SF value is, the more chips used to represent a symbol, which means there will be more processing gain from the receiver side.
  //This will allow receiver to accept data signals with negative SNR value
  LoRa.setSpreadingFactor(9); //ranges from 7-12, default 7
}

void setLoraProfileUltraLongRange() {
  //Lora Send Configuration examples
  //https://github.com/dragino/Arduino-Profile-Examples/blob/master/libraries/Dragino/examples/LoRa/LoRa_Simple_Client_Arduino/LoRa_Simple_Client_Arduino.ino
  //Documentation Source: https://josefmtd.com/2018/08/14/spreading-factor-bandwidth-coding-rate-and-bit-rate-in-lora-english/
  LoRa.setSignalBandwidth(125E3);
  //LoRa modulation also adds a forward error correction (FEC) in every data transmission.
  //This implementation is done by encoding 4-bit data with redundancies into 5-bit, 6-bit, 7-bit, or even 8-bit.
  //Using this redundancy will allow the LoRa signal to endure short interferences.
  //The Coding Rate (CR) value need to be adjusted according to conditions of the channel used for data transmission.
  //If there are too many interference in the channel, then it’s recommended to increase the value of CR.
  //However, the rise in CR value will also increase the duration for the transmission
  LoRa.setCodingRate4(8); //ranges from 5-8, default 5
  //The value of Spreading Factor (SF) determines how many chips used to represent a symbol.
  //The higher the SF value is, the more chips used to represent a symbol, which means there will be more processing gain from the receiver side.
  //This will allow receiver to accept data signals with negative SNR value
  LoRa.setSpreadingFactor(12); //ranges from 7-12, default 7
}

void terminateLoRa() {
  LoRa.end();
  SPI.end();
}

void initializeTemperatureSensor(bool displayOutput, int delayDuration) {

  if (displayOutput) {
    displayClear();
    displaySmallText(0, 0, "Initialize");
    displayLargeText(0, 20, "T&H");
    displayNormalText(56, 28, "Sensor");
    displayDraw();
  }

  //Set Pins for BME280 Sensor
  I2Cone.begin(SDA, SCL, 100000);

  if (!bme.begin(0x76, &I2Cone)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1);
  }

  //The sensor requires special protection so that it does not falsify the measurement data due to its own heat
  bme.setSampling(Adafruit_BME280::MODE_FORCED,   //Query the sensor data only on command
                  Adafruit_BME280::SAMPLING_X1, //Temperature
                  Adafruit_BME280::SAMPLING_X1, //Pressure
                  Adafruit_BME280::SAMPLING_X1, //Humidity
                  Adafruit_BME280::FILTER_X2);  //Specifies how many samples are required until in the case of an abrupt change in the measured value the data output has followed at least 75% of the change

  bme.takeForcedMeasurement();

  //Set value from config
  float temperatureCompensation = getTemperatureCompensation();
  bme.setTemperatureCompensation(temperatureCompensation);

  Serial.print("TemperatureCompensation: ");
  Serial.println(temperatureCompensation);

  delay(delayDuration);
}

void showLogo() {
  displayClear();
  displayLogo();
  displayDraw();
  delay(1500);
}

void showModuleInfo() {
  displayClear();
  displaySmallText(0, 0, "Module Identifier");
  displayLargeText(0, 20, moduleUniqueidentifier);
  displayDraw();

  delay(2000);
}

void showSystemReady() {
  displayClear();
  displaySmallText(0, 0, "System Status");
  displayLargeText(0, 20, "Ready");
  displaySmallText(0, 50, "disable Display");
  displayDraw();

  delay(2500);
}

void setConfiguration() {
  //TODO: set display text "set configuration via serial"

  while (!Serial.available()) {
    Serial.println(F("Configuration mode active, please set ModuleUniqueidentifier via 'config.muid=XXX'"));
    delay(2000);
  }

  String uniqueidentifier = Serial.readString();
  Serial.println(uniqueidentifier);

  setModuleUniqueidentifier(uniqueidentifier);

  delay(100);
  ESP.restart();
}

void checkCommandAvailable() {

  displayClear();
  displaySmallText(0, 0, "Configuration Mode");
  displayLargeText(0, 20, "Active");
  displayDraw();

  for (int i = 0; i <= 5; i++) {
    Serial.println("check command available");
    if (Serial.available() > 0) {
      String command = Serial.readString();
      if (command.equalsIgnoreCase("reset")) {
        ESP.restart();
      }

      if (command.equalsIgnoreCase("send")) {
        //sendLoraPackage();
      }

      if (command.startsWith("config.muid=")) {
        Serial.println("set muid config");
        String value = command.substring(12);

        setModuleUniqueidentifier(value);
      }

      if (command.startsWith("config.tc=")) {
        Serial.println("set tc config");
        String value = command.substring(10);

        temperatureCompensation = value.toFloat();

        setTemperatureCompensation(temperatureCompensation);
        bme.setTemperatureCompensation(temperatureCompensation);
      }
    }
    delay(1000);
  }
}

double calculateAverageTemperature (double currentTemperature) {
  averageTemperature -= averageTemperature / ROLLING_AVERAGE_DATAPOINT_NUMBER;
  averageTemperature += currentTemperature / ROLLING_AVERAGE_DATAPOINT_NUMBER;
}

double calculateAverageHumidity (double currentHumidity) {
  averageHumidity -= averageHumidity / ROLLING_AVERAGE_DATAPOINT_NUMBER;
  averageHumidity += currentHumidity / ROLLING_AVERAGE_DATAPOINT_NUMBER;
}

void sendLoraPackage() {
  Serial.println("Send temperature via LoRa");

  int isReady = LoRa.beginPacket();
  Serial.println("LoRa isReady:" + String(isReady));
  LoRa.print(moduleUniqueidentifier + "#t:" + String(temperature) + "#h:" + String(humidity));
  LoRa.endPacket();

  Serial.println("Send done");
}

void setup() {
  ++bootCount;

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  wakeup_reason = esp_sleep_get_wakeup_cause();

  //Prepare Serial connection
  Serial.begin(115200);
  Serial.setTimeout(250);

  moduleUniqueidentifier = getModuleUniqueidentifier();
  if (moduleUniqueidentifier.length() == 0) {
    setConfiguration();
  }

  if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println("Initialize system");

    if (heap_caps_check_integrity_all(true)) {
      Serial.println("heap integrity is good");
    } else {
      Serial.println("heap integrity has errors");
    }

    resetOledDisplay();
    initializeOledDisplay();
    showLogo();
    showModuleInfo();
    initializeLoRa(true, 500);
    initializeTemperatureSensor(true, 500);

    terminateLoRa();

    averageTemperature = bme.readTemperature();
    averageHumidity = bme.readHumidity();

    checkCommandAvailable();

    Serial.println("System is ready");
    showSystemReady();

    displayClear();
    displayDraw();
  }

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    initializeTemperatureSensor(false, 0);
  }

  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
}

void loop() {
  Serial.println("read sensor data");
  bme.takeForcedMeasurement();
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;

  calculateAverageTemperature(temperature);
  calculateAverageHumidity(humidity);

  Serial.print("Temperature:" + String(temperature) + ", ");
  Serial.print("Humidity:" + String(humidity) + ", ");
  Serial.println("Pressure:" + String(pressure / 100.0F));

  int differenceTemperature = abs((lastSentTemperature - averageTemperature) * 100);
  if (differenceTemperature > 10 || loopCounter > 60) //send info on a change of 0.1% or after 5 minutes
  {
    initializeLoRa(false, 0);
    sendLoraPackage();
    terminateLoRa();
    lastSentTemperature = averageTemperature;
    loopCounter = 0;
  }

  loopCounter++;

  Serial.println("activate deep sleep");
  esp_deep_sleep_start();
}
