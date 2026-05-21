#ifndef FPSOVERLAY_HPP
#define FPSOVERLAY_HPP

#include <deque>

#include <QColor>
#include <QElapsedTimer>
#include <QLabel>

#include "overlays/IOverlay.hpp"

class QVTKOpenGLNativeWidget;

namespace overlays {

/**
 * @brief Qt-native FPS overlay that lives as a child widget on top of a
 *        QVTKOpenGLNativeWidget and tracks real OpenGL frame swap time.
 *
 * Timing is driven by the host widget's frameSwapped() signal, which fires
 * after each OpenGL buffer swap — this captures the full GPU+CPU round-trip
 * per frame rather than VTK's internal render timer.
 *
 * FPS and frame-time values are averaged over a configurable sliding window
 * (default 1 s, adjustable via SetAveragingWindow()) to produce a stable
 * readable display rather than per-frame noise.
 *
 * Configuration (position, margin, text color, font size, averaging window)
 * can be changed at any time and takes effect on the next frame.
 */
class FPSOverlay : public QLabel, public IOverlay {
    Q_OBJECT

  public:
    explicit FPSOverlay(QVTKOpenGLNativeWidget* host);
    ~FPSOverlay() override = default;

    // ── Visibility ────────────────────────────────────────────────────────────
    void SetEnabled(bool enabled) override;

    // ── Appearance ────────────────────────────────────────────────────────────
    void SetTextColor(const QColor& color);
    void SetFontSize(int pt);

    // ── Averaging ─────────────────────────────────────────────────────────────
    /** @brief Set the sliding-window duration for FPS/frame-time averaging (1–60 s). */
    void SetAveragingWindow(int seconds);

    // ── Layout ───────────────────────────────────────────────────────────────
    void SetPosition(OverlayPosition pos) override;
    void SetMargin(int px);

  protected:
    void resizeEvent(QResizeEvent* event) override;

  private Q_SLOTS:
    void _OnFrameSwapped();

  private:
    void _Reposition();
    void _UpdateText(double frameMs, double fps);

    QVTKOpenGLNativeWidget* m_host{nullptr};

    QElapsedTimer m_frameTimer;
    qint64 m_lastFrameNs{0};

    // Sliding window: stores elapsed-ns timestamps of recent frames.
    std::deque<qint64> m_frameTimestamps;
    qint64 m_windowNs{1'000'000'000LL};  // default: 1 second

    int m_margin{8};
};

}  // namespace overlays

#endif  // FPSOVERLAY_HPP
