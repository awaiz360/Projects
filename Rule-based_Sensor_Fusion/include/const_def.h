#ifndef LIDAR_TRACKER_CONST_DEF_H_
#define LIDAR_TRACKER_CONST_DEF_H_

#include "types.h"

namespace tracker {

constexpr TrackerEngineType kEngineType = TrackerEngineType::TET_FUSION;

/********************** Wrapper Constants **********************/
// Linit the output objects in cost of delayed detection (just the first
// encounter)
constexpr int kMinimumlifetime = 5;
constexpr size_t kSlamDataBufferSize = 50;

// ObservationType number of items
constexpr int kNumberOfItemsInObservationType = 5;
constexpr int kNumberOfOneTypeItemsInObservationSensorType = 4;

/*********** Tracker Assiciator and Updater ***********/
constexpr double kRollbackThreshold = 0.5;
constexpr double kMaxObservationGroupHistoryDuration = 1.0;

/*********** Tracker Save and Remove ***********/
constexpr double kMaxCheckpointDuration = 1.0;

}  // namespace tracker
#endif  // LIDAR_TRACKER_CONST_DEF_H_
