//
// Created by Brandon on 3/29/26.
//

#include "VehicleLogger.h"
#include "Log.h"
#include "Clock.h"
#include "RgbSPI.h"

namespace rgb::car {

auto VehicleLogger::start() -> bool {
  if (started) {
    return true;
  }

  if (!SPI::Start()) {
    ERROR("SPI failed to start");
    return false;
  }

  if (!SD.begin(SPI::CSPin().value)) {
    ERROR("SD card initialization failed");
    return false;
  }

  auto fileNum = openNextFile();
  if (!fileNum) {
    ERROR("Could not create log file");
    return false;
  }

  INFO("SD card vehicle logging initialized with file index '%u'", fileNum);
  mLastFlush = Clock::Now();
  started = true;

  return true;
}

auto VehicleLogger::isStarted() -> bool {
  return started;
}

auto VehicleLogger::record(const VehicleData& data) -> void {
  if (!mFile) {
    return;
  }

  if (mCount < BUFFER_CAPACITY) {
    mBuffer[mCount++] = VehicleLogEntry{
      .timestamp = Clock::Now(),
      .data = data,
    };
  }
  else {
    ERROR("Cannot record vehicle log, at max capacity");
  }

  if (shouldFlush()) {
    flush();
  }
}

auto VehicleLogger::flush() -> void {
  if (!mFile || mCount == 0) {
    return;
  }

  INFO("Flushing %i entries", mCount);

  auto dataBuffer = reinterpret_cast<const uint8_t*>(mBuffer.data());
  auto dataSize = sizeof(VehicleLogEntry) * mCount;

  mFile.write(dataBuffer, dataSize);
  mFile.flush();

  mCount = 0;
  mLastFlush = Clock::Now();
}

auto VehicleLogger::stop() -> void {
  flush();
  if (mFile) {
    mFile.close();
  }
  started = false;
}

auto VehicleLogger::shouldFlush() const -> bool {
  return mCount >= BUFFER_CAPACITY
    || Clock::Now().timeSince(mLastFlush) >= FLUSH_INTERVAL;
}

auto VehicleLogger::openNextFile() -> uint {
  char filename[32];
  auto mFileIndex = 1;
  while (mFileIndex < MAX_FILE_INDEX) {
    snprintf(filename, sizeof(filename), FILENAME_FORMAT, mFileIndex);
    if (!SD.exists(filename)) {
      mFile = SD.open(filename, FILE_WRITE);
      if (mFile) {
        return mFileIndex;
      }
      ERROR("Failed to open %s", filename);
      return 0;
    }
    ++mFileIndex;
  }
  ERROR("No available log file slots");
  return 0;
}

}
