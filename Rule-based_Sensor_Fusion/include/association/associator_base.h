#ifndef TRACKER_ASSOCIATION_ASSOCIATOR_BASE_H_
#define TRACKER_ASSOCIATION_ASSOCIATOR_BASE_H_

#include <vector>

#include "Eigen/Core"
#include "localization_service.h"
#include "track.h"

namespace tracker {

class AssociatorBase {
 public:
  AssociatorBase(const LocalizationService* localization_service);
  virtual ~AssociatorBase() = default;

  virtual std::vector<int> Associate(
      const std::vector<TrackPtr>& tracks,
      const std::vector<const ObservationProto*>& observations) const;

 protected:
  virtual double ComputeSimilarity(
      const Track& track, const ObservationProto& observation) const = 0;

  virtual std::vector<int> GreedyNearestNeighbor(
      const Eigen::MatrixXd& similarity_matrix) const;

 protected:
  // Note: Maybe unsued.
  const LocalizationService* localization_service_ = nullptr;
};

}  // namespace tracker

#endif  // TRACKER_ASSOCIATION_ASSOCIATOR_BASE_H_
