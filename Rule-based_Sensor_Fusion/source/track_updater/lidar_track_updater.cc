#include "track_updater/lidar_track_updater.h"

namespace tracker {

LidarTrackUpdater::LidarTrackUpdater(const TrackLifeManager* track_life_manager)
    : UpdaterBase(track_life_manager) {}

// TODO: Refactor this function, removing unnecessary code.
void LidarTrackUpdater::UpdateTrackFromObservation(
    const ObservationProto& observation, Track* track) {
  const double obs_timestamp = observation.timestamp();

  if (observation.has_lidar_observation()) {
    const Matrix3d measurement_noise =
        measurement_noise_.ComputeXYYawObservationNoise(observation);
    track->track_state.estimator.PredictAndUpdate(observation,
                                                  measurement_noise);
    // TODO(somebody) estimate shape.
    const Eigen::Vector2d pos = track->track_state.estimator.pos();
    const double yaw = track->track_state.estimator.yaw();
    track->track_state.sensor_type =
        sensor_voter_.Vote(*track, observation.get_sensorType());
    track->track_state.motion_type.updateMotionType(
        track->track_state.estimator.vx(), track->track_state.estimator.vy());
    track->track_state.bounding_box =
        Box3D({pos.x(), pos.y(), observation.lidar_observation().center_z()},
              yaw, 0.0, observation.lidar_observation().length(),
              observation.lidar_observation().width(),
              observation.lidar_observation().height());
    track->track_state.conf = ConfidenceCalculator(
        obs_timestamp, track->track_state.timestamp,
        observation.lidar_observation().conf(), track->track_state.conf);
    track->track_state.observation =
        std::make_shared<const ObservationProto>(observation);
    track->track_state.type = type_voter_.Vote(
        *track, track->track_state.observation->lidar_observation().type(),
        track->track_state.conf);

  } else {
    Logger::GetInstance().Log(Logger::ERROR,
                              "Error: Not lidar observation type.");
    std::cout << "Unknown observation type." << std::endl;
  }

  // TODO(somebody) update lifestate
  if (track_life_manager_->ShouldChangeToConfirmed(obs_timestamp, *track)) {
    track->track_state.life_state = TrackLifeState::TLS_CONFIRMED;
  }
  track->track_state.timestamp = obs_timestamp;
  track->track_state.lifetime++;
}

}  // namespace tracker
