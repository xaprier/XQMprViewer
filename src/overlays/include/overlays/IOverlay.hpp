#ifndef IOVERLAY_HPP
#define IOVERLAY_HPP

#include <QList>
#include <QString>

namespace overlays {

/**
 * @brief Corner positions for the FPS overlay inside its parent widget.
 */
enum class OverlayPosition {
    TopLeft,
    TopRight,
    TopCenter,
    BottomLeft,
    BottomRight,
    BottomCenter,
};

QString QStringFromOverlayPosition(const OverlayPosition& pos);
OverlayPosition OverlayPositionFromQString(const QString& str);
QList<QString> GetOverlayPositionNames();

class IOverlay {
  public:
    virtual ~IOverlay() = default;

    /** @brief Enable or disable the overlay. */
    virtual void SetEnabled(bool enabled = true) = 0;
    [[nodiscard]] virtual bool IsEnabled() const {
        return m_enabled;
    }

    /** @brief Set position for the visible overlay */
    virtual void SetPosition(OverlayPosition pos = OverlayPosition::TopLeft) = 0;
    [[nodiscard]] OverlayPosition GetPosition() const {
        return m_position;
    }

  protected:
    OverlayPosition m_position{OverlayPosition::TopLeft};
    bool m_enabled{true};
};
}  // namespace overlays

#endif  // IOVERLAY_HPP
