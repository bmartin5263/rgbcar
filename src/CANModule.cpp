//
// Created by Brandon on 7/26/26.
//

#include "CANModule.h"

#include "Clock.h"
#include "Log.h"
#include "RgbSPI.h"

namespace rgb::car {

CANModule::CANModule(u8 pinNumber, ClockRate clockRate) {
  mcp2515.setSpiPins(pinNumber);
  mcp2515.setClockFrequency(static_cast<uint32_t>(clockRate));
  mcp2515.setSpiFrequency(5e6);
}

auto CANModule::connect(BaudRate baudRate, bool listenOnly) -> bool {
  SPI::Start();
  if (!mcp2515.getIsInitialized()) {
    if (!mcp2515.init(static_cast<uint32_t>(baudRate), false)) {
      ERROR("Failed to initialize MCP2515 with baud %lu. Code: %X", static_cast<unsigned long>(baudRate),
            mcp2515.getLastMCPError());
      return false;
    }
  }

  if (listenOnly && mcp2515.getOperationMode() != MCP2515OperationMode::LISTEN) {
    if (!mcp2515.setListenOnlyMode()) {
      ERROR("Failed to set MCP2515 to listen-only mode");
      return false;
    }
  }

  mConnected = true;
  return true;
}

auto CANModule::isConnected() const -> bool {
  return mConnected;
}

auto CANModule::update() -> void {
  if (!isConnected()) {
    return;
  }

  // static auto lastRequest = Timestamp::Zero();
  // if (Clock::Now().timeSince(lastRequest) > Duration::Seconds(1)) {
  //   request();
  //   lastRequest = Clock::Now();
  // }

  auto flags = mcp2515.check4InterruptFlags() & BOTH_BUFFERS;
  if (!flags) {
    // INFO("NO MESSAGES");
    return;
  }
  else {

  }

  if (flags & BUFFER_1) {
    auto buffer1Message = MCP2515Message{};
    if (receive(0, buffer1Message)) {
      log(BUFFER_1, buffer1Message);
    }
    else {
      INFO("MESSAGES: %i, 1=%i, 2=%i", flags, flags & BUFFER_1, flags & BUFFER_2);
      INFO("Receive Buffer 1 Error: %X", mcp2515.getLastMCPError());
    }
  }

  if (flags & BUFFER_2) {
    auto buffer2Message = MCP2515Message{};
    if (receive(1, buffer2Message)) {
      log(BUFFER_2, buffer2Message);
    }
    else {
      INFO("MESSAGES: %i, 1=%i, 2=%i", flags, flags & BUFFER_1, flags & BUFFER_2);
      INFO("Receive Buffer 2 Error: %X", mcp2515.getLastMCPError());
    }
  }
}

// Definition of a CAN-Message
struct CanMessage{
  u32 id;
  bool extended;
  u8 dlc;
  u8 data[8];
};

// Definition of Message_2
CanMessage message = {
  0x7DF,
  false,
  8,
  {0x02, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static bool sendSuccess = false;

auto CANModule::request() -> void {
  if (sendSuccess) {
    return;
  }
  auto buffer = mcp2515.check4FreeTransmitBuffer();
  if (buffer != 0xFF) {
    if (!mcp2515.fillTransmitBuffer(buffer, message.id, message.extended, false, message.dlc, message.data)) {
      INFO("Message_1 fill-Error: %X", mcp2515.getLastMCPError());
      return;
    }
    if (!mcp2515.sendMessage(buffer, 0)){
      INFO("Message_1 send-Error: %X", mcp2515.getLastMCPError());
    } else {
      sendSuccess = true;
      INFO("Message_1 send.");
    }
  }
}

auto CANModule::receive(int buffer, MCP2515Message& message) -> bool {
  return mcp2515.getAllFromReceiveBuffer(buffer, message.id, message.extended, message.rtr, message.dlc, message.data);
}

auto CANModule::log(int buffer, const MCP2515Message& message) -> void {
  if (message.dlc == 0) {
    return;
  }
  Serial.print("Buffer: ");
  Serial.print(buffer);
  Serial.print("  ID: 0x");
  if (!message.extended) {
    Serial.print((uint16_t) (message.id & 0xFFFF), HEX);
  } else {
    Serial.print((uint16_t) (((message.id >> 8) >> 8) & 0xFFFF), HEX);
    Serial.print((uint16_t) (message.id & 0xFFFF), HEX);
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
}
