#ifndef CONTROLLERPANELORIENTATIONMARKERITEM_HPP
#define CONTROLLERPANELORIENTATIONMARKERITEM_HPP

#include <QColor>
#include <QWidget>

#include "overlays/OrientationMarkerOverlay.hpp"

class QCheckBox;
class QSpinBox;
class QPushButton;

namespace ui {

/**
 * @brief Control panel item for anatomical orientation marker overlay.
 *
 * Manages one or more OrientationMarkerOverlay instances simultaneously.
 * All attached overlays are kept in sync with the panel settings.
 *
 * Controls:
 *  - Enable / disable toggle (checkbox)
 *  - Long labels toggle: switches between L/R/A/P/S/I and full anatomical names
 *  - Font size (spin box, pt)
 *  - Label color (color picker button)
 */
class ControllerPanelOrientationMarkerItem : public QWidget {
    Q_OBJECT

  public:
    explicit ControllerPanelOrientationMarkerItem(QWidget* parent = nullptr);
    ~ControllerPanelOrientationMarkerItem() override;

    /** @brief Attach an overlay to be controlled by this panel. */
    void AddOverlay(overlays::OrientationMarkerOverlay* overlay);

    /** @brief Detach all overlays. */
    void ClearOverlays();

  Q_SIGNALS:
    void OrientationMarkerEnableChanged(bool enabled);
    void OrientationMarkerLongLabelsChanged(bool longLabels);
    void OrientationMarkerColorChanged(const QColor& color);
    void OrientationMarkerFontSizeChanged(int fontSize);

  private:
    void _setupUi();

    void _onEnabledToggled(bool checked);
    void _onLongLabelsToggled(bool checked);
    void _onFontSizeChanged(int pt);
    void _onColorPicked();

    QCheckBox*   m_enableCheck{nullptr};
    QCheckBox*   m_longLabelsCheck{nullptr};
    QSpinBox*    m_fontSizeSpin{nullptr};
    QPushButton* m_colorButton{nullptr};

    QColor m_textColor{Qt::yellow};
    QList<overlays::OrientationMarkerOverlay*> m_overlays;
};

}  // namespace ui

#endif  // CONTROLLERPANELORIENTATIONMARKERITEM_HPP
