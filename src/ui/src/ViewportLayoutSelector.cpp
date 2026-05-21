#include "ui/ViewportLayoutSelector.hpp"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "ui/ViewportLayoutItem.hpp"

namespace ui {

namespace {

const ViewportLayoutType kAllLayouts[] = {
    ViewportLayoutType::SingleAxial,
    ViewportLayoutType::SingleCoronal,
    ViewportLayoutType::SingleSagittal,
    ViewportLayoutType::HorizontalSplit,
    ViewportLayoutType::VerticalSplit,
    ViewportLayoutType::TwoPlusOne,
    ViewportLayoutType::TwoPlusOneReversed,
};

// Separator widget
QFrame* makeSeparator() {
    auto* sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    return sep;
}

}  // namespace

ViewportLayoutSelector::ViewportLayoutSelector(QWidget* parent)
    : QWidget(parent) {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(4, 2, 4, 2);
    outerLayout->setSpacing(2);

    auto* label = new QLabel("Layout", this);
    label->setAlignment(Qt::AlignCenter);
    QFont f = label->font();
    f.setPointSize(8);
    label->setFont(f);
    outerLayout->addWidget(label);

    auto* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(3);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Build groups: Singles | Splits | Multi
    const int kSingleCount = 3;  // Axial, Coronal, Sagittal
    for (int i = 0; i < static_cast<int>(std::size(kAllLayouts)); ++i) {
        if (i == kSingleCount)
            row->addWidget(makeSeparator());

        auto* item = new ViewportLayoutItem(kAllLayouts[i], this);
        m_group->addButton(item, i);
        row->addWidget(item);
        m_items.push_back(item);
    }

    outerLayout->addLayout(row);

    // Default: HorizontalSplit selected
    SetActiveLayout(ViewportLayoutType::HorizontalSplit);

    connect(m_group, &QButtonGroup::idClicked, this, [this](int id) {
        if (id >= 0 && id < static_cast<int>(m_items.size())) {
            emit layoutSelected(m_items[id]->layoutType());
        }
    });

    for (auto* item : m_items) {
        connect(item, &ViewportLayoutItem::slotAssignmentRequested,
                this, &ViewportLayoutSelector::slotAssignmentRequested);
    }

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void ViewportLayoutSelector::SetActiveLayout(ViewportLayoutType type) {
    for (auto* item : m_items) {
        const bool match = (item->layoutType() == type);
        item->setChecked(match);
    }
}

ViewportLayoutType ViewportLayoutSelector::ActiveLayout() const {
    for (const auto* item : m_items) {
        if (item->isChecked())
            return item->layoutType();
    }
    return ViewportLayoutType::HorizontalSplit;
}

void ViewportLayoutSelector::UpdateActiveDefinition(const ViewportLayoutDefinition& def) {
    for (auto* item : m_items) {
        if (item->layoutType() == def.type) {
            item->SetDefinition(def);
            break;
        }
    }
}

}  // namespace ui
