#include "overlays/FPSOverlay.hpp"

#include <QVTKOpenGLNativeWidget.h>

#include <QFont>
#include <QPalette>
#include <QResizeEvent>
#include <QString>

namespace overlays {

FPSOverlay::FPSOverlay(QVTKOpenGLNativeWidget* host)
    : QLabel(host), m_host(host) {
    // Transparent background so only the text is visible.
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, Qt::yellow);
    setPalette(pal);

    QFont f = font();
    f.setPointSize(10);
    f.setBold(true);
    setFont(f);

    setStyleSheet("QLabel { background: rgba(0,0,0,120); padding: 4px; border-radius: 3px; }");
    setText(tr("FPS: --\nFrame: -- ms"));
    adjustSize();

    // Drive timing from actual OpenGL frame swaps.
    connect(host, &QVTKOpenGLNativeWidget::frameSwapped,
            this, &FPSOverlay::_OnFrameSwapped);

    m_frameTimer.start();
    raise();  // ensure we're on top of the VTK surface
    show();
}

void FPSOverlay::SetEnabled(bool enabled) {
    setVisible(enabled);
    m_enabled = enabled;
}

void FPSOverlay::SetTextColor(const QColor& color) {
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, color);
    setPalette(pal);
}

void FPSOverlay::SetFontSize(int pt) {
    QFont f = font();
    f.setPointSize(pt);
    setFont(f);
    adjustSize();
    _Reposition();
}

void FPSOverlay::SetPosition(OverlayPosition pos) {
    m_position = pos;
    _Reposition();
}

void FPSOverlay::SetMargin(int px) {
    m_margin = px;
    _Reposition();
}

void FPSOverlay::resizeEvent(QResizeEvent* event) {
    QLabel::resizeEvent(event);
    // Parent widget was resized — reposition relative to the new size.
    if (m_host)
        _Reposition();
}

void FPSOverlay::SetAveragingWindow(int seconds) {
    m_windowNs = static_cast<qint64>(qBound(1, seconds, 60)) * 1'000'000'000LL;
    m_frameTimestamps.clear();
    m_lastFrameNs = 0;
}

void FPSOverlay::_OnFrameSwapped() {
    const qint64 nowNs = m_frameTimer.nsecsElapsed();

    if (m_lastFrameNs > 0) {
        m_frameTimestamps.push_back(nowNs);

        // Drop timestamps older than the averaging window.
        const qint64 cutoff = nowNs - m_windowNs;
        while (!m_frameTimestamps.empty() && m_frameTimestamps.front() < cutoff)
            m_frameTimestamps.pop_front();

        if (m_frameTimestamps.size() >= 2) {
            const qint64 spanNs = m_frameTimestamps.back() - m_frameTimestamps.front();
            const int frames = static_cast<int>(m_frameTimestamps.size()) - 1;
            const double avgFrameMs = static_cast<double>(spanNs) / frames / 1'000'000.0;
            const double avgFps = frames * 1'000'000'000.0 / static_cast<double>(spanNs);
            _UpdateText(avgFrameMs, avgFps);
        }
    }

    m_lastFrameNs = nowNs;
}

void FPSOverlay::_UpdateText(double frameMs, double fps) {
    setText(tr("FPS: %1\nFrame: %2 ms")
                .arg(fps, 0, 'f', 1)
                .arg(frameMs, 0, 'f', 2));
    adjustSize();
    _Reposition();
}

void FPSOverlay::_Reposition() {
    if (!m_host)
        return;

    const QSize parentSz = m_host->size();
    const QSize mySz = size();
    int x = 0, y = 0;

    switch (m_position) {
        case OverlayPosition::TopLeft:
            x = m_margin;
            y = m_margin;
            break;
        case OverlayPosition::TopRight:
            x = parentSz.width() - mySz.width() - m_margin;
            y = m_margin;
            break;
        case OverlayPosition::TopCenter:
            x = (parentSz.width() - mySz.width()) / 2;
            y = m_margin;
            break;
        case OverlayPosition::BottomLeft:
            x = m_margin;
            y = parentSz.height() - mySz.height() - m_margin;
            break;
        case OverlayPosition::BottomRight:
            x = parentSz.width() - mySz.width() - m_margin;
            y = parentSz.height() - mySz.height() - m_margin;
            break;
        case OverlayPosition::BottomCenter:
            x = (parentSz.width() - mySz.width()) / 2;
            y = parentSz.height() - mySz.height() - m_margin;
            break;
    }

    move(x, y);
}

}  // namespace overlays
