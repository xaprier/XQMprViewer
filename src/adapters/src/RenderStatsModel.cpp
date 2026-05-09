#include "adapters/RenderStatsModel.hpp"

namespace adapters {

RenderStatsModel::RenderStatsModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

RenderStatsModel::~RenderStatsModel() = default;

int RenderStatsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_stats.size());
}

int RenderStatsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant RenderStatsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};
    if (index.row() >= static_cast<int>(m_stats.size()))
        return {};

    if (role != Qt::DisplayRole)
        return {};

    const auto& stats = m_stats[static_cast<size_t>(index.row())];
    switch (index.column()) {
        case 0:
            return QString::number(stats.AverageFps(), 'f', 1);
        case 1:
            return QString::number(stats.frameTimeMs, 'f', 2);
        case 2:
            return QString::number(stats.renderTimeMs, 'f', 2);
        default:
            return {};
    }
}

QVariant RenderStatsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return "FPS";
            case 1:
                return "Frame ms";
            case 2:
                return "Render ms";
            default:
                return {};
        }
    }

    if (orientation == Qt::Vertical) {
        if (section < m_labels.size())
            return m_labels[section];
        return QString("Viewport %1").arg(section);
    }
    return {};
}

void RenderStatsModel::OnViewportAdded(int /*index*/) {
    beginInsertRows(QModelIndex(), static_cast<int>(m_stats.size()),
                    static_cast<int>(m_stats.size()));
    m_stats.emplace_back();
    endInsertRows();
}

void RenderStatsModel::OnViewportResized(int /*index*/, int /*w*/, int /*h*/) {
    // No model change needed for resize
}

void RenderStatsModel::OnFrameRendered(int index, const qvv::RenderStats& stats) {
    if (index < 0 || index >= static_cast<int>(m_stats.size()))
        return;
    m_stats[static_cast<size_t>(index)] = stats;
    emit dataChanged(
        createIndex(index, 0),
        createIndex(index, 2),
        {Qt::DisplayRole});
}

void RenderStatsModel::SetRowLabels(const QStringList& labels) {
    m_labels = labels;
    if (!m_stats.empty())
        emit headerDataChanged(Qt::Vertical, 0, static_cast<int>(m_stats.size()) - 1);
}

}  // namespace adapters
