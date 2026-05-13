#ifndef CONTROLLERPANELCORNERANNOTATIONITEM_HPP
#define CONTROLLERPANELCORNERANNOTATIONITEM_HPP

#include <QColor>
#include <QWidget>

#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/IOverlay.hpp"

class QCheckBox;
class QComboBox;
class QSpinBox;
class QPushButton;

namespace ui {

/**
 * @brief Control panel item for corner annotation overlay configuration.
 *
 * Manages one or more CornerAnnotationOverlay instances simultaneously.
 * All attached overlays are kept in sync with the panel settings.
 *
 * Controls:
 *  - Enable / disable (checkbox)
 *  - Position (combo: same options as FPS overlay)
 *  - Show slice info (checkbox)
 *  - Show window/level (checkbox)
 *  - Show spacing (checkbox)
 *  - Show view name (checkbox)
 *  - Font size (spin box, pt)
 *  - Margin (spin box, px)
 *  - Text color (color picker button)
 */
class ControllerPanelCornerAnnotationItem : public QWidget {
    Q_OBJECT

  public:
    explicit ControllerPanelCornerAnnotationItem(QWidget* parent = nullptr);
    ~ControllerPanelCornerAnnotationItem() override;

    void AddOverlay(overlays::CornerAnnotationOverlay* overlay);
    void ClearOverlays();

  Q_SIGNALS:
    void CornerAnnotationEnableChanged(bool enabled);
    void CornerAnnotationPositionChanged(const overlays::OverlayPosition& position);
    void CornerAnnotationColorChanged(const QColor& color);
    void CornerAnnotationFontSizeChanged(int fontSize);
    void CornerAnnotationMarginChanged(int margin);
    void CornerAnnotationShowSliceInfoChanged(bool show);
    void CornerAnnotationShowWindowLevelChanged(bool show);
    void CornerAnnotationShowSpacingChanged(bool show);
    void CornerAnnotationShowViewNameChanged(bool show);

  private:
    void _setupUi();

    void _onEnabledToggled(bool checked);
    void _onPositionChanged(int index);
    void _onShowSliceInfoToggled(bool checked);
    void _onShowWindowLevelToggled(bool checked);
    void _onShowSpacingToggled(bool checked);
    void _onShowViewNameToggled(bool checked);
    void _onFontSizeChanged(int pt);
    void _onMarginChanged(int px);
    void _onColorPicked();

    QCheckBox*   m_enableCheck{nullptr};
    QComboBox*   m_positionCombo{nullptr};
    QCheckBox*   m_showSliceInfoCheck{nullptr};
    QCheckBox*   m_showWindowLevelCheck{nullptr};
    QCheckBox*   m_showSpacingCheck{nullptr};
    QCheckBox*   m_showViewNameCheck{nullptr};
    QSpinBox*    m_fontSizeSpin{nullptr};
    QSpinBox*    m_marginSpin{nullptr};
    QPushButton* m_colorButton{nullptr};

    QColor m_textColor{Qt::white};
    QList<overlays::CornerAnnotationOverlay*> m_overlays;
};

}  // namespace ui

#endif  // CONTROLLERPANELCORNERANNOTATIONITEM_HPP
