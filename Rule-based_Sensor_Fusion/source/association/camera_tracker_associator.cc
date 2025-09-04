#include "association/camera_tracker_associator.h"

#include "Eigen/Core"
#include "iou.h"
#include "logger.h"
#include "type_gate.h"
#include "util.h"

namespace tracker {

double CameraTrackerAssociator::CameraSimilarity(
    const Track& track,
    const CameraObservationProto& camera_observation) const {
  // TODO(vahid) add type gate here
  // EllipseSimilarity
  const double ellipse_sim = util::TwoEllipseSimilarity(
      track, camera_observation, *localization_service_);
  // Gate
  if (ellipse_sim < 0.0) {
    return 0.0;
  }
  // Sum similarity
  const double sim = ellipse_sim;
  return sim;
}

CameraTrackerAssociator::CameraTrackerAssociator(
    const LocalizationService* localization_service)
    : AssociatorBase(localization_service) {}

// TODO: Refactor this function, remove unnecessary code.
double CameraTrackerAssociator::ComputeSimilarity(
    const Track& track, const ObservationProto& observation) const {
  if (!(observation.has_camera_observation())) {
    Logger::GetInstance().Log(
        Logger::ERROR, "Error: non-camera observation in camera associator.");
  }
  return CameraSimilarity(track, observation.camera_observation());
}

}  // namespace tracker
