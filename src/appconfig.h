#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475bms3" // same code for all bms instances 1, 2, 3, 4

#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct SerialConfig
{
  const char *portName;
  const unsigned long baudRate;
  const uint8_t rx;
  const uint8_t tx;
  const bool invert;
};

struct SerialSettings
{
  const SerialConfig device = {"hs1", 9600, 19, 18, false};
};

struct Settings
{
#define ListenChannelsCount 0
  const char *listenChannels[ListenChannelsCount] = {};

  const gpio_num_t led = (gpio_num_t)2; // status led
  const gpio_num_t lspwr = (gpio_num_t)21; //logic converter board power pin

  CollectorConfig colBms[6] = {
      {"ProtStatus", 0}, // protection status - error if >0
      {"Temp1", 0},
      {"Temp2", 0},
      {"Temp3", 0},
      {"Temp4", 0}, 
      {"Remaining", 0}};
  CollectorConfig colBmsCell[24] = {
      {"Cell01", 0},
      {"Cell02", 0},
      {"Cell03", 0},
      {"Cell04", 0},
      {"Cell05", 0},
      {"Cell06", 0},
      {"Cell07", 0},
      {"Cell08", 0},
      {"Cell09", 0},
      {"Cell10", 0},
      {"Cell11", 0},
      {"Cell12", 0},
      {"Cell13", 0},
      {"Cell14", 0},
      {"Cell15", 0},
      {"Cell16", 0},
      {"Cell17", 0},
      {"Cell18", 0},
      {"Cell19", 0},
      {"Cell20", 0},
      {"Cell21", 0},
      {"Cell22", 0},
      {"Cell23", 0},
      {"Cell24", 0}};

#define CollectorCount 0
  CollectorConfig collectors[CollectorCount] = {};

#define SwitchCount 0
  SwitchConfig switches[SwitchCount] = {};

  int getSwitchIndex(devicet device)
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (switches[i].device == device)
        return i;
    }
  }
};

struct Intervals
{
  int statusPublish = 1000;      // interval at which status is published to MQTT
  int click_onceDelay = 1000;    // milliseconds
  int serialInterval = 2000;      // milliseconds - goes fom 500 when battery in use to 30 000 when not in use, configured over MQTT interval command
  int serialIntervalCell = 5000; // milliseconds - goes fom 5000 when battery in use to 30 000 when not in use, configured over MQTT intervalCell command
};

extern Settings settings;
extern Intervals intervals;

#endif /* APPCONFIG_H_ */
