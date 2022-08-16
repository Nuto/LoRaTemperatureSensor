//Secrets
#include "secrets.h"

//Bitmaps
#include "bitmaps.h"

//Libraries for LORA
#include <SPI.h>
#include "LoRa.h"

//Libraries for WLAN
#include <WiFi.h>

//Display
#include "display.h"

//Prepare WLAN (please enter your sensitive data in the tab -> arduino_secrets.h)
const char* ssid = SECRET_WLAN_SSID;
const char* password = SECRET_WLAN_PASS;

//Libraries for WebRequests
#include <HTTPClient.h>

//Prepare HttpClient
HTTPClient httpClient;

//Prepare LORA
#define BAND    868E6

unsigned long previousMillis = 0UL;
unsigned long interval = 1000UL; //1s

#define BUTTON_PIN 0
int buttonPressed = 0;

volatile unsigned int receiveCounter;
volatile bool loraDataAvailable;
String receivedData;

unsigned int failureCounter;

void onReceive(int packetSize) {
  receiveCounter++;
  loraDataAvailable = true;
}

void initializeLoRa() {
  displayClear();
  displaySmallText(0, 0, "Initialize");
  displayLargeText(0, 20, "LoRa");
  displayDraw();

  Serial.println(F("Initialize LoRa"));
  SPI.begin(SCK, MISO, MOSI, SS);
  //LoRa.setPins(SS,RST_LoRa,DIO0);
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

  LoRa.onReceive(onReceive);
  LoRa.receive(); //activate receive mode

  delay(500);
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

void initializeWLAN() {
  Serial.print(F("Initialize WLAN"));

  displayClear();
  displaySmallText(0, 0, "Initialize");
  displayLargeText(0, 20, "WLAN");
  displayDraw();

  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("LDTS-Gateway");
  WiFi.begin(ssid, password);

  byte retryCount = 0;
  byte maxRetries = 15;
  Serial.print(F(", try connect["));
  while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) {
    retryCount++;
    delay(500);
    Serial.print(".");
  }
  Serial.print(F("]"));

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    displaySmallText(0, 44, "Connected");
    displayDraw();
    delay(1000);

    return;
  } else {
    displaySmallText(0, 44, "Cannot connect");
    displayDraw();

    Serial.println(F("WLAN initialization failed!"));
    while (1);
  }
}

void showLogo() {
  displayClear();
  displayLogo();
  displayDraw();
  delay(2000);
}

void sendWebRequest(String graphName, String temperature) {
  String httpRequestData = "0," + graphName + "," + temperature;

  httpClient.setConnectTimeout(2000);
  //httpClient.begin("https://webhook.site/5e077421-31f1-4ab2-a6b3-a52579a1f652");
  httpClient.begin("http://iotplotter.com/api/v2/feed/831079989972422411.csv");
  httpClient.addHeader("api-key", "");
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpClient.setTimeout(5000); //5 seconds
  int httpCode = httpClient.POST(httpRequestData);
  analyzeSystemHttpCodes(httpCode);
  httpClient.end();

  Serial.println("iotplotter.com response code:" + String(httpCode) + ", Send Data(" + httpRequestData + ")");
  if (httpCode != 200) {
    failureCounter++;
  }
}

void analyzeSystemHttpCodes(int httpCode) {
  if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
    Serial.println(F("HTTPC_ERROR_CONNECTION_REFUSED"));
  } else if (httpCode == HTTPC_ERROR_SEND_HEADER_FAILED) {
    Serial.println(F("HTTPC_ERROR_SEND_HEADER_FAILED"));
  } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
    Serial.println(F("HTTPC_ERROR_SEND_PAYLOAD_FAILED"));
  } else if (httpCode == HTTPC_ERROR_NOT_CONNECTED) {
    Serial.println(F("HTTPC_ERROR_NOT_CONNECTED"));
  } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
    Serial.println(F("HTTPC_ERROR_CONNECTION_LOST"));
  } else if (httpCode == HTTPC_ERROR_NO_STREAM) {
    Serial.println(F("HTTPC_ERROR_NO_STREAM"));
  } else if (httpCode == HTTPC_ERROR_NO_HTTP_SERVER) {
    Serial.println(F("HTTPC_ERROR_NO_HTTP_SERVER"));
  } else if (httpCode == HTTPC_ERROR_TOO_LESS_RAM) {
    Serial.println(F("HTTPC_ERROR_TOO_LESS_RAM"));
  } else if (httpCode == HTTPC_ERROR_ENCODING) {
    Serial.println(F("HTTPC_ERROR_ENCODING"));
  } else if (httpCode == HTTPC_ERROR_STREAM_WRITE) {
    Serial.println(F("HTTPC_ERROR_STREAM_WRITE"));
  } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
    Serial.println(F("HTTPC_ERROR_READ_TIMEOUT"));
  }
}

void parseAndProcessReceivedData() {
  unsigned int index = 0;

  char str[20] = { 0 };
  const char *delim = "#";
  char *token = NULL;

  receivedData.toCharArray(str, receivedData.length());

  String graphName = "";
  String temperature = "";

  token = strtok(str, delim);
  graphName = token;
  while (token) {
    //Serial.println(index);
    //Serial.println(token);
    token = strtok(NULL, delim);

    if (token == NULL) {
      break;
    }

    if (token[0] == 't') {
      String temp = String(token);
      temperature = temp.substring(2);
    }

    index++;
  }

  sendWebRequest(graphName, temperature);
}

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize system");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  resetOledDisplay();
  initializeOledDisplay();
  showLogo();
  initializeLoRa();
  initializeWLAN();

  Serial.println("System is ready");

  //Only for testing
  //receivedData = "LSM2#t:25.37#h:53.62";
  //parseAndProcessReceivedData();
}

void loop() {

  buttonPressed = digitalRead(BUTTON_PIN);
  if (buttonPressed == 0) { //If pressed
    displayOn();
  } else {
    displayOff();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    displayClear();
    displaySmallText(0, 0, "Received packages");
    displayLargeText(0, 14, String(receiveCounter));
    displaySmallText(0, 44, "Failures: " + String(failureCounter));
    displayDraw();

    previousMillis = currentMillis;
  }

  if (loraDataAvailable) {
    loraDataAvailable = false;

    //Read data
    int readIteration = 0;
    while (LoRa.available()) {
      receivedData = LoRa.readString();
      readIteration++;
    }

    //Print packet infos
    Serial.print("Receive i:'");
    Serial.print(readIteration);
    Serial.print(", data:'");
    Serial.print(receivedData);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    parseAndProcessReceivedData();

  }
}
