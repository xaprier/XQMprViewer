#include "overlays/CornerAnnotationOverlay.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageViewer2.h>
#include <vtkResliceImageViewer.h>

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStringList>

namespace overlays {

CornerAnnotationOverlay::CornerAnnotationOverlay(QVTKOpenGLNativeWidget* host,
                                                 const QString& viewName)
    : QWidget(host), m_host(host), m_viewName(viewName) {
    m_position = OverlayPosition::BottomRight;
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    host->installEventFilter(this);

    connect(host, &QVTKOpenGLNativeWidget::frameSwapped,
            this, &CornerAnnotationOverlay::_OnFrameSwapped);

    resize(host->size().isEmpty() ? QSize(1, 1) : host->size());
    raise();
    show();
}

CornerAnnotationOverlay::~CornerAnnotationOverlay() {
    if (m_host)
        m_host->removeEventFilter(this);
}

// ── IOverlay ──────────────────────────────────────────────────────────────────

void CornerAnnotationOverlay::SetEnabled(bool enabled) {
    if (m_enabled == enabled)
        return;
    setVisible(enabled);
    m_enabled = enabled;
}

void CornerAnnotationOverlay::SetPosition(OverlayPosition pos) {
    m_position = pos;
    _Reposition();
}

// ── Data source ───────────────────────────────────────────────────────────────

void CornerAnnotationOverlay::SetViewer(vtkResliceImageViewer* viewer) {
    m_viewer = viewer;
    update();
}

void CornerAnnotationOverlay::SetViewName(const QString& name) {
    if (m_viewName == name)
        return;
    m_viewName = name;
    update();
}

// ── Appearance ────────────────────────────────────────────────────────────────

void CornerAnnotationOverlay::SetTextColor(const QColor& color) {
    if (m_textColor == color)
        return;
    m_textColor = color;
    update();
}

void CornerAnnotationOverlay::SetFontSize(int pt) {
    if (m_fontSize == pt)
        return;
    m_fontSize = pt;
    _InvalidateFontCache();
    update();
}

void CornerAnnotationOverlay::SetMargin(int px) {
    if (m_margin == px)
        return;
    m_margin = px;
    _Reposition();
}

// ── Line visibility ───────────────────────────────────────────────────────────

void CornerAnnotationOverlay::SetShowSliceInfo(bool show) {
    if (m_showSliceInfo == show)
        return;
    m_showSliceInfo = show;
    update();
}

void CornerAnnotationOverlay::SetShowWindowLevel(bool show) {
    if (m_showWindowLevel == show)
        return;
    m_showWindowLevel = show;
    update();
}

void CornerAnnotationOverlay::SetShowSpacing(bool show) {
    if (m_showSpacing == show)
        return;
    m_showSpacing = show;
    update();
}

void CornerAnnotationOverlay::SetShowViewName(bool show) {
    if (m_showViewName == show)
        return;
    m_showViewName = show;
    update();
}

// ── Shared-viewport clipping ──────────────────────────────────────────────────

void CornerAnnotationOverlay::SetViewportFraction(double xMin, double xMax) {
    if (m_vpXMin == xMin && m_vpXMax == xMax)
        return;
    m_vpXMin = xMin;
    m_vpXMax = xMax;
    _Reposition();
}

// ── Events ────────────────────────────────────────────────────────────────────

bool CornerAnnotationOverlay::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_host && event->type() == QEvent::Resize) {
        resize(static_cast<QResizeEvent*>(event)->size());
        _Reposition();
    }
    return QWidget::eventFilter(watched, event);
}

