#ifndef VIEWPORTLAYOUTITEM_HPP
#define VIEWPORTLAYOUTITEM_HPP

#include <QAbstractButton>
#include <vector>

#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

/**
 * @brief Clickable button that visually represents a single viewport layout.
 *
 * Paints a miniature diagram of the layout's pane arrangement using the rects
 * stored in the corresponding ViewportLayoutDefinition. Supports a selected
 * state with highlight coloring.
 *
 * Right-clicking on a multi-pane layout button opens a context menu that lets
 * the user reassign which plane (Axial/Coronal/Sagittal) appears in each slot.
 * The slotAssignmentRequested signal carries the desired slot→plane mapping.
 *
 * Designed to be placed inside a ViewportLayoutSelector.
 */
class ViewportLayoutItem : public QAbstractButton {
    Q_OBJECT

  public:
    explicit ViewportLayoutItem(ViewportLayoutType type, QWidget* parent = nullptr);

    /** @brief Returns the layout type this button represents. */
    ViewportLayoutType layoutType() const { return m_type; }

    /** @brief Updates the displayed pane diagram (e.g. after a slot assignment change). */
    void SetDefinition(const ViewportLayoutDefinition& def);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  Q_SIGNALS:
    /**
     * @brief Emitted when the user picks a new slot→plane assignment via the
     *        right-click context menu.
     *
     * slotPlanes[i] is the plane index (0=Axial,1=Coronal,2=Sagittal) for slot i.
     */
    void slotAssignmentRequested(const std::vector<int>& slotPlanes);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

  private:
    void _drawPane(class QPainter& p, const ViewportPaneConfig& pane,
                   int w, int h) const;
    void _showAssignmentMenu();

    ViewportLayoutType m_type;
    ViewportLayoutDefinition m_def;
};

}  // namespace ui

#endif  // VIEWPORTLAYOUTITEM_HPP
