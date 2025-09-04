#include "tracker.h"

namespace tracker {

std::vector<const ObservationProto*> GetValidObservation(
    const ObservationsProto& observations) {
  std::vector<const ObservationProto*> valid_observations;
  valid_observations.reserve(observations.observations_size());
  for (const auto& obs : observations.observations()) {
    valid_observations.push_back(&obs);
  }
  return valid_observations;
}

Tracker::Tracker(const TrackerEngineType engine_type) {
  engine_type_ = engine_type;
  switch (engine_type_) {
    case TrackerEngineType::TET_FUSION:
      wrapper_ = std::make_unique<FusionWrapper>(kSlamDataBufferSize);
      track_updater_ =
          std::make_unique<FusionTrackUpdater>(&track_life_manager_);
      associator_ =
          std::make_unique<FusionTrackerAssociator>(&localization_service_);
      break;
    case TrackerEngineType::TET_CAMERA:
      wrapper_ = std::make_unique<CameraWrapper>(kSlamDataBufferSize);
      track_updater_ =
          std::make_unique<CameraTrackUpdater>(&track_life_manager_);
      associator_ =
          std::make_unique<CameraTrackerAssociator>(&localization_service_);
      break;
    case TrackerEngineType::TET_LIDAR:
      wrapper_ = std::make_unique<LidarWrapper>(kSlamDataBufferSize);
      track_updater_ =
          std::make_unique<LidarTrackUpdater>(&track_life_manager_);
      associator_ =
          std::make_unique<LidarTrackerAssociator>(&localization_service_);
      break;
    case TrackerEngineType::TET_RADAR:
    case TrackerEngineType::TET_UNKNOWN:
      Logger::GetInstance().Log(Logger::ERROR,
                                "Error: Unknown tracker engine type.");
      std::cout << "Unknown tracker engine type." << std::endl;
      break;
  }
}

bool Tracker::UpdatePose(const Eigen::Matrix4d& pose, double timestamp) {
  return localization_service_.PushBackPose(pose, timestamp);
}

std::unique_ptr<Track> Tracker::NewTrackFromObservation(
    const ObservationProto& observation) {
  auto track_ptr = std::make_unique<Track>();

  // If engine type is fusion, we only create track from lidar
  if ((engine_type_ == TET_LIDAR || engine_type_ == TET_FUSION) &&
      observation.has_lidar_observation()) {
    const auto& obs = observation.lidar_observation();
    track_ptr->track_state.id = NewTrackId();
    track_ptr->track_state.lifetime = 1;
    track_ptr->track_state.timestamp = obs.timestamp();
    track_ptr->track_state.type = obs.type();
    track_ptr->track_state.conf = obs.conf();
    track_ptr->track_state.sensor_type = observation.get_sensorType();
    track_ptr->track_state.bounding_box =
        Box3D({obs.center_x(), obs.center_y(), obs.center_z()}, obs.yaw(), 0.0,
              obs.length(), obs.width(), obs.height());
    track_ptr->track_state.life_state = TrackLifeState::TLS_INIT;
    track_ptr->track_state.estimator.Init(obs.center_x(), obs.center_y(), 0.0,
                                          obs.yaw(), 0.0, obs.timestamp());

  } else if ((engine_type_ == TET_CAMERA) &&
             observation.has_camera_observation()) {
    const auto& obs = observation.camera_observation();
    track_ptr->track_state.id = NewTrackId();
    track_ptr->track_state.lifetime = 1;
    track_ptr->track_state.timestamp = obs.timestamp();
    track_ptr->track_state.type = obs.type();
    track_ptr->track_state.conf = obs.conf();
    track_ptr->track_state.sensor_type = observation.get_sensorType();
    track_ptr->track_state.bounding_box =
        Box3D({obs.center_x(), obs.center_y(), obs.center_z()}, obs.yaw(), 0.0,
              obs.length(), obs.width(), obs.height());
    track_ptr->track_state.life_state = TrackLifeState::TLS_INIT;
    track_ptr->track_state.estimator.Init(obs.center_x(), obs.center_y(), 0.0,
                                          obs.yaw(), 0.0, obs.timestamp());

  } else if ((engine_type_ == TET_LIDAR || engine_type_ == TET_FUSION) &&
             observation.has_camera_observation()) {
    std::cout << "No camera observation new track for lidar or fusion engine."
              << std::endl;
  } else {
    Logger::GetInstance().Log(Logger::ERROR,
                              "Error: Unknown observation type.");
    std::cout << "Unknown observation type." << std::endl;
  }

  track_ptr->track_state.observation =
      std::make_shared<const ObservationProto>(observation);
  return track_ptr;
}

void Tracker::SaveCheckPoint() {
  for (auto track : tracks_) {
    // Check if checkpoints exist for this track
    if (track->checkpoints.empty()) {
      TrackState ckpt = track->track_state;
      track->checkpoints.push_back(std::move(ckpt));
      // Check if sensor type or timestamp has changed
    } else if ((track->track_state.observation->get_sensorType() !=
                track->checkpoints.back().observation->get_sensorType()) ||
               (track->track_state.timestamp !=
                track->checkpoints.back().timestamp)) {
      // Save a new checkpoint
      TrackState ckpt = track->track_state;
      track->checkpoints.push_back(std::move(ckpt));
      // Remove old checkpoints if the duration exceeds the limit
      while (!track->checkpoints.empty() &&
             track->checkpoints.back().timestamp -
                     track->checkpoints.front().timestamp >
                 kMaxCheckpointDuration) {
        track->checkpoints.pop_front();
      }
    }
  }
}

void Tracker::PruneObservationGroupHistory() {
  while (!observations_group_history_.empty() &&
         observations_group_history_.back()->min_timestamp() -
                 observations_group_history_.front()->min_timestamp() >
             kMaxObservationGroupHistoryDuration) {
    observations_group_history_.pop_front();  // Remove the oldest observation
  }
}

void Tracker::AssociateObservationsAndUpdateTracks(
    const std::vector<const ObservationProto*> valid_observations) {
  // Association
  const auto match_result = associator_->Associate(tracks_, valid_observations);

  // Update
  for (int i = 0; i < match_result.size(); ++i) {
    const int track_index = match_result[i];
    const auto* obs = valid_observations[i];

    if (track_index != -1) {
      // Update existing track
      track_updater_->UpdateTrackFromObservation(*obs,
                                                 tracks_[track_index].get());
    } else {
      // Create new track
      tracks_.push_back(NewTrackFromObservation(*obs));
    }
  }

  // Save checkpoint
  SaveCheckPoint();
}

void Tracker::Tracking(const ObservationsProto& observations) {
  const auto valid_observations = GetValidObservation(observations);

  if (!valid_observations.empty()) {
    bool should_roll_back = false;
    double current_observations_timestamp = observations.min_timestamp();

    if (!observations_group_history_.empty()) {
      double latest_observations_timestamp =
          observations_group_history_.back()->min_timestamp();

      // Check for rollback condition (with tolerance for slight
      // out-of-orderness)
      if (current_observations_timestamp < latest_observations_timestamp) {
        if (latest_observations_timestamp - current_observations_timestamp >
            kRollbackThreshold) {
          // Observation is too old - discard
          return;
        }
        if (valid_observations.at(0)
                ->has_lidar_observation()) {  // Rollback only for Lidar (for
                                              // now)
          should_roll_back = true;
        }
      }
    }

    if (!should_roll_back) {
      // Normal tracking flow
      observations_group_history_.push_back(
          std::make_shared<const ObservationsProto>(observations));

      // Do association and updating for the current frame
      AssociateObservationsAndUpdateTracks(valid_observations);

    } else {
      // Rollback scenario

      // 1. Find rollback point
      int start_index = 0;
      for (int i = 0; i < observations_group_history_.size(); i++) {
        start_index = i;
        if (current_observations_timestamp <=
            observations_group_history_[i]->min_timestamp()) {
          break;
        }
      }

      // 2. Update observations_group_history
      observations_group_history_.insert(
          observations_group_history_.begin() + start_index,
          std::make_shared<const ObservationsProto>(observations));

      // 3. Roll back track states
      double rollback_timestamp =
          observations_group_history_[start_index]->min_timestamp();
      std::unordered_set<uint32_t> delete_rollback_ids;
      for (auto& track : tracks_) {
        // Find the closest checkpoint before the rollback timestamp
        auto it = std::upper_bound(track->checkpoints.begin(),
                                   track->checkpoints.end(), rollback_timestamp,
                                   [](double ts, const TrackState& state) {
                                     return ts < state.timestamp;
                                   });
        if (it != track->checkpoints.begin()) {
          --it;  // Get the checkpoint just before the rollback timestamp
          track->track_state = *it;
          // Clear checkpoints and observations history after the rollback point
          track->checkpoints.erase(
              std::upper_bound(track->checkpoints.begin(),
                               track->checkpoints.end(), rollback_timestamp,
                               [](double ts, const TrackState& state) {
                                 return ts < state.timestamp;
                               }),
              track->checkpoints.end());
        } else {
          // delete the track
          delete_rollback_ids.insert(track->track_state.id);
        }
      }
      RemoveTracks(delete_rollback_ids);

      // 4. Re-association and re-updating
      for (int i = start_index; i < observations_group_history_.size(); i++) {
        // Perform association and update for each observation
        // from the rollback point to the current frame
        const auto valid_observations_in_rollback =
            GetValidObservation(*observations_group_history_[i]);
        AssociateObservationsAndUpdateTracks(valid_observations_in_rollback);
      }
    }

    PruneObservationGroupHistory();
  }
  // Remove idle tracks and update life state
  std::unordered_set<uint32_t> delete_ids;
  for (int i = 0; i < tracks_.size(); ++i) {
    auto& track = *tracks_[i];
    if (observations.has_min_timestamp()) {
      const double timestamp = observations.min_timestamp();
      if (track_life_manager_.ShouldChangeToLost(timestamp, track)) {
        track.track_state.life_state = TrackLifeState::TLS_LOST;
      } else if (track_life_manager_.ShouldChangeToIdle(timestamp, track)) {
        track.track_state.life_state = TrackLifeState::TLS_IDLE;
      }
    }
    if (track.track_state.life_state == TrackLifeState::TLS_IDLE) {
      delete_ids.insert(track.track_state.id);
    }
  }

  RemoveTracks(delete_ids);
}

void Tracker::RemoveTracks(const std::unordered_set<uint32_t>& delete_ids) {
  tracks_.erase(
      std::remove_if(tracks_.begin(), tracks_.end(),
                     [&](const auto& track) {
                       return delete_ids.count(track->track_state.id) > 0;
                     }),
      tracks_.end());
  RecoverTrackIds(delete_ids);
}

uint32_t Tracker::NewTrackId() { return id_manager_.GenerateNewId(); }

void Tracker::RecoverTrackIds(const std::unordered_set<uint32_t>& trackIds) {
  // Call the API function to return the ID to the pool
  for (const auto& id : trackIds) {
    id_manager_.RecoverId(id);
  }
}

std::vector<TrackPtr> Tracker::GetTracks() const { return tracks_; }

std::vector<TrackPtr> Tracker::GetConfirmedTracks() const {
  std::vector<TrackPtr> confirmed_tracks;
  confirmed_tracks.reserve(tracks_.size());
  for (const auto track : tracks_) {
    if (track->track_state.life_state == TrackLifeState::TLS_CONFIRMED) {
      confirmed_tracks.push_back(track);
    }
  }
  return confirmed_tracks;
}

template <typename PreLidarType, typename PreCameraType>
std::tuple<ObservationsProto, ObservationsProto> Tracker::PreProcess(
    PreLidarType* Lidar_Det_Objs, PreCameraType* Camera_Det_Objs,
    const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
    const double CurrentTimestamp) {
  // doing the pose update for localization services
  auto vehicle_pos_time = wrapper_->getVehiclePos(vehicle_odom_vec);
  UpdatePose(std::get<0>(vehicle_pos_time), std::get<1>(vehicle_pos_time));

  // making the frames
  ObservationsProto lidar_frame =
      wrapper_->LidarInputWrapper(*Lidar_Det_Objs, CurrentTimestamp);
  ObservationsProto camera_frame =
      wrapper_->CameraInputWrapper(*Camera_Det_Objs, CurrentTimestamp);
  // Doing the coordinate conversion
  wrapper_->MiddleWrapper(&lidar_frame, &camera_frame, vehicle_odom_vec,
                          CurrentTimestamp);

  return std::make_tuple(lidar_frame, camera_frame);
};

template <typename PostOutputType>
PostOutputType Tracker::PostProcess(const std::vector<TrackPtr>& Track_List,
                                    const double CurrentTimestamp) {
  PostOutputType output_objs;
  wrapper_->OutputWrapper(&output_objs, Track_List, CurrentTimestamp);
  return output_objs;
}

}  // namespace tracker

