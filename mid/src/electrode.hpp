#ifndef MID_ELECTRODE_H
#define MID_ELECTRODE_H

#include "time_utils.hpp"

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
  int frequency_score;
  Spike last_spike;
  TriggerConditions trigger_conditions;

public:
  explicit Electrode(const Spike first_spike,
                     const TriggerConditions trigger_conditions)
      : name{first_spike.electrode_name}, last_spike{first_spike},
        trigger_conditions{trigger_conditions} {
    frequency_score = 0;
  }
  void UpdateFrequencyScore(const Spike new_spike);
};
#endif
