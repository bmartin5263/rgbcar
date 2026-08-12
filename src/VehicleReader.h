//
// Created by Brandon on 8/12/26.
//

#ifndef RGBLIB_VEHICLEREADER_H
#define RGBLIB_VEHICLEREADER_H

namespace rgb::car {
auto launchReader() -> void;

[[noreturn]]
auto vehicleReader2(void* args) -> void;
}

#endif //RGBLIB_VEHICLEREADER_H
