#include "XQVtkViewport/ViewportLayout.hpp"

namespace qvv {

std::vector<ViewportConfig> ViewportLayout::HorizontalSplit(
    int count,
    std::array<double, 3> background,
    const std::vector<std::string>& labels) {
    if (count < 1)
        count = 1;

    std::vector<ViewportConfig> configs;
    configs.reserve(count);

    const double step = 1.0 / count;
    for (int i = 0; i < count; ++i) {
        ViewportConfig cfg;
        cfg.xMin = i * step;
        cfg.xMax = (i + 1) * step;
        cfg.yMin = 0.0;
        cfg.yMax = 1.0;
        cfg.background = background;
        if (i < static_cast<int>(labels.size()))
            cfg.label = labels[i];
        configs.push_back(cfg);
    }
    return configs;
}

std::vector<ViewportConfig> ViewportLayout::VerticalSplit(
    int count,
    std::array<double, 3> background,
    const std::vector<std::string>& labels) {
    if (count < 1)
        count = 1;

    std::vector<ViewportConfig> configs;
    configs.reserve(count);

    const double step = 1.0 / count;
    for (int i = 0; i < count; ++i) {
        ViewportConfig cfg;
        cfg.xMin = 0.0;
        cfg.xMax = 1.0;
        cfg.yMin = i * step;
        cfg.yMax = (i + 1) * step;
        cfg.background = background;
        if (i < static_cast<int>(labels.size()))
            cfg.label = labels[i];
        configs.push_back(cfg);
    }
    return configs;
}

std::vector<ViewportConfig> ViewportLayout::Grid(
    int rows,
    int cols,
    std::array<double, 3> background,
    const std::vector<std::string>& labels) {
    if (rows < 1) rows = 1;
    if (cols < 1) cols = 1;

    std::vector<ViewportConfig> configs;
    configs.reserve(rows * cols);

    const double colStep = 1.0 / cols;
    const double rowStep = 1.0 / rows;

    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            ViewportConfig cfg;
            cfg.xMin = c * colStep;
            cfg.xMax = (c + 1) * colStep;
            // VTK uses bottom-left origin; row 0 is top visually, so invert Y.
            cfg.yMin = (rows - 1 - r) * rowStep;
            cfg.yMax = (rows - r) * rowStep;
            cfg.background = background;
            if (idx < static_cast<int>(labels.size()))
                cfg.label = labels[idx];
            configs.push_back(cfg);
            ++idx;
        }
    }
    return configs;
}

}  // namespace qvv
