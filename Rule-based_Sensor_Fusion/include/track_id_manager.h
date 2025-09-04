#ifndef TRACKER_TRACK_ID_MANAGER_H_
#define TRACKER_TRACK_ID_MANAGER_H_

#include <cstdint>
#include <iostream>
#include <queue>

#include "logger.h"

namespace tracker {

class TrackIdManager {
 public:
  TrackIdManager();
  ~TrackIdManager() = default;
  // Call this function when create new track.
  uint32_t GenerateNewId();
  // Call this function when erase track.
  bool RecoverId(const uint32_t id);

 private:
  // FIFO to avoid duplicate id.
  std::deque<uint32_t> valid_ids_;
};

}  // namespace tracker

#endif  // TRACKER_TRACK_ID_MANAGER_H_
