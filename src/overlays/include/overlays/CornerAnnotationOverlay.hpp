#ifndef CORNERANNOTATIONOVERLAY_HPP
#define CORNERANNOTATIONOVERLAY_HPP

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QString>
#include <QWidget>

#include "overlays/IOverlay.hpp"

class QVTKOpenGLNativeWidget;
class vtkResliceImageViewer;

namespace overlays {

/**
 * @brief Slice-metadata annotation overlay rendered as a child widget on a
 *        QVTKOpenGLNativeWidget.
 *
 * Displays enabled metadata lines (slice index/count, window/level, spacing,
 * view name) stacked vertically inside a single text block positioned at any
 * corner or edge via SetPosition() — identical placement mechanics to FPSOverlay.
 *
 * Data is read from the attached vtkResliceImageViewer on every frameSwapped()
 * event, so values update automatically during interaction.
 *
 * In shared-viewport mode (one widget, multiple renderers) call
 * SetViewportFraction() to restrict painting to a horizontal sub-region.
 */
class CornerAnnotationOverlay : public QWidget, public IOverlay {
    Q_OBJECT

  public:
    explicit CornerAnnotationOverlay(QVTKOpenGLNativeWidget* host,
                                     const QString& viewName = QString());
    ~CornerAnnotationOverlay() override;

    // ── IOverlay ─────────────────────────────────────────────────────────────
    void SetEnabled(bool enabled) override;
    void SetPosition(OverlayPosition pos) override;

    // ── Data source ───────────────────────────────────────────────────────────
    /** @brief Attach the viewer from which slice/window-level data is read. Non-owning. */
    void SetViewer(vtkResliceImageViewer* viewer);

    /** @brief Override the view name line. Defaults to the constructor argument. */
    void SetViewName(const QString& name);

    // ── Appearance ────────────────────────────────────────────────────────────
    void SetTextColor(const QColor& color);
    void SetFontSize(int pt);
    void SetMargin(int px);

    // ── Line visibility ───────────────────────────────────────────────────────
    void SetShowSliceInfo(bool show);
    void SetShowWindowLevel(bool show);
    void SetShowSpacing(bool show);
    void SetShowViewName(bool show);

    // ── Shared-viewport clipping ──────────────────────────────────────────────
    /**
     * @brief Restrict painting to the horizontal sub-region [xMin, xMax] ∈ [0, 1].
     *
     * Default (0.0, 1.0) covers the full widget width.
     */
    void SetViewportFraction(double xMin, double xMax);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private Q_SLOTS:
    void _OnFrameSwapped();

  private:
    QString _BuildText() const;
    void    _Reposition();
    void    _InvalidateFontCache();

    QVTKOpenGLNativeWidget* m_host{nullptr};
    vtkResliceImageViewer*  m_viewer{nullptr};

    QString m_viewName;

    QColor m_textColor{Qt::white};
    int    m_fontSize{10};
    int    m_margin{8};
    int    m_padding{4};

    bool m_showSliceInfo{true};
    bool m_showWindowLevel{true};
    bool m_showSpacing{true};
    bool m_showViewName{true};

    double m_vpXMin{0.0};
    double m_vpXMax{1.0};

    mutable QFont        m_cachedFont;
    mutable QFontMetrics m_cachedMetrics{m_cachedFont};
    mutable bool         m_fontDirty{true};
};

}  // namespace overlays

#endif  // CORNERANNOTATIONOVERLAY_HPP
