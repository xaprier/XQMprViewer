#ifndef XQVtkViewport_RENDERSTATSMODEL_HPP
#define XQVtkViewport_RENDERSTATSMODEL_HPP

#include <QAbstractTableModel>
#include <QObject>
#include <XQVtkViewport/IViewportObserver.hpp>
#include <XQVtkViewport/RenderStats.hpp>
#include <vector>

namespace adapters {

/**
 * @brief Implements IViewportObserver and exposes render statistics as a Qt table model.
 *
 * Rows correspond to viewport indices; columns map to statistic fields:
 *   Col 0 → FPS
 *   Col 1 → Frame ms
 *   Col 2 → Render ms
 */
class RenderStatsModel
    : public QAbstractTableModel,
      public qvv::IViewportObserver {
    Q_OBJECT

  public:
    explicit RenderStatsModel(QObject* parent = nullptr);
    ~RenderStatsModel() override;

    // ── QAbstractTableModel ──────────────────────────────────────────────
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    // ── IViewportObserver ────────────────────────────────────────────────
    void OnViewportAdded(int index) override;
    void OnViewportResized(int index, int w, int h) override;
    void OnFrameRendered(int index, const qvv::RenderStats& stats) override;

    /**
     * @brief Sets the display label for each row.
     * @param labels List such as {"Axial", "Coronal", "Sagittal"}.
     */
    void SetRowLabels(const QStringList& labels);

  private:
    std::vector<qvv::RenderStats> m_stats;
    QStringList m_labels;
};

}  // namespace adapters

#endif  // XQVtkViewport_RENDERSTATSMODEL_HPP
