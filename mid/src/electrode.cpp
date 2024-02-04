#include "electrode.hpp"
#include "time_utils.hpp"

#include <format>
#include <iostream>

void Electrode::UpdateFrequencyScore(const Spike new_spike) {
  auto time_gap_between_spikes = new_spike.timestamp - last_spike.timestamp;
  if (time_gap_between_spikes < trigger_conditions.maximum_interval) {
    ++frequency_score;
  } else {
    frequency_score = 0;
  }
  if (frequency_score == trigger_conditions.number_of_spikes) {
    LogText(std::format("{}: trigger", name));
    frequency_score = 0;
  }
  last_spike = new_spike;
  std::cout << std::format("Delay: {}",
                           CurrentTimeMillis() - last_spike.timestamp)
            << std::endl;
}
