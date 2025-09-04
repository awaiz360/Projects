#include "association/associator_base.h"

#include <Eigen/Core>
#include <vector>

#include "localization_service.h"
#include "track.h"
#include "type_gate.h"

namespace tracker {

AssociatorBase::AssociatorBase(const LocalizationService* localization_service)
    : localization_service_(localization_service) {}

std::vector<int> AssociatorBase::Associate(
    const std::vector<TrackPtr>& tracks,
    const std::vector<const ObservationProto*>& observations) const {
  Eigen::MatrixXd similarity_matrix;
  const int row = observations.size();
  const int col = tracks.size();
  similarity_matrix.resize(row, col);
  similarity_matrix.setConstant(-1);

  // Build similarity matrix
  for (int i = 0; i < col; ++i) {
    const auto& track = *(tracks[i]);
    for (int j = 0; j < row; ++j) {
      const auto* observation = observations[j];
      if (TypeGate::CanAssociate(track, *observation)) {
        similarity_matrix(j, i) = ComputeSimilarity(track, *observation);
      } else {
        similarity_matrix(j, i) = 0;
      }
    }
  }

  // Solve similarity matrix
  const auto observation_to_track = GreedyNearestNeighbor(similarity_matrix);

  return observation_to_track;
}

std::vector<int> AssociatorBase::GreedyNearestNeighbor(
    const Eigen::MatrixXd& similarity_matrix) const {
  const int rows = similarity_matrix.rows();
  const int cols = similarity_matrix.cols();
  std::vector<bool> matched_rows(rows);  // observations
  std::vector<bool> matched_cols(cols);  // tracks
  std::vector<int> match_result(rows, -1);
  while (true) {
    double max_val = -std::numeric_limits<double>::max();
    int max_val_col, max_val_row;
    for (int i = 0; i < cols; ++i) {
      if (matched_cols[i]) {
        continue;
      }
      for (int j = 0; j < rows; ++j) {
        if (matched_rows[j]) {
          continue;
        }
        if (similarity_matrix(j, i) > max_val) {
          max_val = similarity_matrix(j, i);
          max_val_col = i;
          max_val_row = j;
        }
      }
    }
    if (max_val > 0.05) {
      matched_cols[max_val_col] = true;
      matched_rows[max_val_row] = true;
      match_result[max_val_row] = max_val_col;
    } else {
      break;
    }
  }
  return match_result;
}

}  // namespace tracker
