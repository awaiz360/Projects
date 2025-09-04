#include "track_id_manager.h"

namespace tracker {

TrackIdManager::TrackIdManager() {
  for (uint32_t i = 1; i < 20000; ++i) {
    valid_ids_.push_back(i);
  }
}
uint32_t TrackIdManager::GenerateNewId() {
  if (valid_ids_.empty()) {
    Logger::GetInstance().Log(
        Logger::FATAL,
        "Fatal error: Track ID pool is exhausted. No new IDs available.");
    std::cout << "Running out of track IDs." << std::endl;
    return 1u;
  }
  const uint32_t id = valid_ids_.front();
  valid_ids_.pop_front();
  return id;
}

bool TrackIdManager::RecoverId(uint32_t id) {
  valid_ids_.push_back(id);
  return true;  // Indicate success
}

}  // namespace tracker
