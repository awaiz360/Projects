#ifndef TRACKER_TRACK_H_
#define TRACKER_TRACK_H_

#include <stdint.h>

#include <deque>
#include <memory>

#include "box2d.h"
#include "box3d.h"
#include "estimator_filter/estimator.h"
#include "motion_type_detection.h"
#include "types.h"

namespace tracker {

struct TrackState {
  uint32_t id;
  uint32_t lifetime;
  double timestamp;
  ObservationType type;
  ObservationSensorType sensor_type;
  double conf;
  Box3D bounding_box;
  TrackLifeState life_state = TrackLifeState::TLS_IDLE;
  Estimator estimator;
  MotionTypeDetection motion_type;
  std::shared_ptr<const ObservationProto> observation = nullptr;
};

struct Track {
  TrackState track_state;
  std::deque<TrackState> checkpoints;
};

using TrackPtr = std::shared_ptr<Track>;

}  // namespace tracker

#endif  // TRACKER_TRACK_H_
