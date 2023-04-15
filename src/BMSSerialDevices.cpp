#include "BMSserialDevices.h"

BMSSerialDevice::BMSSerialDevice(const char *portName, Stream &dev, unsigned long baudRate, uint8_t rx, uint8_t tx, bool invert)
{
    device = &dev;
    name = portName;
    baud = baudRate;
    rxPin = rx;
    txPin = tx;
    invertSignal = invert;
    waitingForResponse = false;
    device->setTimeout(250);
};

BMSSerialDevice &BMSSerialDevice::onMessage(THandlerFunction_Message fn)
{
    _message_callback = fn;
    return *this;
}

BMSSerialHW::BMSSerialHW(const char *portName, HardwareSerial &dev, unsigned long baudRate, uint8_t rx, uint8_t tx, bool invert)
    : BMSSerialDevice(portName, dev, baudRate, rx, tx, invert) { device = &dev; };

void BMSSerialHW::begin()
{
    device->begin(baud, SERIAL_8N1, rxPin, txPin, invertSignal);
}

void BMSSerialHW::end()
{
    device->end();
}

void BMSSerialDevice::begin()
{ /* https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable */
}

void BMSSerialDevice::clearWriteError()
{
    device->clearWriteError();
}

void BMSSerialDevice::flush()
{
    device->flush();
}

void BMSSerialDevice::end()
{ //
}

int BMSSerialDevice::read()
{
    // add byte to wifi buffer
    incomingByte = device->read();
    if (status.serialBytesSize > 2000)
        status.serialBytesSize = 0;
    status.serialBytes[status.serialBytesSize++] = incomingByte;
    return incomingByte;
}

size_t BMSSerialDevice::send(uint8_t mb[], size_t length)
{
    device->flush();
    if (device->availableForWrite())
    {
        uint8_t l = length;
        size_t r = device->write(mb, l);
        waitingForResponse = r == l; // wait for response if successful send
        // Serial.println(String("Written ") + length + " - " + index + " " + waitingForResponse);
        return r;
    }
    return 0;
}

size_t BMSSerialDevice::reqCells()
{
    lastCommandSentCell = status.currentMillis;
    uint8_t m[7] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
    return sendMessage(m, 7) > 0 ? 0x04 : 0;
}

size_t BMSSerialDevice::reqInfo()
{
    lastCommandSentInfo = status.currentMillis;
    uint8_t m[7] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
    uint8_t written = sendMessage(m, 7);
    if (written != 7)
        Serial.println(String("Error writing [") + index + "]...");

    Serial.println("sending 03...");
    return written == 7 ? 0x03 : 0;
}

size_t BMSSerialDevice::reqHardware()
{
    uint8_t m[7] = {0xDD, 0xA5, 0x05, 0x00, 0xFF, 0xFB, 0x77};
    return sendMessage(m, 7) > 0 ? 0x05 : 0;
}

size_t BMSSerialDevice::sendMessage(uint8_t *m, size_t length)
{
    bufferIdx = 0; // reset buffer index
    return send(m, length);
    // Serial.print("Sending ");
    // Serial.println(name);
}

const char *BMSSerialDevice::handle()
{
    // source from https://github.com/bres55/Smart-BMS-arduino-Reader/blob/master/smart_bms_reader_Mega_v11.ino

    // doesn't help when out of sync
    // if (!device->available() && waitingForResponse)
    // {
    //     //Serial.println(String("delaying ") + index);
    //     delay(10);
    // }

    if (device->available())
    {
        if (bufferIdx == bufferSize - 5) // keeps some free space in buffer
            bufferIdx = 0;
        read();

        buffer[bufferIdx++] = incomingByte;

        if (bufferIdx - 1 == 3)
        {
            msgLength = (buffer[bufferIdx - 1]); // The fourth byte holds the length of data, excluding last 3 bytes checksum etc
        }

        if (msgLength > 0 && bufferIdx >= msgLength + 7)
        {
            _message_callback(index, buffer[1], buffer, bufferIdx);
            // total message received, reset buffer index and collect new messsage
            bufferIdx = 0;
            waitingForResponse = false;
            return "1";
        }
    }
    return "";
}
