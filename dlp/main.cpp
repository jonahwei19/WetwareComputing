#include <memory>
#include <mutex>
#include <iostream>

#include "dlp.hpp"
#include "mea_pattern.hpp"

int main() {
    std::shared_ptr<MeaPattern> mea_pattern = std::make_shared<MeaPattern>(dlp_resolution_width, dlp_resolution_height);
    Dlp dlp(mea_pattern);

    dlp.startRendering();

    size_t curr_row = 0;
    size_t curr_col = 0;

    // iterate through stimulating all regions as fast as possible until we exit the program
    while (dlp.getRenderThread().joinable()) {
        // use try semantics for writing and discard changes made during a render
        // assumes the main thread runs (much) faster than the render loop (120hz)
        std::unique_lock<std::mutex> lock(mea_pattern->getMutex(), std::try_to_lock);
        if (lock.owns_lock()) {
            mea_pattern->clearRegionStimulation();
            mea_pattern->setRegionStimulation(curr_col, curr_row, true);
            
            ++curr_row;
            ++curr_col;
            if (curr_row >= mea_rows) curr_row = 0;
            if (curr_col >= mea_cols) curr_col = 0;
        } // else we discard this pattern (go get newer data while we render)
    }
    
    
}