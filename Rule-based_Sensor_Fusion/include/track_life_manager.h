#ifndef TRACKER_TRACK_LIFE_MANAGER_H_
#define TRACKER_TRACK_LIFE_MANAGER_H_

#include "track.h"
#include "types.h"

namespace tracker {

// Simple track life manager. Needs to tuned for specific use cases. Maybe use a
// base class.
class TrackLifeManager {
 public:
  TrackLifeManager() = default;
  ~TrackLifeManager() = default;

  bool ShouldChangeToConfirmed(const double timestamp,
                               const Track& track) const;

  bool ShouldChangeToLost(const double timestamp, const Track& track) const;

  bool ShouldChangeToIdle(const double timestamp, const Track& track) const;
};

}  // namespace tracker

#endif  // TRACKER_TRACK_LIFE_MANAGER_H_
