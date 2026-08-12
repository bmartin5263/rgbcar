//
// Created by Brandon on 3/29/26.
//

#ifndef RGBLIB_VEHICLEUPDATECODE_H
#define RGBLIB_VEHICLEUPDATECODE_H

namespace rgb::car {

enum VehicleUpdateCode : u32 {
  NONE = 0, PARTIAL = 1, FULL = 2
};

}

#endif //RGBLIB_VEHICLEUPDATECODE_H
