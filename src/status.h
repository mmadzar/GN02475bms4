#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"
#include "shared/base/Collector.h"

struct Status : public StatusBase
{
  Collector *colBms[6];
  Collector *colBmsCell[24];

  String bmsCommand = "";
  int switches[SwitchCount]{};

  // BMS
  byte serialBytes[2048];
  int serialBytesSize;
  bool monitorStarted = false;
  bool fakeError = false;

#define BMScollectorCount 29 // 24 cells + 4 temps + protection_status

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

    // JsonObject jcollectors = root.createNestedObject("collectors");
    // for (size_t i = 0; i < CollectorCount; i++)
    //   jcollectors[settings.collectors[i].name] = collectors[i];

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
