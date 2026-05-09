#ifndef CONTROLLERPANELDICOMITEM_HPP
#define CONTROLLERPANELDICOMITEM_HPP

#include <QWidget>

class QLabel;
class QListWidget;
class QPropertyAnimation;
class QPushButton;
class QStringListModel;

namespace ui {

/**
 * @brief Control panel item for DICOM directory and series selection.
 *
 * Pressing "Select Series" expands an animated series list; clicking an
 * entry selects it and collapses the list automatically.
 */
class ControllerPanelDicomItem : public QWidget {
    Q_OBJECT
  public:
    explicit ControllerPanelDicomItem(QWidget* parent = nullptr);
    ~ControllerPanelDicomItem() override;

    void SetSeries(const QStringList& seriesNames);

  Q_SIGNALS:
    void DirectorySelected(const QString& directory);
    void SeriesLoadRequested(int seriesIndex);

  private slots:
    void _OnSeriesToggle();
    void _OnSeriesItemClicked(int row);

  private:
    void _setupUi();
    void _SetListVisible(bool visible);

    QPushButton* m_browseButton{nullptr};
    QPushButton* m_seriesToggleButton{nullptr};
    QListWidget* m_seriesList{nullptr};
    QPushButton* m_loadButton{nullptr};
    QLabel* m_selectedLabel{nullptr};

    QString m_currentDirectory;
    int m_selectedIndex{-1};
    bool m_listExpanded{false};
    QPropertyAnimation* m_listAnimation{nullptr};
};

}  // namespace ui

#endif  // CONTROLLERPANELDICOMITEM_HPP
