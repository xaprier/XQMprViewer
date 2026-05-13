#ifndef ORIENTATIONMARKEROVERLAY_HPP
#define ORIENTATIONMARKEROVERLAY_HPP

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QString>
#include <QWidget>

#include "overlays/IOverlay.hpp"

class QVTKOpenGLNativeWidget;

namespace overlays {

/**
 * @brief Anatomical orientation label overlay rendered as a child widget on top
 *        of a QVTKOpenGLNativeWidget.
 *
 * Draws four labels at the cardinal edges of the viewport (left, right, top,
 * bottom). Labels are derived from @p orientation:
 *  - Axial    (XY): L/Left  · R/Right · A/Anterior  · P/Posterior
 *  - Coronal  (XZ): L/Left  · R/Right · S/Superior  · I/Inferior
 *  - Sagittal (YZ): A/Anterior · P/Posterior · S/Superior · I/Inferior
 *
 * Use SetLongLabels(true) to switch from abbreviated (L, R, …) to full
 * anatomical names (Left, Right, …).
 *
 * In shared-viewport mode (one widget, multiple renderers side-by-side) call
 * SetViewportFraction() to restrict painting to a horizontal sub-region.
 *
 * Rendering is a lightweight Qt paint operation — no VTK pipeline involvement.
 * The overlay tracks host resize events via an event filter so it always fills
 * the host widget.
 */
class OrientationMarkerOverlay : public QWidget, public IOverlay {
    Q_OBJECT

  public:
    enum class SliceOrientation {
        Axial = 0,
        Coronal = 1,
        Sagittal = 2,
    };

    // Canonical per-plane ordering used by View classes.
    static constexpr SliceOrientation kSliceOrientations[3] = {
        SliceOrientation::Axial,
        SliceOrientation::Coronal,
        SliceOrientation::Sagittal,
    };

    explicit OrientationMarkerOverlay(QVTKOpenGLNativeWidget* host,
                                      SliceOrientation orientation = SliceOrientation::Axial);
    ~OrientationMarkerOverlay() override;

    // ── IOverlay ─────────────────────────────────────────────────────────────
    void SetEnabled(bool enabled) override;

    /** @brief No-op — labels are always drawn at fixed cardinal edges. */
    void SetPosition(OverlayPosition pos) override;

    // ── Appearance ────────────────────────────────────────────────────────────
    void SetTextColor(const QColor& color);
    void SetFontSize(int pt);

    /**
     * @brief Toggle between short (L/R/A/P/S/I) and long
     *        (Left/Right/Anterior/Posterior/Superior/Inferior) label mode.
     */
    void SetLongLabels(bool longLabels);

    // ── Slice plane ───────────────────────────────────────────────────────────
    void SetSliceOrientation(SliceOrientation orientation);

    /**
     * @brief Restrict painting to a horizontal sub-region of the host widget.
     *
     * Used in shared-viewport mode where several renderers share one widget.
     * @param xMin  Normalised left edge  [0.0, 1.0]
     * @param xMax  Normalised right edge [0.0, 1.0]
     *
     * Default (0.0, 1.0) paints across the full widget width.
     */
    void SetViewportFraction(double xMin, double xMax);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

  private:
    struct Labels {
        QString left;
        QString right;
        QString top;
        QString bottom;
    };

    Labels _ResolvedLabels() const;
    void _InvalidateFontCache();

    void _DrawLabel(QPainter& p, const QFontMetrics& fm,
                    const QString& text, Qt::Alignment align,
                    int vpLeft, int vpRight, int vpW, int totalH) const;

    QVTKOpenGLNativeWidget* m_host{nullptr};
    SliceOrientation m_orientation{SliceOrientation::Axial};
    QColor m_textColor{Qt::yellow};
    int m_fontSize{12};
    int m_margin{6};
    bool m_longLabels{false};

    // Cached font + metrics — invalidated whenever m_fontSize changes.
    mutable QFont m_cachedFont;
    mutable QFontMetrics m_cachedMetrics{m_cachedFont};
    mutable bool m_fontDirty{true};

    // Horizontal sub-region for shared-viewport mode [0, 1].
    double m_vpXMin{0.0};
    double m_vpXMax{1.0};
};

}  // namespace overlays

#endif  // ORIENTATIONMARKEROVERLAY_HPP
