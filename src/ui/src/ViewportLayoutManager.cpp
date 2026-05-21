#include "ui/ViewportLayoutManager.hpp"

#include "ui/ILayoutTarget.hpp"

namespace ui {

ViewportLayoutManager::ViewportLayoutManager(QObject* parent)
    : QObject(parent) {}

ViewportLayoutManager::~ViewportLayoutManager() = default;

void ViewportLayoutManager::RegisterTarget(ILayoutTarget* target) {
    if (!target)
        return;
    m_targets.push_back(target);
    // Apply current layout immediately so the new target is in sync.
    target->ApplyLayout(m_current);
}

void ViewportLayoutManager::RegisterOverlayAdapter(std::unique_ptr<OverlayLayoutAdapter> adapter) {
    if (!adapter)
        return;
    adapter->Sync(m_current);
    m_adapters.push_back(std::move(adapter));
}

void ViewportLayoutManager::ApplyLayout(ui::ViewportLayoutType type) {
    m_current = MakeLayoutDefinition(type);
    _Broadcast(m_current);
    emit layoutChanged(m_current);
}

void ViewportLayoutManager::ApplySlotAssignment(const std::vector<int>& slotPlanes) {
    m_current = MakeLayoutDefinition(m_current.type, slotPlanes);
    _Broadcast(m_current);
    emit layoutChanged(m_current);
}

void ViewportLayoutManager::SetOrientationMarkersEnabled(bool enabled) {
    for (auto& adapter : m_adapters)
        adapter->SetAllOrientationMarkersUserEnabled(enabled);
}

void ViewportLayoutManager::SetCornerAnnotationsEnabled(bool enabled) {
    for (auto& adapter : m_adapters)
        adapter->SetAllCornerAnnotationsUserEnabled(enabled);
}

void ViewportLayoutManager::_Broadcast(const ViewportLayoutDefinition& def) {
    for (auto* target : m_targets)
        target->ApplyLayout(def);
    for (auto& adapter : m_adapters)
        adapter->Sync(def);
}

}  // namespace ui
