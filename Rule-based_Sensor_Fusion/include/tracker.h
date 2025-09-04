#ifndef TRACKER_TRACKER_H_
#define TRACKER_TRACKER_H_

#include <Eigen/Core>
#include <deque>
#include <memory>

#include "association/associator_base.h"
#include "association/camera_tracker_associator.h"
#include "association/fusion_tracker_associator.h"
#include "association/lidar_tracker_associator.h"
#include "const_def.h"
#include "localization_service.h"
#include "logger.h"
#include "pre_and_post_process/camera_wrapper.h"
#include "pre_and_post_process/fusion_wrapper.h"
#include "pre_and_post_process/lidar_wrapper.h"
#include "pre_and_post_process/wrapper_base.h"
#include "track.h"
#include "track_id_manager.h"
#include "track_life_manager.h"
#include "track_updater/camera_track_updater.h"
#include "track_updater/fusion_track_updater.h"
#include "track_updater/lidar_track_updater.h"
#include "track_updater/updater_base.h"
#include "types.h"

namespace tracker {

class Tracker {
 public:
  explicit Tracker(TrackerEngineType engine_type);

  bool UpdatePose(const Eigen::Matrix4d& pose, const double timestamp);

  void Tracking(const ObservationsProto& observations);

  std::vector<TrackPtr> GetTracks() const;

  std::vector<TrackPtr> GetConfirmedTracks() const;

  TrackerEngineType GetEnginType() const { return engine_type_; };

  template <typename PreLidarType, typename PreCameraType>
  std::tuple<ObservationsProto, ObservationsProto> PreProcess(
      PreLidarType* Lidar_Det_Objs, PreCameraType* Camera_Det_Objs,
      const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
      const double CurrentTimestamp);

  template <typename PostOutputType>
  PostOutputType PostProcess(const std::vector<TrackPtr>& Track_List,
                             const double CurrentTimestamp);

 private:
  uint32_t NewTrackId();

  std::unique_ptr<Track> NewTrackFromObservation(
      const ObservationProto& observation);

  void CreateNewTracks(
      const std::vector<const ObservationProto*>& observations);

  void DeleteTimeOutTracks(double timestamp);

  void RecoverTrackIds(const std::unordered_set<uint32_t>& trackIds);

  void SaveCheckPoint();

  void PruneObservationGroupHistory();

  void AssociateObservationsAndUpdateTracks(
      const std::vector<const ObservationProto*> valid_observations);

  void RemoveTracks(const std::unordered_set<uint32_t>& delete_ids);

  std::deque<std::shared_ptr<const ObservationsProto>>
      observations_group_history_;

 private:
  std::vector<TrackPtr> tracks_;
  TrackIdManager id_manager_;
  LocalizationService localization_service_;
  std::unique_ptr<UpdaterBase> track_updater_ = nullptr;
  std::unique_ptr<AssociatorBase> associator_ = nullptr;
  std::unique_ptr<WrapperBase> wrapper_ = nullptr;
  TrackLifeManager track_life_manager_;
  TrackerEngineType engine_type_;
};

}  // namespace tracker

#endif  // TRACKER_TRACKER_H_
