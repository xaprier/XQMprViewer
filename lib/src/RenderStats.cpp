#include "XQVtkViewport/RenderStats.hpp"

namespace qvv {

double RenderStats::AverageFps() const {
    if (frameTimeMs <= 0.0)
        return 0.0;
    return 1000.0 / frameTimeMs;
}

double RenderStats::AverageFrameTimeMs() const {
    if (frameCount <= 0)
        return 0.0;
    return frameTimeMs;
}

void RenderStats::Reset() {
    frameTimeMs = 0.0;
    renderTimeMs = 0.0;
    frameCount = 0;
}

}  // namespace qvv
