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
    // bufferIdx = 0;
    // buffer[bufferIdx++] = 0xDD;
    // buffer[bufferIdx++] = 0x04;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x3C;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x42;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xDB;
    // buffer[bufferIdx++] = 0x0A;
    // buffer[bufferIdx++] = 0x46;
    // buffer[bufferIdx++] = 0x0C;
    // buffer[bufferIdx++] = 0x2A;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0xB2;
    // buffer[bufferIdx++] = 0x0C;
    // buffer[bufferIdx++] = 0x3D;
    // buffer[bufferIdx++] = 0x0C;
    // buffer[bufferIdx++] = 0x3F;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x2E;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x23;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x3D;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x38;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x3D;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x2F;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x4D;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x44;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x7D;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x7E;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x85;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x87;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x81;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x91;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xD1;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x6F;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0x9D;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xA7;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xA8;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xA9;
    // buffer[bufferIdx++] = 0x0D;
    // buffer[bufferIdx++] = 0xA5;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0xE9;
    // buffer[bufferIdx++] = 0xF1;
    // buffer[bufferIdx++] = 0x8C;
    // buffer[bufferIdx++] = 0x77;
    // waitingForResponse = true;
    // return 0x04;
    uint8_t m[7] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};
    return sendMessage(m, 7) > 0 ? 0x04 : 0;
}

size_t BMSSerialDevice::reqInfo()
{
    lastCommandSentInfo = status.currentMillis;
    // bufferIdx = 0;
    // buffer[bufferIdx++] = 0xDD;
    // buffer[bufferIdx++] = 0x03;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x1F;
    // buffer[bufferIdx++] = 0x20;
    // buffer[bufferIdx++] = 0x01;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x0F;
    // buffer[bufferIdx++] = 0xA0;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x28;
    // buffer[bufferIdx++] = 0xEB;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = status.fakeError ? 0x0A : 0x00; // 0a error, 00 no error
    // buffer[bufferIdx++] = 0x24;
    // buffer[bufferIdx++] = 0x00;
    // buffer[bufferIdx++] = 0x01;
    // buffer[bufferIdx++] = 0x1E;
    // buffer[bufferIdx++] = 0x04;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0x40;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0x43;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0x3E;
    // buffer[bufferIdx++] = 0x0B;
    // buffer[bufferIdx++] = 0x3E;
    // buffer[bufferIdx++] = 0xFC;
    // buffer[bufferIdx++] = 0x82;
    // buffer[bufferIdx++] = 0x77;
    // waitingForResponse = true;
    // return 0x03;
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

        // acccept only good bytes
        if ((bufferIdx == 0 && incomingByte == 0xDD) || (bufferIdx == 1 && incomingByte == 0xA5) || bufferIdx > 1)
        {
            buffer[bufferIdx++] = incomingByte;

            if (bufferIdx - 1 == 3)
            {
                msgLength = (buffer[bufferIdx - 1]); // The fourth byte holds the length of data, excluding last 3 bytes checksum etc
            }

            // if (bufferIdx > 0)
            // {
            //     Serial.print(name);
            //     Serial.print(" -> ");
            //     Serial.print(bufferIdx - 1);
            //     Serial.print(" -> ");
            //     Serial.print(msgLength);
            //     Serial.print(" -> ");
            //     Serial.println(String(buffer[bufferIdx - 1], HEX));
            // }
        }
        else
        {
            // skip invalid message
            // reset buffer
            // Serial.print("r");
            // Serial.println(String("Reseting buffer ") + index + "..->" + String(incomingByte, 16) + "<-");
            bufferIdx = 0;
        }
        // Serial.print(bufferIdx);
        // Serial.print(" ... ");
        // Serial.println(msgLength);

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
