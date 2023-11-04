#include <memory>

#include "dlp.hpp"
#include "mea_pattern.hpp"

int main() {
    std::shared_ptr<MeaPattern> mea_pattern = std::make_shared<MeaPattern>();
    Dlp dlp(mea_pattern);

    dlp.start();
}