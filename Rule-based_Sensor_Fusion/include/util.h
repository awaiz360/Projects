#ifndef TRACKER_UTIL_H_
#define TRACKER_UTIL_H_

#include "localization_service.h"
#include "track.h"

namespace tracker::util {

const TrackState* FindLatestCheckPointBeforeTime(const Track& track,
                                                 double timestamp);

double TwoEllipseSimilarity(const Track& track,
                            const CameraObservationProto& camera_observation,
                            const LocalizationService& localization_service);

}  // namespace tracker::util

#endif  // TRACKER_UTIL_H_
