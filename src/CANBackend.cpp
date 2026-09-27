//
// Created by Brandon on 7/26/26.
//

#if RGB_ARDUINO_ESP32

#include "CANBackend.h"

#include <Util.h>

#include "Clock.h"
#include "Log.h"
#include "RgbSPI.h"

namespace rgb::car {

CANBackend::CANBackend(u8 pinNumber, ClockRate clockRate, BaudRate baudRate, bool listenOnly):
  mClockRate{clockRate}, mBaudRate{baudRate}, mPinNumber{pinNumber}, mNextProperty{0}, mListenOnly{listenOnly}, mConnected{false}
{
}

auto CANBackend::connect(Vehicle& vehicle) -> bool {
  if (mConnected) {
    return true;
  }

  mMcp2515.setConfigurationMode();

  SPI::Start();
  if (!mMcp2515.getIsInitialized()) {
    mMcp2515.setSpiPins(A0);
    mMcp2515.setClockFrequency(static_cast<uint32_t>(8E6));
    mMcp2515.setSpiFrequency(5e6);
    if (!mMcp2515.init(static_cast<uint32_t>(mBaudRate), true)) {
      ERROR("Failed to initialize MCP2515 with baud %lu. Code: %X", static_cast<unsigned long>(mBaudRate),
            mMcp2515.getLastMCPError());
      return false;
    }
    mMcp2515.changeBaudRate(125E3); // Prime the system for the next change (bug in MCP2515 lib)
    if (!mMcp2515.changeBaudRate(static_cast<uint32_t>(mBaudRate))) {
      return false;
    }
  }

  if (mListenOnly && mMcp2515.getOperationMode() != MCP2515OperationMode::LISTEN) {
    if (!mMcp2515.setListenOnlyMode()) {
      ERROR("Failed to set MCP2515 to listen-only mode");
      return false;
    }
  }

  mMcp2515.setMask(0, 0x7FF << 18);
  mMcp2515.setFilter(0, 0x7E8, false);
  mMcp2515.setFilter(1, 0x728, false);

  mMcp2515.setMask(1, 0x7FF << 18);
  mMcp2515.setFilter(2, 0x7E8, false);
  mMcp2515.setFilter(3, 0x728, false);
  mMcp2515.setFilter(4, 0x768, false);
  mMcp2515.setFilter(5, 0x768, false);

  mMcp2515.enableFilterMask(0);
  mMcp2515.enableFilterMask(1);

  mMcp2515.setNormalMode();
  mConnected = true;
  vehicle.setConnected(true);

  resetProperties();

  INFO("CAN Module Initialized");
  return true;
}

auto CANBackend::isConnected() const -> bool {
  return mConnected;
}

auto CANBackend::ToString(PropertyType type) -> const char* {
  switch (type) {
    case PropertyType::RPM: return "RPM";
    case PropertyType::SPEED: return "SPEED";
    case PropertyType::COOLANT_TEMP: return "COOLANT_TEMP";
    case PropertyType::FUEL_LEVEL: return "FUEL_LEVEL";
    case PropertyType::THROTTLE_POSITION: return "THROTTLE_POSITION";
    case PropertyType::GEAR_NUMBER: return "GEAR_NUMBER";
    case PropertyType::GEAR_POSITION: return "GEAR_POSITION";
    case PropertyType::BRAKE_APPLIED: return "BRAKE_APPLIED";
    case PropertyType::OVERDRIVE_ACTIVE: return "OVERDRIVE_ACTIVE";
    case PropertyType::TCS_ACTIVE: return "TCS_ACTIVE";
    case PropertyType::INFO_SWITCH_PRESSED: return "INFO_SWITCH_PRESSED";
    case PropertyType::SELECT_SWITCH_PRESSED: return "SELECT_SWITCH_PRESSED";
    case PropertyType::Count_: return "UNKNOWN";
  }
  return "UNKNOWN";
}

auto CANBackend::nextProperty() -> Property& {
  auto& property = mProperties[mNextProperty++];
  if (mNextProperty >= mProperties.size()) {
    mNextProperty = 0;
  }
  return property;
}

auto CANBackend::update(Vehicle& vehicle) -> void {
  if (!mConnected) {
    ERROR("Not Connected");
    return;
  }

  auto now = Clock::Now();

  if (auto& property = nextProperty(); now.timeSince(property.lastRequestedAt) > property.frequency) {
    if (auto buffer = mMcp2515.check4FreeTransmitBuffer(); buffer != INVALID_BUFFER) {
      requestPID(property, buffer, now);
    }
  }

  auto flags = mMcp2515.check4InterruptFlags() & BOTH_BUFFERS;
  while (flags) {
    if (flags & 0x01 && receive(0, mBuffer0Message, vehicle)) {
      // log(BUFFER_0, mBuffer0Message);
    }

    if (flags & 0x02 && receive(1, mBuffer1Message, vehicle)) {
      // log(BUFFER_1, mBuffer1Message);
    }

    flags = mMcp2515.check4InterruptFlags() & BOTH_BUFFERS;
  }
}

auto CANBackend::requestPID(Property& property, u8 buffer, Timestamp now) -> void {
  auto& [canId, pId, mode] = property.message;
  u8 data[8] = {};

  auto canIdToUse = canId;
  if (mode == 1) {
    canIdToUse = 0x7DF;
    data[LENGTH_BYTE_IDX] = 0x02;
    data[MODE_BYTE_IDX] = mode;
    data[2] = static_cast<u8>(pId);
  } else {
    data[LENGTH_BYTE_IDX] = 0x03;
    data[MODE_BYTE_IDX] = mode;
    data[2] = static_cast<u8>(pId >> 8);
    data[3] = static_cast<u8>(pId & 0xFF);
  }


  if (!mMcp2515.fillTransmitBuffer(buffer, canIdToUse, false, false, 8, data)) {
    ERROR("%s Message Fill Error: %X", ToString(property.type), mMcp2515.getLastMCPError());
    ++property.sendFailures;
    return;
  }

  if (!mMcp2515.sendMessage(buffer, property.priority)) {
    if (property.failedAttempt()) {
      ERROR("%s Message Send Error: %X", ToString(property.type), mMcp2515.getLastMCPError());
    }
    return;
  }

  property.messageSent(now);
}

auto CANBackend::receive(int buffer, ResponseMessage& response, Vehicle& vehicle) -> bool {
  if (!mMcp2515.getAllFromReceiveBuffer(buffer, response.canId, response.extended, response.rtr, response.dlc, response.data)) {
    return false;
  }

  auto mode = response.data[1];
  u16 pId;
  if (mode == 0x41) {
    pId = response.data[2];
  } else if (mode == 0x62) {
    pId = static_cast<u16>(response.data[2] << 8) | response.data[3];
  } else {
    INFO("Unknown Mode");
    log(buffer, response);
    return true;
  }

  for (auto& property : mProperties) {
    if (property.handles(response, pId)) {
      property.responseReceived(response, Clock::Now(), vehicle);
      return true;
    }
  }

  // INFO("Unknown Message was Received");
  // log(buffer, response);

  return true;
}

auto CANBackend::log(int buffer, const ResponseMessage& message) -> void {
  if (message.dlc == 0) {
    INFO("EMPTY MESSAGE from ID: %X", message.canId);
    return;
  }
  Serial.print("Buffer: ");
  Serial.print(buffer);
  Serial.print("  ID: 0x");
  if (!message.extended) {
    Serial.print(static_cast<uint16_t>(message.canId & 0xFFFF), HEX);
  } else {
    Serial.print(static_cast<uint16_t>(((message.canId >> 8) >> 8) & 0xFFFF), HEX);
    Serial.print(static_cast<uint16_t>(message.canId & 0xFFFF), HEX);
  }

  Serial.print("\tFrame: ");
  if (!message.extended) {
    Serial.print("Standard");
  } else {
    Serial.print("Extended");
  }

  if (!message.rtr) {
    Serial.print("\tDLC: ");
    Serial.print(message.dlc, DEC);

    Serial.print("\tData:");

    for (size_t i = 0; i < message.dlc; i++) {
      Serial.print(" 0x");
      Serial.print(message.data[i], HEX);
    }
    Serial.println();
  } else {
    Serial.println("\tRemote Transmission Request");
  }
}

auto CANBackend::resetProperties() -> void {
  auto now = Clock::Now();
  auto offset = Duration::Zero();
  for (auto& property : mProperties) {
    auto initialTime = now - property.frequency + offset;

    property.lastRequestedAt = initialTime;
    property.lastReceivedAt = initialTime;
    property.averageResponseTime = Duration::Zero();
    property.sentMessages = 0;
    property.droppedMessages = 0;
    property.sendFailures = 0;

    offset += Duration::Milliseconds(1);
  }
}

auto CANBackend::logInformation() const -> void {
  auto now = Clock::Now();

  Serial.printf(
    "%-22s | %6s | %6s | %14s | %8s | %8s | %9s | %8s | %10s\n",
    "Property", "CAN ID", "PID", "Avg Resp (ms)", "Sent", "Dropped", "Success %", "Failures", "Lag (ms)"
  );
  Serial.println("---------------------------------------------------------------------------------------------------------------------");
  for (const auto& property : mProperties) {
    auto successRate = property.sentMessages == 0
      ? 0.f
      : (100.f / property.sentMessages) * (property.sentMessages - property.droppedMessages);

    Serial.printf(
      "%-22s | %6X | %6X | %14llu | %8u | %8u | %8.1f%% | %8u | %10llu\n",
      ToString(property.type),
      property.message.canId,
      property.message.pId,
      property.averageResponseTime.asMilliseconds(),
      property.sentMessages,
      property.droppedMessages,
      successRate,
      property.sendFailures,
      now.timeSince(property.lastReceivedAt).asMilliseconds()
    );
  }
}

}

#endif
