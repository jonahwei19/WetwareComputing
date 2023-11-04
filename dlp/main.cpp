#include <memory>

#include "dlp.hpp"
#include "mea_pattern.hpp"

int main() {
    std::shared_ptr<MeaPattern> mea_pattern = std::make_shared<MeaPattern>(dlp_resolution_width, dlp_resolution_height);
    Dlp dlp(mea_pattern);

    mea_pattern->setRegionStimulation(0, 0, true);
    dlp.start();
}