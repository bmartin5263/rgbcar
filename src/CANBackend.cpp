//
// Created by Brandon on 7/26/26.
//

#if RGB_ARDUINO_ESP32

#include "CANBackend.h"

#include "Clock.h"
#include "Log.h"
#include "RgbSPI.h"
#include "Vehicle.h"

namespace rgb::car {

CANBackend::CANBackend(u8 pinNumber, ClockRate clockRate, BaudRate baudRate, tcb::span<CANEcu> ecus, tcb::span<CANParameter> properties) :
  mEcus(ecus), mProperties(properties), mClockRate{clockRate}, mBaudRate{baudRate}, mPinNumber{pinNumber}, mNextParameter{0}, mConnected{false}
{
}

auto CANBackend::connect(Vehicle& vehicle) -> bool {
  if (mConnected) {
    return true;
  }

  mMcp2515.setConfigurationMode();

  SPI::Start();
  if (!mMcp2515.getIsInitialized()) {
    mMcp2515.setSpiPins(mPinNumber);
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

auto CANBackend::nextParameter() -> CANParameter& {
  auto& parameter = mProperties[mNextParameter++];
  if (mNextParameter >= mProperties.size()) {
    mNextParameter = 0;
  }
  return parameter;
}

auto CANBackend::ecuGate(u16 canId) -> CANEcu& {
  for (auto& gate : mEcus) {
    if (gate.canId == canId) {
      return gate;
    }
  }
  return mDefaultEcu;
}

auto CANBackend::update(Vehicle& vehicle) -> void {
  if (!mConnected) {
    ERROR("Not Connected");
    return;
  }

  auto now = Clock::Now();

  if (auto& parameter = nextParameter(); now.timeSince(parameter.metrics.lastRequestedAt) > parameter.frequency) {
    if (auto& ecu = ecuGate(parameter.message.canId); ecu.isAvailable(now)) {
      if (auto buffer = mMcp2515.check4FreeTransmitBuffer(); buffer != INVALID_BUFFER) {
        if (requestPID(parameter, buffer, now)) {
          ecu.requestedAt = now;
          ecu.awaitingResponse = true;
        }
      }
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

auto CANBackend::requestPID(CANParameter& parameter, u8 buffer, Timestamp now) -> bool {
  auto& [canId, pId, mode] = parameter.message;
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
    ERROR("%X Message Fill Error: %X", parameter.message.pId, mMcp2515.getLastMCPError());
    ++parameter.metrics.sendFailures;
    return false;
  }

  if (!mMcp2515.sendMessage(buffer, parameter.priority)) {
    if (parameter.failedAttempt()) {
      ERROR("%X Message Send Error: %X", parameter.message.pId, mMcp2515.getLastMCPError());
    }
    return false;
  }

  parameter.messageSent(now);
  return true;
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

  for (auto& parameter : mProperties) {
    if (parameter.handles(response, pId)) {
      parameter.responseReceived(response, Clock::Now(), vehicle);
      ecuGate(parameter.message.canId).awaitingResponse = false;
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

auto CANBackend::resetProperties() const -> void {
  auto now = Clock::Now();
  auto offset = Duration::Zero();
  for (auto& parameter : mProperties) {
    auto initialTime = now - parameter.frequency + offset;
    parameter.reset(initialTime);
    offset += Duration::Milliseconds(1);
  }

  for (auto& gate : mEcus) {
    gate.reset();
  }
}

auto CANBackend::logInformation() const -> void {
  auto now = Clock::Now();

  Serial.printf(
    "%6s | %6s | %10s | %14s | %8s | %8s | %9s | %8s | %10s\n",
    "CAN ID", "PID", "Freq (ms)", "Avg Resp (ms)", "Sent", "Dropped", "Success %", "Failures", "Lag (ms)"
  );
  Serial.println("---------------------------------------------------------------------------------------------------------------------");
  for (const auto& parameter : mProperties) {
    auto successRate = parameter.metrics.sentMessages == 0
      ? 0.f
      : (100.f / parameter.metrics.sentMessages) * (parameter.metrics.sentMessages - parameter.metrics.droppedMessages);

    Serial.printf(
      "%6X | %6X | %10llu | %14llu | %8u | %8u | %8.1f%% | %8u | %10llu\n",
      parameter.message.canId,
      parameter.message.pId,
      parameter.frequency.asMilliseconds(),
      parameter.metrics.averageResponseTime.asMilliseconds(),
      parameter.metrics.sentMessages,
      parameter.metrics.droppedMessages,
      successRate,
      parameter.metrics.sendFailures,
      now.timeSince(parameter.metrics.lastReceivedAt).asMilliseconds()
    );
  }
}

}

#endif
