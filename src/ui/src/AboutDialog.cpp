#include "ui/AboutDialog.hpp"

#include <QDialogButtonBox>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "app/Application.hpp"
#include "app/Version.hpp"

namespace ui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("About %1").arg(app::qAppInstance()->applicationName()));
    setFixedWidth(480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    _BuildUi();
    adjustSize();
}

void AboutDialog::_BuildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(0);

    // ── App name ─────────────────────────────────────────────────────────────
    auto* nameLabel = new QLabel(app::qAppInstance()->applicationDisplayName(), this);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(20);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    root->addWidget(nameLabel);
    root->addSpacing(4);

    // ── Version ───────────────────────────────────────────────────────────────
    auto* versionLabel = new QLabel(tr("Version %1").arg(app::qAppInstance()->applicationVersion()), this);
    QFont versionFont = versionLabel->font();
    versionFont.setPointSize(10);
    versionLabel->setFont(versionFont);
    root->addWidget(versionLabel);
    root->addSpacing(16);

    // ── Separator ─────────────────────────────────────────────────────────────
    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    root->addWidget(sep1);
    root->addSpacing(14);

    // ── Description ───────────────────────────────────────────────────────────
    auto* descLabel = new QLabel(QString::fromUtf8(app::Version::kDescription), this);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QFont descFont = descLabel->font();
    descFont.setPointSize(10);
    descLabel->setFont(descFont);
    root->addWidget(descLabel);
    root->addSpacing(18);

    // ── Details grid ──────────────────────────────────────────────────────────
    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(8);
    grid->setColumnStretch(1, 1);

    auto makeCaptionLabel = [this](const QString& text) {
        auto* lbl = new QLabel(text, this);
        QFont f = lbl->font();
        f.setBold(true);
        f.setPointSize(9);
        lbl->setFont(f);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return lbl;
    };

    auto makeValueLabel = [this](const QString& text) {
        auto* lbl = new QLabel(text, this);
        lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
        QFont f = lbl->font();
        f.setPointSize(9);
        lbl->setFont(f);
        return lbl;
    };

    auto makeLinkLabel = [this](const QString& url) {
        auto* lbl = new QLabel(
            QString("<a href=\"%1\" style=\"color: palette(highlight);\">%1</a>").arg(url), this);
        lbl->setOpenExternalLinks(true);
        lbl->setTextFormat(Qt::RichText);
        QFont f = lbl->font();
        f.setPointSize(9);
        lbl->setFont(f);
        return lbl;
    };

    int row = 0;
    grid->addWidget(makeCaptionLabel(tr("Organization")), row, 0);
    grid->addWidget(makeValueLabel(app::qAppInstance()->organizationName()), row, 1);
    ++row;

    grid->addWidget(makeCaptionLabel(tr("Homepage")), row, 0);
    grid->addWidget(makeLinkLabel(QString::fromUtf8(app::Version::kHomepageUrl)), row, 1);
    ++row;

    grid->addWidget(makeCaptionLabel(tr("Documentation")), row, 0);
    grid->addWidget(makeLinkLabel(app::qAppInstance()->documentation()), row, 1);
    ++row;

    root->addLayout(grid);
    root->addSpacing(18);

    // ── Separator ─────────────────────────────────────────────────────────────
    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    root->addWidget(sep2);
    root->addSpacing(14);

    // ── Copyright + close button ──────────────────────────────────────────────
    auto* bottomRow = new QHBoxLayout();

    auto* copyrightLabel = new QLabel(
        tr("© %1 — %2").arg(app::qAppInstance()->organizationName(), app::qAppInstance()->applicationName()), this);
    QFont copyrightFont = copyrightLabel->font();
    copyrightFont.setPointSize(8);
    copyrightLabel->setFont(copyrightFont);

    auto* closeBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(closeBox, &QDialogButtonBox::rejected, this, &QDialog::accept);

    bottomRow->addWidget(copyrightLabel, 1);
    bottomRow->addWidget(closeBox);
    root->addLayout(bottomRow);
}

}  // namespace ui
