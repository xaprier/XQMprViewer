#ifndef VIEWPORTLAYOUTSELECTOR_HPP
#define VIEWPORTLAYOUTSELECTOR_HPP

#include <QWidget>
#include <vector>

#include "ui/ViewportLayoutTypes.hpp"

class QButtonGroup;

namespace ui {

class ViewportLayoutItem;

/**
 * @brief Toolbar-like panel that displays one ViewportLayoutItem per layout type.
 *
 * Maintains an exclusive selection (QButtonGroup) and emits layoutSelected()
 * whenever the active layout changes. The widget is pure UI — it knows nothing
 * about renderers or controllers.
 *
 * Embed it above or beside the VTK widget in ViewportView and connect its
 * layoutSelected() signal to ViewportLayoutManager::ApplyLayout().
 */
class ViewportLayoutSelector : public QWidget {
    Q_OBJECT

  public:
    explicit ViewportLayoutSelector(QWidget* parent = nullptr);

    /** @brief Programmatically activates the button for @p type without emitting layoutSelected(). */
    void SetActiveLayout(ViewportLayoutType type);

    /** @brief Returns the currently checked layout type. */
    ViewportLayoutType ActiveLayout() const;

    /** @brief Update the definition of the active button (e.g. after slot reassignment). */
    void UpdateActiveDefinition(const ViewportLayoutDefinition& def);

  Q_SIGNALS:
    /** @brief Emitted when the user clicks a different layout button. */
    void layoutSelected(ui::ViewportLayoutType type);

    /**
     * @brief Emitted when the user picks a new slot→plane assignment via
     *        right-click on a layout button.
     */
    void slotAssignmentRequested(const std::vector<int>& slotPlanes);

  private:
    QButtonGroup* m_group{nullptr};
    std::vector<ViewportLayoutItem*> m_items;
};

}  // namespace ui

#endif  // VIEWPORTLAYOUTSELECTOR_HPP
