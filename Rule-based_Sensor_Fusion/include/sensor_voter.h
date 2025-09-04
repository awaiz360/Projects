#ifndef TRACKER_SENSOR_VOTER_H
#define TRACKER_SENSOR_VOTER_H

#include <iterator>
#include <vector>

#include "const_def.h"
#include "track.h"
#include "types.h"

namespace tracker {

class SensorVoter {
 public:
  ObservationSensorType Vote(const Track& track,
                             const ObservationSensorType current_sensor_type);

 private:
  int findOnBitPosition(int number);

  double kDuration_ = 0.5;
};

}  // namespace tracker

#endif  // TRACKER_SENSOR_VOTER_H
