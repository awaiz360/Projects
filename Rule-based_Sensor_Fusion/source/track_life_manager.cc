#include "track_life_manager.h"

#include "track.h"
#include "types.h"

namespace tracker {
constexpr int kMinLifetimeToConfirmed = 5;                // frames
constexpr double kMaxNoObsTimeForKeepingConfirmed = 0.3;  // s
constexpr double kMaxNoObsTimeForConfirmed2Lost = 0.3;    // s
// constexpr double kMaxNoObsTimeForIdle = 0.5;  //s

// To be consistent with the original code
constexpr double kMaxNoObsTimeForIdle = 0.3;  // s

bool TrackLifeManager::ShouldChangeToConfirmed(const double timestamp,
                                               const Track& track) const {
  if (track.track_state.life_state == TrackLifeState::TLS_INIT) {
    if (track.track_state.lifetime >= kMinLifetimeToConfirmed) {
      return true;
    }
  } else if (track.track_state.life_state == TrackLifeState::TLS_LOST) {
    const double time_to_last_observation =
        timestamp - track.track_state.timestamp;
    if (time_to_last_observation < kMaxNoObsTimeForKeepingConfirmed) {
      return true;
    }
  }
  return false;
}

bool TrackLifeManager::ShouldChangeToLost(const double timestamp,
                                          const Track& track) const {
  if (track.track_state.life_state == TrackLifeState::TLS_CONFIRMED) {
    const double time_to_last_observation =
        timestamp - track.track_state.timestamp;
    if (time_to_last_observation > kMaxNoObsTimeForConfirmed2Lost) {
      return true;
    }
  }
  return false;
}

bool TrackLifeManager::ShouldChangeToIdle(const double timestamp,
                                          const Track& track) const {
  if (track.track_state.life_state == TrackLifeState::TLS_LOST) {
    const double time_to_last_observation =
        timestamp - track.track_state.timestamp;
    if (time_to_last_observation > kMaxNoObsTimeForIdle) {
      return true;
    }
  } else if (track.track_state.life_state == TrackLifeState::TLS_INIT) {
    const double time_to_last_observation =
        timestamp - track.track_state.timestamp;
    if (time_to_last_observation > kMaxNoObsTimeForIdle) {
      return true;
    }
  }
  return false;
}

}  // namespace tracker
