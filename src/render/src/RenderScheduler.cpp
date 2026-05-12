#include "render/RenderScheduler.hpp"

#include <vtkResliceImageViewer.h>

#include <algorithm>

#include "render/RivRenderTarget.hpp"
#include "render/WindowRenderTarget.hpp"

namespace render {

RenderScheduler::RenderScheduler() = default;

// Destructor defined here so that unique_ptr<IRenderTarget> inside Entry can
// call ~IRenderTarget() with the complete type visible (it is included above).
RenderScheduler::~RenderScheduler() = default;

TargetHandle RenderScheduler::RegisterWindow(vtkRenderWindow* window) {
    if (!window)
        return kInvalidHandle;

    // Avoid duplicate registrations for the same window.
    for (const auto& e : m_entries) {
        if (e.window == window)
            return e.handle;
    }

    TargetHandle h = _NextHandle();
    m_entries.push_back({h, window, std::make_unique<WindowRenderTarget>(window)});
    return h;
}

TargetHandle RenderScheduler::RegisterRiv(vtkResliceImageViewer* riv) {
    if (!riv)
        return kInvalidHandle;

    vtkRenderWindow* win = riv->GetRenderWindow();

    // One RIV target per RIV — do not deduplicate against WindowRenderTarget
    // for the same window, because a RIV target must execute riv->Render() first.
    // If both a WindowRenderTarget and a RivRenderTarget share the same window,
    // Flush() will see the window as dirty from either RequestRender() path, but
    // only execute the first matching target found. For mixed registrations on the
    // same window, prefer always using RegisterRiv (never mix the two for the same window).
    for (const auto& e : m_entries) {
        // If a RivRenderTarget for this exact riv is already registered, reuse it.
        if (auto* rivTarget = dynamic_cast<RivRenderTarget*>(e.target.get())) {
            (void)rivTarget;  // only reachable if window matches riv's window
            if (e.window == win)
                return e.handle;
        }
    }

    TargetHandle h = _NextHandle();
    m_entries.push_back({h, win, std::make_unique<RivRenderTarget>(riv)});
    return h;
}

void RenderScheduler::Unregister(TargetHandle handle) {
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
                           [handle](const Entry& e) { return e.handle == handle; });
    if (it != m_entries.end()) {
        m_dirty.erase(it->window);
        m_entries.erase(it);
    }
}

void RenderScheduler::RequestRender(vtkRenderWindow* window) {
    if (!window)
        return;

    // Only accept requests for registered windows to avoid stale pointers.
    for (const auto& e : m_entries) {
        if (e.window == window) {
            m_dirty.insert(window);
            return;
        }
    }
}

void RenderScheduler::RequestRenderAll() {
    for (const auto& e : m_entries)
        m_dirty.insert(e.window);
}

void RenderScheduler::Flush() {
    if (m_dirty.empty())
        return;

    // For each dirty window, find its target and render once.
    // Iteration order over m_entries (insertion order) is deterministic,
    // which matters when RIV targets must fire before WindowRenderTargets
    // on the same window — so always RegisterRiv before RegisterWindow
    // for the same window (or simply never mix them).
    for (const auto& e : m_entries) {
        if (!m_dirty.count(e.window))
            continue;

        e.target->ExecuteRender();
        m_dirty.erase(e.window);  // prevent a second target hitting the same window
    }

    m_dirty.clear();
}

bool RenderScheduler::HasPendingRenders() const {
    return !m_dirty.empty();
}

std::size_t RenderScheduler::TargetCount() const {
    return m_entries.size();
}

TargetHandle RenderScheduler::_NextHandle() {
    return m_nextHandle++;
}

}  // namespace render
