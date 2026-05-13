#include "overlays/OrientationMarkerOverlay.hpp"

#include <QVTKOpenGLNativeWidget.h>

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

namespace overlays {

// Label table indexed by [longLabels][orientation][edge: L, R, T, B].
static constexpr const char* kLabels[2][3][4] = {
    // short
    {
        {"L", "R", "A", "P"},  // Axial
        {"L", "R", "S", "I"},  // Coronal
        {"A", "P", "S", "I"},  // Sagittal
    },
    // long
    {
        {"Left", "Right", "Anterior",  "Posterior"},  // Axial
        {"Left", "Right", "Superior",  "Inferior"},   // Coronal
        {"Anterior", "Posterior", "Superior", "Inferior"},  // Sagittal
    },
};

constexpr OrientationMarkerOverlay::SliceOrientation
    OrientationMarkerOverlay::kSliceOrientations[3];

OrientationMarkerOverlay::OrientationMarkerOverlay(QVTKOpenGLNativeWidget* host,
                                                   SliceOrientation orientation)
    : QWidget(host), m_host(host), m_orientation(orientation) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    host->installEventFilter(this);

    resize(host->size().isEmpty() ? QSize(1, 1) : host->size());
    raise();
    show();
}

OrientationMarkerOverlay::~OrientationMarkerOverlay() {
    if (m_host)
        m_host->removeEventFilter(this);
}

// ── IOverlay ─────────────────────────────────────────────────────────────────

void OrientationMarkerOverlay::SetEnabled(bool enabled) {
    if (m_enabled == enabled)
        return;
    setVisible(enabled);
    m_enabled = enabled;
}

void OrientationMarkerOverlay::SetPosition(OverlayPosition /*pos*/) {}

// ── Appearance ────────────────────────────────────────────────────────────────

void OrientationMarkerOverlay::SetTextColor(const QColor& color) {
    if (m_textColor == color)
        return;
    m_textColor = color;
    update();
}

void OrientationMarkerOverlay::SetFontSize(int pt) {
    if (m_fontSize == pt)
        return;
    m_fontSize = pt;
    _InvalidateFontCache();
    update();
}

void OrientationMarkerOverlay::SetLongLabels(bool longLabels) {
    if (m_longLabels == longLabels)
        return;
    m_longLabels = longLabels;
    update();
}

// ── Slice plane ───────────────────────────────────────────────────────────────

void OrientationMarkerOverlay::SetSliceOrientation(SliceOrientation orientation) {
    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    update();
}

void OrientationMarkerOverlay::SetViewportFraction(double xMin, double xMax) {
    if (m_vpXMin == xMin && m_vpXMax == xMax)
        return;
    m_vpXMin = xMin;
    m_vpXMax = xMax;
    update();
}

// ── Event filter ─────────────────────────────────────────────────────────────

bool OrientationMarkerOverlay::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_host && event->type() == QEvent::Resize)
        resize(static_cast<QResizeEvent*>(event)->size());
    return QWidget::eventFilter(watched, event);
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void OrientationMarkerOverlay::paintEvent(QPaintEvent* /*event*/) {
    if (!m_enabled)
        return;

    const int totalW = width();
    const int totalH = height();
    const int vpLeft  = static_cast<int>(m_vpXMin * totalW);
    const int vpRight = static_cast<int>(m_vpXMax * totalW);
    const int vpW     = vpRight - vpLeft;

    if (vpW <= 0 || totalH <= 0)
        return;

    if (m_fontDirty) {
        m_cachedFont = font();
        m_cachedFont.setPointSize(m_fontSize);
        m_cachedFont.setBold(true);
        m_cachedMetrics = QFontMetrics(m_cachedFont);
        m_fontDirty = false;
    }

    const Labels labels = _ResolvedLabels();

    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setClipRect(vpLeft, 0, vpW, totalH);
    p.setFont(m_cachedFont);

    _DrawLabel(p, m_cachedMetrics, labels.left,   Qt::AlignLeft,   vpLeft, vpRight, vpW, totalH);
    _DrawLabel(p, m_cachedMetrics, labels.right,  Qt::AlignRight,  vpLeft, vpRight, vpW, totalH);
    _DrawLabel(p, m_cachedMetrics, labels.top,    Qt::AlignTop,    vpLeft, vpRight, vpW, totalH);
    _DrawLabel(p, m_cachedMetrics, labels.bottom, Qt::AlignBottom, vpLeft, vpRight, vpW, totalH);
}

void OrientationMarkerOverlay::_DrawLabel(QPainter& p, const QFontMetrics& fm,
                                          const QString& text, Qt::Alignment align,
                                          int vpLeft, int vpRight, int vpW, int totalH) const {
    if (text.isEmpty())
        return;

    const int textW = fm.horizontalAdvance(text);
    const int textH = fm.height();

    int x = 0, y = 0;
    if (align & Qt::AlignLeft) {
        x = vpLeft + m_margin;
        y = (totalH - textH) / 2;
    } else if (align & Qt::AlignRight) {
        x = vpRight - textW - m_margin;
        y = (totalH - textH) / 2;
    } else if (align & Qt::AlignTop) {
        x = vpLeft + (vpW - textW) / 2;
        y = m_margin;
    } else if (align & Qt::AlignBottom) {
        x = vpLeft + (vpW - textW) / 2;
        y = totalH - textH - m_margin;
    } else {
        return;
    }

    p.setPen(Qt::black);
    p.drawText(x + 1, y + 1 + fm.ascent(), text);
    p.setPen(m_textColor);
    p.drawText(x, y + fm.ascent(), text);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

OrientationMarkerOverlay::Labels OrientationMarkerOverlay::_ResolvedLabels() const {
    const int li = m_longLabels ? 1 : 0;
    const int oi = static_cast<int>(m_orientation);
    return {
        kLabels[li][oi][0],
        kLabels[li][oi][1],
        kLabels[li][oi][2],
        kLabels[li][oi][3],
    };
}

void OrientationMarkerOverlay::_InvalidateFontCache() {
    m_fontDirty = true;
}

}  // namespace overlays
