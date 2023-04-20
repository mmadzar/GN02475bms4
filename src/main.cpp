#include "appconfig.h"
#include "status.h"
#include <Arduino.h>
#include "shared/WiFiOTA.h"
#include "MqttMessageHandler.h"
#include <ArduinoOTA.h>
#include <string.h>
#include "shared/MqttPubSub.h"
#include "shared/Bytes2WiFi.h"
#include "BMSserial.h"

Status status;
WiFiOTA wota;
MqttPubSub mqtt;
Bytes2WiFi bytesWiFi;
Bytes2WiFi debugWiFi;
Settings settings;

uint32_t lastBytesSent = 0; // millis when last packet is sent

BMSserial bms;

long loops = 0;
unsigned long lastLoopReport = 0;

bool firstRun = true;

long lastVacuumReadTime = 0;
int lastVacuumRead = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Serial started!");
  pinMode(settings.led, OUTPUT);

  // pinMode(settings.lspwr, OUTPUT);
  // digitalWrite(settings.lspwr, HIGH);

  wota.setupWiFi();
  wota.setupOTA();
  mqtt.setup();
  bytesWiFi.setup(23, true);
  debugWiFi.setup(24);
  bms.setup(mqtt, bytesWiFi, debugWiFi);
}

void loop()
{
  status.currentMillis = millis();
  if (status.currentMillis - lastLoopReport > 1000) // number of loops in 1 second - for performance measurement
  {
    lastLoopReport = status.currentMillis;
    // Serial.printf("Loops in a second: %u\n", loops);
    status.loops = loops;
    loops = 0;
  }
  loops++;

  wota.handleWiFi();
  wota.handleOTA();
  if (loops % 10 == 0) // check mqtt every 10th cycle
    mqtt.handle();

  bms.handle();

  bytesWiFi.handle();
  debugWiFi.handle();

  mqtt.publishStatus(!firstRun);
  firstRun = false;
}
