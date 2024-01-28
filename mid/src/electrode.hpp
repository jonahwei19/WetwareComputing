#ifndef MID_ELECTRODE_H
#define MID_ELECTRODE_H

#include "utils.hpp"

#include <format>
#include <map>
#include <string>

typedef struct Spike {
  JuceTime timestamp;
  std::string electrode_name;
} Spike;

typedef struct TriggerConditions {
  JuceTime maximum_interval;
  int number_of_spikes;
} TriggerConditions;

class Electrode {
private:
  std::string name;
  Spike last_spike;
  TriggerConditions trigger_conditions;
  int frequency_score = 0;
  JuceTime last_trigger;

public:
  Electrode() {}
  explicit Electrode(const Spike first_spike,
                     const TriggerConditions trigger_conditions)
      : name{first_spike.electrode_name}, last_spike{first_spike},
        trigger_conditions{trigger_conditions} {}
  void UpdateFrequencyScore(const Spike new_spike) {
    JuceTime gap_between_spikes = new_spike.timestamp - last_spike.timestamp;
    if (gap_between_spikes < trigger_conditions.maximum_interval) {
      ++frequency_score;
    } else {
      frequency_score = 0;
    }
    if (frequency_score == trigger_conditions.number_of_spikes) {
      LogText(std::format("{}: trigger", name));
      frequency_score = 0;
    }
    last_spike = new_spike;
  }
};

typedef std::map<std::string, Electrode> ElectrodeMap;

#endif