// Explicit template instantiation for Fusion_Tracker case
template std::tuple<tracker::ObservationsProto, tracker::ObservationsProto>
tracker::Tracker::PreProcess<perception::vector_LiDARPerception2Fusion,
                             perception::vector_CameraPerception2Fusion>(
    perception::vector_LiDARPerception2Fusion*,
    perception::vector_CameraPerception2Fusion*, const dpc::SLAMLocalEgoOdom&,
    const double);
template dpc::TrafficParticipantStruct tracker::Tracker::PostProcess<
    dpc::TrafficParticipantStruct>(const std::vector<TrackPtr>&, const double);

// Explicit template instantiation for Lidar-Tracker case
template std::vector<perception::LiDARPerception2Fusion>
tracker::Tracker::PostProcess<std::vector<perception::LiDARPerception2Fusion>>(
    const std::vector<TrackPtr>&, const double);

// Explicit template instantiation for Camera-Tracker case
template std::vector<perception::CameraPerception2Fusion>
tracker::Tracker::PostProcess<std::vector<perception::CameraPerception2Fusion>>(
    const std::vector<TrackPtr>&, const double);

// Explicit template instantiation for both Lidar and Camera Tracker case
template std::tuple<tracker::ObservationsProto, tracker::ObservationsProto>
tracker::Tracker::PreProcess<std::vector<LidarPerceptionOutput>,
                             std::vector<InstanceData>>(
    std::vector<LidarPerceptionOutput>*, std::vector<InstanceData>*,
    const dpc::SLAMLocalEgoOdom&, const double);
