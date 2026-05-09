#include "XQVtkViewport/ViewportManager.hpp"

#include <vtkCallbackCommand.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>

#include "XQVtkViewport/RenderStatsOverlay.hpp"

namespace qvv {

// ── Internal viewport slot ────────────────────────────────────────────────
namespace detail {

struct ViewportSlot {
    int index{-1};
    ViewportConfig config;
    vtkSmartPointer<vtkRenderer> renderer;
    RenderStats stats;
    std::unique_ptr<RenderStatsOverlay> overlay;
    std::chrono::steady_clock::time_point lastFrameEnd;
    bool firstFrame{true};
};

}  // namespace detail

// ── pImpl ─────────────────────────────────────────────────────────────────
class ViewportManager::Impl {
  public:
    vtkSmartPointer<vtkRenderWindow> m_renderWindow;
    bool m_ownsWindow{false};
    bool m_statsOverlayEnabled{false};
    int m_nextIndex{0};

    std::vector<detail::ViewportSlot> m_slots;
    std::vector<IViewportObserver*> m_observers;

    // VTK callback for render start/end timing
    vtkSmartPointer<vtkCallbackCommand> m_startCallback;
    vtkSmartPointer<vtkCallbackCommand> m_endCallback;

    // Per-render timing (shared across all viewports in one Render() call)
    std::chrono::steady_clock::time_point m_renderStart;

    ViewportManager* m_owner{nullptr};

    // ── helpers ──────────────────────────────────────────────────────────
    void EnsureRenderWindow() {
        if (m_renderWindow)
            return;
        auto win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        win->SetMultiSamples(0);
        m_renderWindow = win;
        m_ownsWindow = true;
        AttachTimingCallbacks();
    }

    void AttachTimingCallbacks() {
        if (!m_renderWindow)
            return;

        m_startCallback = vtkSmartPointer<vtkCallbackCommand>::New();
        m_startCallback->SetCallback(&Impl::OnRenderStart);
        m_startCallback->SetClientData(this);
        m_renderWindow->AddObserver(vtkCommand::StartEvent, m_startCallback);

        m_endCallback = vtkSmartPointer<vtkCallbackCommand>::New();
        m_endCallback->SetCallback(&Impl::OnRenderEnd);
        m_endCallback->SetClientData(this);
        m_renderWindow->AddObserver(vtkCommand::EndEvent, m_endCallback);
    }

    detail::ViewportSlot* FindSlot(int index) {
        for (auto& slot : m_slots)
            if (slot.index == index)
                return &slot;
        return nullptr;
    }

    const detail::ViewportSlot* FindSlot(int index) const {
        for (const auto& slot : m_slots)
            if (slot.index == index)
                return &slot;
        return nullptr;
    }

    void ApplyViewportToRenderer(const detail::ViewportSlot& slot) {
        const auto& cfg = slot.config;
        slot.renderer->SetViewport(cfg.xMin, cfg.yMin, cfg.xMax, cfg.yMax);
        slot.renderer->SetBackground(
            cfg.background[0], cfg.background[1], cfg.background[2]);
        slot.renderer->SetLayer(cfg.layer);
    }

    void NotifyFrameRendered(int index, const RenderStats& stats) {
        for (auto* obs : m_observers)
            obs->OnFrameRendered(index, stats);
        emit m_owner->frameRendered(index, stats.frameTimeMs);
    }

    // ── VTK callbacks ─────────────────────────────────────────────────────
    static void OnRenderStart(vtkObject*, unsigned long, void* clientData, void*) {
        auto* self = static_cast<Impl*>(clientData);
        self->m_renderStart = std::chrono::steady_clock::now();
    }

