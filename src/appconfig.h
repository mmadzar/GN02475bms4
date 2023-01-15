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
      {"ProtStatus", 500}, // protection status - error if >0
      {"Temp1", 1000},
      {"Temp2", 1000},
      {"Temp3", 1000},
      {"Temp4", 1000}, 
      {"Remaining", 1000}};
  CollectorConfig colBmsCell[24] = {
      {"Cell01", 5000},
      {"Cell02", 5000},
      {"Cell03", 5000},
      {"Cell04", 5000},
      {"Cell05", 5000},
      {"Cell06", 5000},
      {"Cell07", 5000},
      {"Cell08", 5000},
      {"Cell09", 5000},
      {"Cell10", 5000},
      {"Cell11", 5000},
      {"Cell12", 5000},
      {"Cell13", 5000},
      {"Cell14", 5000},
      {"Cell15", 5000},
      {"Cell16", 5000},
      {"Cell17", 5000},
      {"Cell18", 5000},
      {"Cell19", 5000},
      {"Cell20", 5000},
      {"Cell21", 5000},
      {"Cell22", 5000},
      {"Cell23", 5000},
      {"Cell24", 5000}};

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
