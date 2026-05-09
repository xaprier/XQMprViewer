#ifndef COLORADAPTER_HPP
#define COLORADAPTER_HPP

#include <QColor>

namespace adapters {

/**
 * @brief Converts between QColor and VTK-style [R, G, B] double arrays.
 */
class ColorAdapter {
  public:
    /** @brief Writes the [0,1] RGB components of @p color into @p rgb. */
    static void QColorToRGB(const QColor& color, std::array<double, 3>& rgb) {
        rgb[0] = color.redF();
        rgb[1] = color.greenF();
        rgb[2] = color.blueF();
    }

    /** @brief Constructs a QColor from a [0,1] RGB double array. */
    static QColor RGBToQColor(const std::array<double, 3>& rgb) {
        return QColor::fromRgbF(rgb[0], rgb[1], rgb[2]);
    }
};
}  // namespace adapters

#endif  // COLORADAPTER_HPP
