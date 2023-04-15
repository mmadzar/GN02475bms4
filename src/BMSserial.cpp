#include "BMSserial.h"

// Serial 0 reserved and not used
HardwareSerial Serial_one(1);
MqttPubSub *mqttClientBMS;
Settings settingsBmsCollectors;
Intervals intervals;
WiFiSettings wifiSettings;

long executed[2]; // number of executions send, receive in a second

BMSserial::BMSserial()
{
}

void getTimestamp(char *buffer)
{
    if (strcmp(status.SSID, "") == 0 || !getLocalTime(&(status.timeinfo), 10))
        sprintf(buffer, "INVALID TIME               ");
    else
    {
        long microsec = 0;
        struct timeval tv;
        gettimeofday(&tv, NULL);

        microsec = tv.tv_usec;
        strftime(buffer, 29, "%Y-%m-%d %H:%M:%S", &(status.timeinfo));
        sprintf(buffer, "%s.%06d", buffer, microsec);
    }
}

void BMSserial::setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport, Bytes2WiFi &portDebug)
{
    mqttClientBMS = &mqtt_client;
    b2w = &wifiport;
    b2wdebug = &portDebug;

    for (size_t i = 0; i < 6; i++)
    {
        CollectorConfig *sc = &settingsBmsCollectors.colBms[i];
        configsBms[i] = new CollectorConfig(sc->name, sc->sendRate);
        status.colBms[i] = new Collector(*configsBms[i]);
        status.colBms[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                                   { mqttClientBMS->sendMessageToTopic(String("{\"value\": ") + String(value) + ", \"min\": " + min + ", \"max\": " + max + ", \"samples\": " + samplesCollected +
                                                                           ", \"timestamp\": \"" + timestamp + "\"}",
                                                                       String(wifiSettings.hostname) + "/out/collectors/" + name); });
        status.colBms[i]->setup();
    }

    for (size_t i = 0; i < 24; i++)
    {
        CollectorConfig *sc = &settingsBmsCollectors.colBmsCell[i];
        configsCell[i] = new CollectorConfig(sc->name, sc->sendRate);
        status.colBmsCell[i] = new Collector(*configsCell[i]);
        status.colBmsCell[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                                       { mqttClientBMS->sendMessageToTopic(String("{\"value\": ") + String(value) + ", \"min\": " + min + ", \"max\": " + max + ", \"samples\": " + samplesCollected +
                                                                               ", \"timestamp\": \"" + timestamp + "\"}",
                                                                           String(wifiSettings.hostname) + "/out/collectors/" + name); });
        status.colBmsCell[i]->setup();
    }

    const SerialConfig *ss;
    ss = &settings.device;
    device = new BMSSerialHW(ss->portName, Serial_one, ss->baudRate, ss->rx, ss->tx, ss->invert); // 28 msg/s on 9600, 338 on 115200

    device->begin();
    device->onMessage([](uint8_t deviceIndex, uint8_t command, uint8_t *message, size_t length)
                      {
                                executed[1]++;
                                switch (command)
                                {
                                case 0x03:
                                {
                                    uint64_t ts = status.getTimestampMicro();

                                    status.colBms[0]->handle ((int)((message[20] * 256.0) + (message[21])), ts); // protection status

                                    //8, 9 - remaining
                                    //10, 11 - full
                                    // pack info
                                    int remaining = (message[8] * 256.0) + (message[9]); // remaining capacity
                                    int full = (message[10] * 256.0) + (message[11]); // full capacity
                                    status.colBms[1]->handle ((int)(remaining / full * 100), ts);

                                    status.colBms[2]->handle((int)((((message[27] * 256.0) + message[28])) - 273.15), ts); // Celsius degrees*10
                                    status.colBms[3]->handle((int)((((message[29] * 256.0) + message[30])) - 273.15), ts); 
                                    status.colBms[4]->handle((int)((((message[31] * 256.0) + message[32])) - 273.15), ts); 
                                    status.colBms[5]->handle((int)((((message[33] * 256.0) + message[34])) - 273.15), ts); 
                                }
                                break;
                                case 0x04:
                                {
                                    uint64_t ts = status.getTimestampMicro();
                                    // cell voltages
                                    for (size_t i = 0; i < 24; i++)
                                    {
                                        status.colBmsCell[i]->handle((int)((message[(i*2) + 4] * 256.0) + (message[(i*2) + 1 + 4])), ts);
                                    }
                                }
                                break;
                                case 0x05:
                                {
                                }
                                break;
                                default:
                                {
                                }
                                break;
                                } });
}

long lastReport = 0;
void BMSserial::handle()
{
    bool reqSent = 0;
    if (!status.monitorStarted)
        return;
    if (device->waitingForResponse)
        device->handle();

    // report performance
    if (status.currentMillis - lastReport > 1000)
    {
        lastReport = status.currentMillis;
        // Serial.println(String("executed ") + executed[0] + ", " + executed[1] + " " + device->waitingForResponse + " " + device->lastCommandSent + " " + status.currentMillis);
        executed[0] = 0;
        executed[1] = 0;
    }

    // cell check status interval is usually longer so checking it first
    if (!device->waitingForResponse && (device->lastCommandSentCell == 0 || status.currentMillis - device->lastCommandSentCell >= intervals.serialIntervalCell)) // request interval ms (3 ms doesn't affect total number of request on baud 9600 (28 per sec - 331 on 115200 baud) but keeps queries devices less busy)
    {
        executed[0]++;
        device->reqCells();
        reqSent = 1;
    }
    else if (!device->waitingForResponse && (device->lastCommandSentInfo == 0 || status.currentMillis - device->lastCommandSentInfo >= intervals.serialInterval))
    {
        executed[0]++;
        device->reqInfo();
        reqSent = 2;
    }

    // retry code with timeout
    //   if (device->waitingForResponse && status.currentMillis - device->lastCommandSent > 250) // cancel waiting if there is no response for 250 ms
    //   {
    //       // resend because there is no response
    //       device->flush();
    //       device->clearWriteError();
    //       device->lastCommandSent = status.currentMillis + 500; // give time to recover - restarting (end->begin) doesn't help
    //       device->waitingForResponse = false;

    //     Serial.print("Reset wait...");
    //     for (size_t k = 0; k < device->bufferIdx; k++)
    //     {
    //         Serial.print(device->buffer[k], 16);
    //         Serial.print(" ");
    //     }
    //     Serial.println(".");
    // }

    // handle again in case there is immediate response - good at higher speeds
    if (device->waitingForResponse)
        device->handle();

    // handle collectors
    if (strcmp(status.SSID, "") && reqSent > 0)
    {
        if (reqSent == 1)
            for (size_t i = 0; i < 24; i++)
                status.colBmsCell[i]->handle();

        if (reqSent == 2)
            for (size_t i = 0; i < 6; i++)
                status.colBms[i]->handle();
    }
}
