#ifndef TRACKER_TYPE_GATE_H
#define TRACKER_TYPE_GATE_H

#include "logger.h"
#include "types.h"

namespace tracker {

class TypeGate {
 public:
  // Check if two entities can associate
  static bool CanAssociate(const Track& track,
                           const ObservationProto& observation) {
    if (!TypeGate::CheckType(track, observation)) {
      return false;
    }
    // You can add more gates here.
    return true;
  }

  static bool CheckType(const Track& track,
                        const ObservationProto& observation) {
    ObservationType type1 = track.track_state.type;
    ObservationType type2 = GetObservationType(observation);

    // Disallow specific combinations
    if ((type1 == OT_VEHICLE && type2 == OT_PEDESTRIAN) ||
        (type1 == OT_PEDESTRIAN && type2 == OT_VEHICLE) ||
        (type1 == OT_TRUCK && type2 == OT_PEDESTRIAN) ||
        (type1 == OT_PEDESTRIAN && type2 == OT_TRUCK) ||
        (type1 == OT_TRUCK && type2 == OT_CYCLIST) ||
        (type1 == OT_CYCLIST && type2 == OT_TRUCK)) {
      return false;
    }
    // Allow all other combinations
    return true;
  }

 private:
  static ObservationType GetObservationType(
      const ObservationProto& observation) {
    ObservationType type;
    if (observation.has_lidar_observation()) {
      type = observation.lidar_observation().type();
    } else if (observation.has_camera_observation()) {
      type = observation.camera_observation().type();
    } else if (observation.has_radar_observation()) {
      type = observation.radar_observation().type();
    } else {
      Logger::GetInstance().Log(
          Logger::ERROR, "Error: Unknown observation type in type_gate.");
      std::cout << "Unknown observation type." << std::endl;
    }
    return type;
  }
};

}  // namespace tracker
#endif  // TRACKER_TYPE_GATE_H
