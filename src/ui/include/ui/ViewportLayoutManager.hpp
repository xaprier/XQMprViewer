#ifndef VIEWPORTLAYOUTMANAGER_HPP
#define VIEWPORTLAYOUTMANAGER_HPP

#include <QObject>
#include <memory>
#include <vector>

#include "ui/OverlayLayoutAdapter.hpp"
#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

class ILayoutTarget;

/**
 * @brief Global orchestrator for runtime layout switching.
 *
 * Owned by MainWindow. Drives layout changes across both views:
 *
 *  - Calls ILayoutTarget::ApplyLayout() on every registered view.
 *  - Calls OverlayLayoutAdapter::Sync() on every registered adapter.
 *  - Keeps the current layout definition so new views can query it.
 *
 * Decouples the selector widget (ViewportLayoutSelector / ViewportLayoutPanel)
 * from both the view layer and the controller layer.
 *
 * Overlay enable/disable events from the ControllerPanel must also pass
 * through this manager so it can forward them to the correct adapters.
 */
class ViewportLayoutManager : public QObject {
    Q_OBJECT

  public:
    explicit ViewportLayoutManager(QObject* parent = nullptr);
    ~ViewportLayoutManager() override;

    // ── Targets ───────────────────────────────────────────────────────────────

    /**
     * @brief Register a view as a layout target.
     *
     * The caller retains ownership. ApplyLayout() is called immediately with
     * the current layout when a new target registers.
     */
    void RegisterTarget(ILayoutTarget* target);

    // ── Overlay adapters ──────────────────────────────────────────────────────

    /**
     * @brief Register an overlay adapter that will be synced on every layout change.
     *
     * Ownership is transferred to the manager.
     */
    void RegisterOverlayAdapter(std::unique_ptr<OverlayLayoutAdapter> adapter);

    // ── Overlay user-enable forwarding ────────────────────────────────────────

    /**
     * @brief Forward the global orientation-marker user-enable state to all adapters.
     *
     * Called by MainWindow when ControllerPanel emits OrientationMarkerEnableChanged.
     */
    void SetOrientationMarkersEnabled(bool enabled);

    /**
     * @brief Forward the global corner-annotation user-enable state to all adapters.
     *
     * Called by MainWindow when ControllerPanel emits CornerAnnotationEnableChanged.
     */
    void SetCornerAnnotationsEnabled(bool enabled);

    // ── Layout ────────────────────────────────────────────────────────────────

    const ViewportLayoutDefinition& CurrentLayout() const { return m_current; }

  public slots:
    /** @brief Apply a new layout to all registered targets and adapters. */
    void ApplyLayout(ui::ViewportLayoutType type);

    /**
     * @brief Re-apply current layout with a custom slot→plane assignment.
     *
     * slotPlanes[i] = plane index (0=Axial,1=Coronal,2=Sagittal) for slot i.
     * The geometry (viewport rects) stay the same; only which plane renders
     * in each slot changes.
     */
    void ApplySlotAssignment(const std::vector<int>& slotPlanes);

  Q_SIGNALS:
    void layoutChanged(const ui::ViewportLayoutDefinition& def);

  private:
    void _Broadcast(const ViewportLayoutDefinition& def);

    ViewportLayoutDefinition m_current{MakeLayoutDefinition(ViewportLayoutType::HorizontalSplit)};
    std::vector<ILayoutTarget*> m_targets;
    std::vector<std::unique_ptr<OverlayLayoutAdapter>> m_adapters;
};

}  // namespace ui

#endif  // VIEWPORTLAYOUTMANAGER_HPP
