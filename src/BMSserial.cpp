#include "BMSserial.h"

// Serial 0 reserved and not used
HardwareSerial Serial_one(1);
MqttPubSub *mqttClientBMS;
Settings settingsBmsCollectors;
Intervals intervals;
WiFiSettings wifiSettings;
Collector *colBms[6];
Collector *colBmsCell[24];
int msgLength = 0;

long executed[2]; // number of executions send, receive in a second

BMSserial::BMSserial()
{
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
        colBms[i] = new Collector(*configsBms[i]);
        colBms[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                            {
                             if(samplesCollected>0) 
                             {
                                status.colBms[settingsBmsCollectors.getColBmsIndex(name)] = value;
                                mqttClientBMS->sendMessageToTopic(String("{\"value\": ") + String(value) + ", \"min\": " + min + ", \"max\": " + max + ", \"samples\": " + samplesCollected +
                                                                    ", \"timestamp\": \"" + timestamp + "\"}",
                                                                String(wifiSettings.hostname) + "/out/collectors/" + name); 
                             } });
        colBms[i]->setup();
    }

    for (size_t i = 0; i < 24; i++)
    {
        CollectorConfig *sc = &settingsBmsCollectors.colBmsCell[i];
        configsCell[i] = new CollectorConfig(sc->name, sc->sendRate);
        colBmsCell[i] = new Collector(*configsCell[i]);
        colBmsCell[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                                { 
                                     if(samplesCollected>0) {
                                        status.colBmsCell[settingsBmsCollectors.getColBmsCellIndex(name)] = value;
                                        mqttClientBMS->sendMessageToTopic(String("{\"value\": ") + String(value) + ", \"min\": " + min + ", \"max\": " + max + ", \"samples\": " + samplesCollected +
                                                                        ", \"timestamp\": \"" + timestamp + "\"}",
                                                                    String(wifiSettings.hostname) + "/out/collectors/" + name);
                                } });
        colBmsCell[i]->setup();
    }

    const SerialConfig *ss;
    ss = &settings.device;
    device = new BMSSerialHW(ss->portName, Serial_one, ss->baudRate, ss->rx, ss->tx, ss->invert); // 28 msg/s on 9600, 338 on 115200

    device->begin();
    device->onMessage([](uint8_t deviceIndex, uint8_t command, uint8_t *message, size_t length)
                      {
                            msgLength=length;
                                executed[1]++;
                                switch (command)
                                {
                                case 0x03:
                                {
                                    uint64_t ts = status.getTimestampMicro();

                                    colBms[0]->handle ((int)((message[20] * 256.0) + (message[21])), ts); // protection status

                                    //8, 9 - remaining
                                    //10, 11 - full
                                    // pack info
                                    int remaining = (message[8] * 256.0) + (message[9]); // remaining capacity
                                    int full = (message[10] * 256.0) + (message[11]); // full capacity
                                    colBms[1]->handle ((int)(remaining / full * 100), ts);

                                    colBms[2]->handle((int)((((message[27] * 256.0) + message[28])) - 273.15), ts); // Celsius degrees*10
                                    colBms[3]->handle((int)((((message[29] * 256.0) + message[30])) - 273.15), ts); 
                                    colBms[4]->handle((int)((((message[31] * 256.0) + message[32])) - 273.15), ts); 
                                    colBms[5]->handle((int)((((message[33] * 256.0) + message[34])) - 273.15), ts); 
                                }
                                break;
                                case 0x04:
                                {
                                    uint64_t ts = status.getTimestampMicro();
                                    // cell voltages
                                    for (size_t i = 0; i < 24; i++)
                                    {
                                        colBmsCell[i]->handle((int)((message[(i*2) + 4] * 256.0) + (message[(i*2) + 1 + 4])), ts);
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

void BMSserial::handleDevice()
{
    device->handle();
    if (msgLength > 0)
    {
        b2wdebug->addBuffer('c');
        b2w->addBuffer((char *)device->buffer, msgLength);
        msgLength = 0; // message sent to WiFi port
    }
}

long lastReport = 0;
void BMSserial::handle()
{
    handleDevice();
    if (b2w->wifiCmdPos > 0)
    {
        b2wdebug->addBuffer('a');
        b2wdebug->addBuffer(device->send(b2w->wifiCommand, b2w->wifiCmdPos));
        b2wdebug->addBuffer('b');
        // device->send(b2w->wifiCommand, b2w->wifiCmdPos);
        b2w->wifiCmdPos = 0;
        handleDevice();
    }

    bool reqSent = 0;

    // report performance
    if (status.currentMillis - lastReport > 1000)
    {
        millis();
        lastReport = status.currentMillis;
    }

    if (!status.monitorStarted)
        return;
    // cell check status interval is usually longer so checking it first
    if ((device->lastCommandSentInfo == 0 || status.currentMillis - device->lastCommandSentInfo >= intervals.serialInterval))
    {
        executed[0]++;
        device->reqInfo();
        reqSent = 2;
        handleDevice();
    }

    if ((device->lastCommandSentCell == 0 || status.currentMillis - device->lastCommandSentCell >= intervals.serialIntervalCell)) // request interval ms (3 ms doesn't affect total number of request on baud 9600 (28 per sec - 331 on 115200 baud) but keeps queries devices less busy)
    {
        executed[0]++;
        device->reqCells();
        reqSent = 1;
        handleDevice();
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

    // // handle collectors
    // if (strcmp(status.SSID, "") && reqSent > 0)
    // {
    //     if (reqSent == 1)
    //         for (size_t i = 0; i < 24; i++)
    //             status.colBmsCell[i]->handle();

    //     if (reqSent == 2)
    //         for (size_t i = 0; i < 6; i++)
    //             status.colBms[i]->handle();
    // }
}
