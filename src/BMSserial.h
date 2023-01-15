#ifndef BMSSERIAL_H_
#define BMSSERIAL_H_

#include <Arduino.h>
#include "shared/base/Collector.h"
#include "shared/configtypes/configtypes.h"
#include "appconfig.h"
#include "status.h"
#include "shared/MqttPubSub.h"
#include "shared/Bytes2WiFi.h"
#include "BMSserialDevices.h"

class BMSserial
{
private:
    BMSSerialDevice *device;
    SerialSettings settings;
    Bytes2WiFi *b2w;
    Bytes2WiFi *b2wdebug;
    CollectorConfig *configsBms[6];
    CollectorConfig *configsCell[24];

public:
    BMSserial();
    void handle();
    void setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport, Bytes2WiFi &portDebug);
};

#endif /* BMSSERIAL_H_ */