    static void OnRenderEnd(vtkObject*, unsigned long, void* clientData, void*) {
        auto* self = static_cast<Impl*>(clientData);
        const auto renderEnd = std::chrono::steady_clock::now();
        const double renderMs = std::chrono::duration<double, std::milli>(
                                    renderEnd - self->m_renderStart)
                                    .count();

        for (auto& slot : self->m_slots) {
            const auto now = renderEnd;
            double frameMs = 0.0;
            if (!slot.firstFrame) {
                frameMs = std::chrono::duration<double, std::milli>(
                              now - slot.lastFrameEnd)
                              .count();
            }
            slot.firstFrame = false;
            slot.lastFrameEnd = now;

            slot.stats.frameTimeMs = frameMs;
            slot.stats.renderTimeMs = renderMs;
            slot.stats.frameCount++;

            if (self->m_statsOverlayEnabled && slot.overlay)
                slot.overlay->Update(slot.config.label, slot.stats);

            self->NotifyFrameRendered(slot.index, slot.stats);
        }
    }
};

// ── ViewportManager ────────────────────────────────────────────────────────

ViewportManager::ViewportManager(QObject* parent)
    : QObject(parent), m_impl(std::make_unique<Impl>()) {
    m_impl->m_owner = this;
}

ViewportManager::~ViewportManager() = default;

void ViewportManager::SetRenderWindow(vtkRenderWindow* window) {
    m_impl->m_renderWindow = window;
    m_impl->m_ownsWindow = false;
    m_impl->AttachTimingCallbacks();
}

vtkRenderWindow* ViewportManager::GetRenderWindow() const {
    return m_impl->m_renderWindow;
}

int ViewportManager::AddViewport(const ViewportConfig& config) {
    m_impl->EnsureRenderWindow();

    detail::ViewportSlot slot;
    slot.index = m_impl->m_nextIndex++;
    slot.config = config;
    slot.renderer = vtkSmartPointer<vtkRenderer>::New();

    m_impl->ApplyViewportToRenderer(slot);
    m_impl->m_renderWindow->AddRenderer(slot.renderer);

    // Keep layer count up to date
    const int maxLayer = slot.config.layer + 1;
    if (m_impl->m_renderWindow->GetNumberOfLayers() < maxLayer)
        m_impl->m_renderWindow->SetNumberOfLayers(maxLayer);

    slot.overlay =
        std::make_unique<RenderStatsOverlay>(slot.renderer);
    slot.overlay->SetVisible(m_impl->m_statsOverlayEnabled);

    const int idx = slot.index;
    m_impl->m_slots.push_back(std::move(slot));

    for (auto* obs : m_impl->m_observers)
        obs->OnViewportAdded(idx);

    return idx;
}

void ViewportManager::RemoveViewport(int index) {
    auto it = std::find_if(m_impl->m_slots.begin(), m_impl->m_slots.end(),
                           [index](const detail::ViewportSlot& s) { return s.index == index; });
    if (it == m_impl->m_slots.end())
        return;

    if (m_impl->m_renderWindow)
        m_impl->m_renderWindow->RemoveRenderer(it->renderer);

    m_impl->m_slots.erase(it);
}

void ViewportManager::SetViewportConfig(int index, const ViewportConfig& config) {
    auto* slot = m_impl->FindSlot(index);
    if (!slot)
        return;
    slot->config = config;
    m_impl->ApplyViewportToRenderer(*slot);
}

ViewportConfig ViewportManager::GetViewportConfig(int index) const {
    const auto* slot = m_impl->FindSlot(index);
    if (!slot)
        return {};
    return slot->config;
}

int ViewportManager::GetViewportCount() const {
    return static_cast<int>(m_impl->m_slots.size());
}

vtkRenderer* ViewportManager::GetRenderer(int index) const {
    const auto* slot = m_impl->FindSlot(index);
    if (!slot)
        return nullptr;
    return slot->renderer;
}

void ViewportManager::EnableStatsOverlay(bool enable) {
    m_impl->m_statsOverlayEnabled = enable;
    for (auto& slot : m_impl->m_slots)
        if (slot.overlay)
            slot.overlay->SetVisible(enable);
}

bool ViewportManager::IsStatsOverlayEnabled() const {
    return m_impl->m_statsOverlayEnabled;
}

RenderStats ViewportManager::GetStats(int index) const {
    const auto* slot = m_impl->FindSlot(index);
    if (!slot)
        return {};
    return slot->stats;
}

void ViewportManager::AddObserver(IViewportObserver* observer) {
    if (!observer)
        return;
    auto it = std::find(m_impl->m_observers.begin(), m_impl->m_observers.end(), observer);
    if (it == m_impl->m_observers.end())
        m_impl->m_observers.push_back(observer);
}

void ViewportManager::RemoveObserver(IViewportObserver* observer) {
    auto& v = m_impl->m_observers;
    v.erase(std::remove(v.begin(), v.end(), observer), v.end());
}

void ViewportManager::Render() {
    if (m_impl->m_renderWindow)
        m_impl->m_renderWindow->Render();
}

void ViewportManager::ResetAllCameras() {
    for (auto& slot : m_impl->m_slots)
        slot.renderer->ResetCamera();
}

}  // namespace qvv
