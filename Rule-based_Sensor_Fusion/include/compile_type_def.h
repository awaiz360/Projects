#pragma once

#include "const_def.h"
#include "external_types.h"

namespace tracker {

template <TrackerEngineType EngineType>
struct TrackerTypes;

template <>
struct TrackerTypes<TrackerEngineType::TET_FUSION> {
  using LidarInputType = perception::vector_LiDARPerception2Fusion;
  using CameraInputType = perception::vector_CameraPerception2Fusion;
  using OutputType = dpc::TrafficParticipantStruct;
};

template <>
struct TrackerTypes<TrackerEngineType::TET_LIDAR> {
  using LidarInputType = std::vector<LidarPerceptionOutput>;
  using CameraInputType = std::vector<InstanceData>;
  using OutputType = std::vector<perception::LiDARPerception2Fusion>;
};

template <>
struct TrackerTypes<TrackerEngineType::TET_CAMERA> {
  using LidarInputType = std::vector<LidarPerceptionOutput>;
  using CameraInputType = std::vector<InstanceData>;
  using OutputType = std::vector<perception::CameraPerception2Fusion>;
};

}  // namespace tracker

using TrackerLidarInputType =
    typename tracker::TrackerTypes<tracker::kEngineType>::LidarInputType;
using TrackerCameraInputType =
    typename tracker::TrackerTypes<tracker::kEngineType>::CameraInputType;
using TrackerOutputType =
    typename tracker::TrackerTypes<tracker::kEngineType>::OutputType;
