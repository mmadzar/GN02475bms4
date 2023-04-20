#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"
#include "shared/base/Collector.h"

struct Status : public StatusBase
{
  int colBms[6];
  int colBmsCell[24];

  String bmsCommand = "";
  int switches[SwitchCount]{};

  // BMS
  byte serialBytes[2048];
  int serialBytesSize;
  bool monitorStarted = false;
  bool fakeError = false;

  void initBMScollectors()
  {
  }

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();

    root["monitorStarted"] = monitorStarted;
    root["serialInterval"] = intervals.serialInterval;
    root["serialIntervalCell"] = intervals.serialIntervalCell;
    root["fakeError"] = fakeError;

    JsonObject jcollectors = root.createNestedObject("collectorsGeneral");
    for (size_t i = 0; i < 4; i++)
      jcollectors[settings.colBms[i].name] = colBms[i];

    JsonObject jcollectors1 = root.createNestedObject("collectorsCell");
    for (size_t i = 0; i < 24; i++)
      jcollectors1[settings.colBmsCell[i].name] = colBmsCell[i];

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
