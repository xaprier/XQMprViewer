#ifndef CONTROLLERPANELFPSOVERLAYITEM_HPP
#define CONTROLLERPANELFPSOVERLAYITEM_HPP

#include <QColor>
#include <QWidget>

#include "overlays/FPSOverlay.hpp"  // included because it's used in QList

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;

namespace overlays {
enum class OverlayPosition;
}  // namespace overlays

namespace ui {

/**
 * @brief Control panel item for FPS overlay configuration.
 *
 * Manages one or more FPSOverlay instances simultaneously.
 * All attached overlays are kept in sync with the panel settings.
 *
 * Controls:
 *  - Enable / disable toggle (checkbox)
 *  - Corner position (combo: Top-Left, Top-Right, Top-Center, Bottom-Left, Bottom-Right, Bottom-Center)
 *  - Margin (spin box, pixels)
 *  - Text color (color picker button)
 *  - Font size (spin box, pt)
 */
class ControllerPanelFPSOverlayItem : public QWidget {
    Q_OBJECT

  public:
    explicit ControllerPanelFPSOverlayItem(QWidget* parent = nullptr);
    ~ControllerPanelFPSOverlayItem() override;

    /** @brief Attach an overlay to be controlled by this panel. */
    void AddOverlay(overlays::FPSOverlay* overlay);

    /** @brief Detach all overlays. */
    void ClearOverlays();

  Q_SIGNALS:
    void FPSOverlayEnableChanged(bool enabled);
    void FPSOverlayColorChanged(const QColor& color);
    void FPSOverlayPositionChanged(const overlays::OverlayPosition& position);
    void FPSOverlayMarginChanged(int margin);
    void FPSOverlayFontSizeChanged(int fontSize);

  private:
    void _setupUi();

    void _onEnabledToggled(bool checked);
    void _onPositionChanged(int index);
    void _onMarginChanged(int px);
    void _onFontSizeChanged(int pt);
    void _onColorPicked();

    QCheckBox* m_enableCheck{nullptr};
    QComboBox* m_positionCombo{nullptr};
    QSpinBox* m_marginSpin{nullptr};
    QSpinBox* m_fontSizeSpin{nullptr};
    QPushButton* m_colorButton{nullptr};

    QColor m_textColor{Qt::yellow};
    QList<overlays::FPSOverlay*> m_overlays;
};

}  // namespace ui

#endif  // CONTROLLERPANELFPSOVERLAYITEM_HPP
