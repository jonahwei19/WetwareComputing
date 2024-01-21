#include <memory>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>

#include "dlp.hpp"
#include "mea_pattern.hpp"

void iterateStimulatedRegion(std::shared_ptr<MeaPattern> mea_pattern);
void toggleFlash(std::shared_ptr<MeaPattern> mea_pattern);
template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start) {
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

int main() {
    std::shared_ptr<MeaPattern> mea_pattern = std::make_shared<MeaPattern>(dlp_resolution_width, dlp_resolution_height);
    Dlp dlp(mea_pattern);

    dlp.startRendering();
    auto start = std::chrono::steady_clock::now();
    // iterate through stimulating all regions as fast as possible until we exit the program
    while (dlp.getRenderThread().joinable()) {
        // use try semantics for writing and discard changes made during a render
        // assumes the main thread runs (much) faster than the render loop (120hz)
        std::unique_lock<std::mutex> lock(mea_pattern->getMutex(), std::try_to_lock);
        if (lock.owns_lock()) {
            if (since(start).count() > 1000) {
                toggleFlash(mea_pattern);
                start = std::chrono::steady_clock::now();
            }
        } // else we discard this pattern (go get newer data while we render)
    }
}

void iterateStimulatedRegion(std::shared_ptr<MeaPattern> mea_pattern) {
    static size_t curr_row = 0;
    static size_t curr_col = 0;

    mea_pattern->clearRegionStimulation();
    mea_pattern->setRegionStimulation(curr_col, curr_row, true);
    
    ++curr_row;
    ++curr_col;
    if (curr_row >= mea_rows) curr_row = 0;
    if (curr_col >= mea_cols) curr_col = 0;
}


void toggleFlash(std::shared_ptr<MeaPattern> mea_pattern) {
    static bool stimulate = false;
    mea_pattern->setGlobalStimulation(stimulate);
    stimulate = !stimulate;
}

