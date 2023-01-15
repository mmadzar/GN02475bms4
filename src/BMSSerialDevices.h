#ifndef BMSSERIALDEVICES_H_
#define BMSSERIALDEVICES_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "appconfig.h"
#include "status.h"

// compiler requires base classes at top of the file
class BMSSerialDevice
{
public:
    SerialSettings settings;

    // callback event related
    typedef void (*THandlerFunction_Message)(uint8_t index, uint8_t command, uint8_t *message, size_t length);
    // This callback will be called when full message from serial device is received
    BMSSerialDevice &onMessage(THandlerFunction_Message fn);

    BMSSerialDevice(const char *portName, Stream &dev, unsigned long baudRate, uint8_t rx, uint8_t tx, bool invert);
    bool waitingForResponse;
    const char *name;
    int8_t index;
    virtual void begin();
    virtual void end();
    size_t send(uint8_t msgbuffer[], size_t length);
    int read();
    void clearWriteError();
    void flush();
    size_t reqCells();
    size_t reqInfo();
    size_t reqHardware();
    size_t sendMessage(uint8_t *m, size_t length);
    const char *handle();
    uint16_t bufferIdx = 0;
    uint8_t buffer[1024];
    int lastCommandSentInfo = 0;          // ms when last command was sent for info request
    int lastCommandSentCell = 0;          // ms when last command was sent for cells request

private:

protected:
    Stream *device;
    uint16_t bufferSize = 1024;
    int incomingByte = 0;
    int msgLength = 0;
    bool messageReceived = false;
    THandlerFunction_Message _message_callback;

    // params from contructor
    unsigned long baud = 115200; // 9600;
    uint8_t config;
    uint8_t rxPin;
    uint8_t txPin;
    bool invertSignal;
    int numberOfCells = 24;
    int maxCurrentPerCell = 20000; // mA
    int nomVperCell = 3700;        // mV
    int minVperCell = 3000;        // mV
    int maxVperCell = 4200;        // mV
    int nomPackVoltage;
    int minPackVoltage;
    int maxPackVoltage;
    int capacityPack = 50000; // mAh
};

class BMSSerialHW : public BMSSerialDevice
{
public:
    BMSSerialHW(const char *portName, HardwareSerial &dev, unsigned long baudRate, uint8_t rx, uint8_t tx, bool invert);
    void begin();
    void end();

protected:
    HardwareSerial *device;
};

#endif