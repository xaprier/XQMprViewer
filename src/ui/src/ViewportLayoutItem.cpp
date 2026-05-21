#include "ui/ViewportLayoutItem.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

namespace ui {

namespace {
constexpr int kItemSize = 44;
constexpr int kPadding = 4;

static const char* kPlaneNames[3] = {"Axial", "Coronal", "Sagittal"};
}  // namespace

ViewportLayoutItem::ViewportLayoutItem(ViewportLayoutType type, QWidget* parent)
    : QAbstractButton(parent),
      m_type(type),
      m_def(MakeLayoutDefinition(type)) {
    setCheckable(true);
    setToolTip(QString::fromStdString(m_def.label));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ViewportLayoutItem::SetDefinition(const ViewportLayoutDefinition& def) {
    m_def = def;
    update();
}

QSize ViewportLayoutItem::sizeHint() const {
    return {kItemSize, kItemSize};
}

QSize ViewportLayoutItem::minimumSizeHint() const {
    return sizeHint();
}

void ViewportLayoutItem::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();

    // Background
    const bool selected = isChecked();
    const bool hovered = underMouse();

    QColor bgColor = selected  ? QColor(60, 120, 215)
                     : hovered ? QColor(80, 80, 90)
                               : QColor(50, 50, 58);

    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawRoundedRect(rect(), 4, 4);

    // Border
    QColor borderColor = selected ? QColor(100, 160, 255) : QColor(90, 90, 100);
    p.setPen(QPen(borderColor, selected ? 2 : 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);

    // Draw pane diagram
    for (const auto& pane : m_def.panes) {
        _drawPane(p, pane, w, h);
    }
}

void ViewportLayoutItem::_drawPane(QPainter& p, const ViewportPaneConfig& pane,
                                   int w, int h) const {
    const auto& vp = pane.viewport;
    const int drawW = w - 2 * kPadding;
    const int drawH = h - 2 * kPadding;

    // VTK uses bottom-left origin; Qt uses top-left — invert Y
    const int x = kPadding + static_cast<int>(vp.xMin * drawW);
    const int y = kPadding + static_cast<int>((1.0 - vp.yMax) * drawH);
    const int pw = static_cast<int>((vp.xMax - vp.xMin) * drawW);
    const int ph = static_cast<int>((vp.yMax - vp.yMin) * drawH);

    if (pw <= 0 || ph <= 0)
        return;

    static const QColor kPaneColors[3] = {
        QColor(70, 130, 180),   // Axial — steel blue
        QColor(100, 160, 100),  // Coronal — muted green
        QColor(180, 120, 70),   // Sagittal — warm amber
    };

    const int idx = pane.planeIndex;
    QColor fill = (idx >= 0 && idx < 3) ? kPaneColors[idx] : QColor(100, 100, 100);

    p.setPen(QPen(Qt::black, 1));
    p.setBrush(fill.darker(isChecked() ? 110 : 130));
    p.drawRect(x + 1, y + 1, pw - 2, ph - 2);
}

void ViewportLayoutItem::contextMenuEvent(QContextMenuEvent* event) {
    // Only show assignment menu for multi-pane layouts.
    if (m_def.panes.size() <= 1) {
        QAbstractButton::contextMenuEvent(event);
        return;
    }
    _showAssignmentMenu();
    event->accept();
}

void ViewportLayoutItem::_showAssignmentMenu() {
    const int slotCount = static_cast<int>(m_def.panes.size());

    // Build current slot→plane mapping from the definition.
    std::vector<int> current(slotCount);
    for (const auto& pane : m_def.panes) {
        if (pane.slotIndex >= 0 && pane.slotIndex < slotCount)
            current[pane.slotIndex] = pane.planeIndex;
    }

    QMenu menu(this);
    menu.setTitle(tr("Assign Planes"));

    std::vector<int> assignment = current;

    struct SlotMenu {
        QMenu* menu;
        QAction* actions[3];
    };
    std::vector<SlotMenu> slotMenus(slotCount);

    for (int s = 0; s < slotCount; ++s) {
        SlotMenu& sm = slotMenus[s];
        sm.menu = menu.addMenu(tr("Slot %1 ").arg(s + 1));

        for (int p = 0; p < 3; ++p) {
            sm.actions[p] = sm.menu->addAction(tr(kPlaneNames[p]));
            sm.actions[p]->setCheckable(true);
            sm.actions[p]->setChecked(assignment[s] == p);
        }
    }

    // Wire up actions: selecting plane p for slot s swaps with the slot that
    // already holds p, so every plane appears in exactly one slot.
    for (int s = 0; s < slotCount; ++s) {
        for (int p = 0; p < 3; ++p) {
            connect(slotMenus[s].actions[p], &QAction::triggered, this,
                    [this, s, p, slotCount, slotMenus, assignment]() mutable {
                        // Find which slot currently holds plane p and swap.
                        for (int other = 0; other < slotCount; ++other) {
                            if (other != s && assignment[other] == p) {
                                assignment[other] = assignment[s];  // give s's old plane to other
                                break;
                            }
                        }
                        assignment[s] = p;
                        emit slotAssignmentRequested(assignment);
                    });
        }
    }

    menu.exec(QCursor::pos());
}

}  // namespace ui
