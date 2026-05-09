#ifndef CONTROLLERPANEL_HPP
#define CONTROLLERPANEL_HPP

#include <QWidget>

class QColor;

namespace ui {
class ControllerPanelDicomItem;
class ControllerPanelSphereItem;

/**
 * @brief Shared control panel used by both Viewport and Multi-Window modes.
 *
 * Stacks a ControllerPanelDicomItem (folder/series selection) and a
 * ControllerPanelSphereItem (sphere toggle, radius, colour) with a separator
 * between them. A status label at the bottom reflects incoming StatusChanged
 * notifications.
 */
class ControllerPanel : public QWidget {
    Q_OBJECT
  public:
    explicit ControllerPanel(QWidget* parent = nullptr);
    ~ControllerPanel() override;

    void SetSeries(const QStringList& seriesNames);

    [[nodiscard]] ControllerPanelDicomItem* GetDicomItem() const;
    [[nodiscard]] ControllerPanelSphereItem* GetSphereItem() const;

  Q_SIGNALS:
    void DirectorySelected(const QString& directory);
    void SeriesLoadRequested(int seriesIndex);
    void SphereAddRemoveClicked();
    void SphereRadiusChanged(double radius);
    void SphereColorChanged(const QColor& color);

  private:
    void _setupUi();

    ControllerPanelDicomItem* m_dicomItem{nullptr};
    ControllerPanelSphereItem* m_sphereItem{nullptr};
};
}  // namespace ui

#endif  // CONTROLLERPANEL_HPP
