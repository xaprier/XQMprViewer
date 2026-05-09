#ifndef CONTROLLERPANELSPHEREITEM_HPP
#define CONTROLLERPANELSPHEREITEM_HPP

#include <QWidget>

class QColor;
class QDoubleSpinBox;
class QPushButton;

namespace ui {

/**
 * @brief Control panel item for sphere add/remove, radius, and colour settings.
 *
 * Contains a single toggle button (not a spawner), a radius spin box, and a
 * colour picker button. At most one sphere can be active at a time.
 */
class ControllerPanelSphereItem : public QWidget {
    Q_OBJECT
  public:
    explicit ControllerPanelSphereItem(QWidget* parent = nullptr);
    ~ControllerPanelSphereItem() override;

    [[nodiscard]] double GetRadius() const;
    [[nodiscard]] QColor GetColor() const;

  Q_SIGNALS:
    void SphereAddRemoveClicked();
    void SphereRadiusChanged(double radius);
    void SphereColorChanged(const QColor& color);

  private:
    void _setupUi();

    QPushButton* m_enableButton{nullptr};
    QDoubleSpinBox* m_radiusSpin{nullptr};
    QPushButton* m_colorButton{nullptr};
    QColor m_color;
};

}  // namespace ui

#endif  // CONTROLLERPANELSPHEREITEM_HPP
