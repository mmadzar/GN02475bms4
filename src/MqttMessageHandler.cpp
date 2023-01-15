#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "start") == 0)
    status.monitorStarted = true;
  else if (strcmp(command, "stop") == 0)
    status.monitorStarted = false;
  else if (strcmp(command, "monitorStarted") == 0)
    status.monitorStarted = String(message).toInt();
  else if (strcmp(command, "serialInterval") == 0)
    intervals.serialInterval = String(message).toInt();
  else if (strcmp(command, "serialIntervalCell") == 0)
    intervals.serialIntervalCell = String(message).toInt();
  else if (strcmp(command, "fakeError") == 0)
    status.fakeError = String(message).toInt();
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{
}

void MqttMessageHandler::handle()
{
}