void CornerAnnotationOverlay::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    _Reposition();
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void CornerAnnotationOverlay::_OnFrameSwapped() {
    if (m_enabled)
        update();
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void CornerAnnotationOverlay::paintEvent(QPaintEvent* /*event*/) {
    if (!m_enabled)
        return;

    const QString text = _BuildText();
    if (text.isEmpty())
        return;

    if (m_fontDirty) {
        m_cachedFont = font();
        m_cachedFont.setPointSize(m_fontSize);
        m_cachedFont.setBold(true);
        m_cachedMetrics = QFontMetrics(m_cachedFont);
        m_fontDirty = false;
    }

    const QStringList lines = text.split('\n');
    const QFontMetrics& fm  = m_cachedMetrics;
    const int lineH = fm.height();
    int maxW = 0;
    for (const auto& line : lines)
        maxW = std::max(maxW, fm.horizontalAdvance(line));
    const int blockH = lines.size() * lineH;

    // Viewport sub-region
    const int totalW  = width();
    const int totalH  = height();
    const int vpLeft  = static_cast<int>(m_vpXMin * totalW);
    const int vpRight = static_cast<int>(m_vpXMax * totalW);
    const int vpW     = vpRight - vpLeft;
    if (vpW <= 0 || totalH <= 0)
        return;

    // Block origin from position (same logic as FPSOverlay::_Reposition)
    int bx = 0, by = 0;
    switch (m_position) {
        case OverlayPosition::TopLeft:
            bx = vpLeft  + m_margin;
            by = m_margin;
            break;
        case OverlayPosition::TopRight:
            bx = vpRight - maxW - m_margin;
            by = m_margin;
            break;
        case OverlayPosition::TopCenter:
            bx = vpLeft  + (vpW - maxW) / 2;
            by = m_margin;
            break;
        case OverlayPosition::BottomLeft:
            bx = vpLeft  + m_margin;
            by = totalH - blockH - m_margin;
            break;
        case OverlayPosition::BottomRight:
            bx = vpRight - maxW - m_margin;
            by = totalH - blockH - m_margin;
            break;
        case OverlayPosition::BottomCenter:
            bx = vpLeft  + (vpW - maxW) / 2;
            by = totalH - blockH - m_margin;
            break;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setClipRect(vpLeft, 0, vpW, totalH);
    p.setFont(m_cachedFont);

    // Semi-transparent background box — same style as FPSOverlay.
    const QRect bgRect(bx - m_padding,
                       by - m_padding,
                       maxW + 2 * m_padding,
                       blockH + 2 * m_padding);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 120));
    p.drawRoundedRect(bgRect, 3, 3);

    for (int li = 0; li < lines.size(); ++li) {
        const int x = bx;
        const int y = by + li * lineH + fm.ascent();
        p.setPen(Qt::black);
        p.drawText(x + 1, y + 1, lines[li]);
        p.setPen(m_textColor);
        p.drawText(x,     y,     lines[li]);
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

QString CornerAnnotationOverlay::_BuildText() const {
    QStringList lines;

    if (m_showViewName && !m_viewName.isEmpty())
        lines << m_viewName;

    if (m_viewer) {
        if (m_showSliceInfo) {
            const int cur   = m_viewer->GetSlice();
            const int minS  = m_viewer->GetSliceMin();
            const int maxS  = m_viewer->GetSliceMax();
            const int count = maxS - minS + 1;
            lines << tr("Slice: %1 / %2").arg(cur - minS + 1).arg(count);
        }

        if (m_showWindowLevel) {
            // Read from ImageActor's property — these are the values actually
            // applied to the rendered image, set by SliceController::SetupPipeline().
            if (auto* actor = m_viewer->GetImageActor()) {
                if (auto* prop = actor->GetProperty()) {
                    const double ww = prop->GetColorWindow();
                    const double wl = prop->GetColorLevel();
                    lines << tr("W: %1  L: %2")
                                 .arg(ww, 0, 'f', 0)
                                 .arg(wl, 0, 'f', 0);
                }
            }
        }

        if (m_showSpacing) {
            if (auto* img = m_viewer->GetInput()) {
                double sp[3];
                img->GetSpacing(sp);

                // Show only the through-plane (slice-normal) spacing for each
                // orientation — the axis perpendicular to the viewed plane.
                // Orientation enum: YZ=0 (Sagittal,X=sp[0]),
                //                   XZ=1 (Coronal, Y=sp[1]),
                //                   XY=2 (Axial,   Z=sp[2]).
                const int orientation = m_viewer->GetSliceOrientation();
                static const char* kAxisLabel[3] = {"X", "Y", "Z"};
                const double throughPlaneSpacing = sp[orientation];
                const char*  axisLabel           = kAxisLabel[orientation];

                lines << tr("Sp(%1): %2 mm")
                             .arg(QLatin1String(axisLabel))
                             .arg(throughPlaneSpacing, 0, 'f', 2);
            }
        }
    }

    return lines.join('\n');
}

void CornerAnnotationOverlay::_Reposition() {
    // The overlay covers the full host — repositioning is handled in paintEvent
    // via m_position, matching FPSOverlay's approach. Nothing to move here since
    // this widget always fills the host (like OrientationMarkerOverlay).
    update();
}

void CornerAnnotationOverlay::_InvalidateFontCache() {
    m_fontDirty = true;
}

}  // namespace overlays
