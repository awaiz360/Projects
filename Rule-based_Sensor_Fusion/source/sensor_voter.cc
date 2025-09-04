#include "sensor_voter.h"

#include <iostream>

#include "logger.h"

namespace tracker {

ObservationSensorType SensorVoter::Vote(
    const Track& track, const ObservationSensorType current_sensor_type) {
  int resultSensor = current_sensor_type;
  std::vector<bool> types(kNumberOfOneTypeItemsInObservationSensorType, false);
  types[findOnBitPosition(static_cast<int>(current_sensor_type))] = true;

  for (auto it = track.checkpoints.rbegin(); it != track.checkpoints.rend();
       ++it) {
    const auto& ckpt = *it;
    double dt = track.track_state.timestamp - ckpt.timestamp;
    if (dt < 0) {
      Logger::GetInstance().Log(
          Logger::WARNING,
          "Warning: The checkpoint time stamp inversion is found.");
      std::cout << "The checkpoint time stamp inversion is found." << std::endl;
      continue;
    }
    if (dt > kDuration_) {
      break;
    }
    if (ckpt.observation) {
      if (!types[findOnBitPosition(
              static_cast<int>(ckpt.observation->get_sensorType()))]) {
        types[findOnBitPosition(
            static_cast<int>(ckpt.observation->get_sensorType()))] = true;
        resultSensor |= (int)ckpt.observation->get_sensorType();
      }
    }
  }
  return static_cast<ObservationSensorType>(resultSensor);
}

int SensorVoter::findOnBitPosition(int number) {
  int position = 0;
  int onBitCount = 0;

  for (int i = 0; i < sizeof(int) * 8; ++i) {
    if (number & (1 << i)) {
      onBitCount++;
      if (onBitCount == 1) {
        position = i;
      } else if (onBitCount > 1) {
        Logger::GetInstance().Log(
            Logger::ERROR, "Error: Multiple sensor types in one observation.");
        std::cout << "Multiple sensor types in one observation." << std::endl;
        return 0;
      }
    }
  }
  return position;
}

}  // namespace tracker
