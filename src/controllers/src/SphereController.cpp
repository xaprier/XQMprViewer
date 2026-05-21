#include "controllers/SphereController.hpp"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

#include <QDebug>
#include <algorithm>

#include "render/RenderScheduler.hpp"

namespace controllers {

SphereController::SphereController(QObject* parent)
    : IControllerBase(parent) {
    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetThetaResolution(16);
    m_sphereSource->SetPhiResolution(16);
    m_sphereSource->SetRadius(3.0);

    m_picker = vtkSmartPointer<vtkCellPicker>::New();
    m_picker->SetTolerance(0.005);

    m_leftDownCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_leftDownCmd->SetCallback(&SphereController::OnLeftButtonDown);
    m_leftDownCmd->SetClientData(this);

    m_mouseMoveCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_mouseMoveCmd->SetCallback(&SphereController::OnMouseMove);
    m_mouseMoveCmd->SetClientData(this);

    m_leftUpCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_leftUpCmd->SetCallback(&SphereController::OnLeftButtonUp);
    m_leftUpCmd->SetClientData(this);
}

SphereController::~SphereController() = default;

// ── Registration ──────────────────────────────────────────────────────────────

void SphereController::AddRenderer(vtkSmartPointer<vtkRenderer> renderer, DragPlane plane) {
    if (!renderer)
        return;

    for (const auto& e : m_rendererEntries) {
        if (e.renderer == renderer)
            return;
    }

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(m_sphereSource->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.3, 0.3);
    actor->GetProperty()->SetOpacity(0.85);

    renderer->AddActor(actor);
    m_rendererEntries.push_back({renderer, actor, plane});
}

void SphereController::AddInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor) {
    if (!interactor)
        return;

    if (std::find(m_interactors.begin(), m_interactors.end(), interactor) != m_interactors.end())
        return;

    m_interactors.push_back(interactor);

    // Priority 3.0 — sphere events run before viewport router (2.0) and default style (1.0)
    interactor->AddObserver(vtkCommand::LeftButtonPressEvent, m_leftDownCmd, 3.0f);
    interactor->AddObserver(vtkCommand::MouseMoveEvent, m_mouseMoveCmd, 3.0f);
    interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, m_leftUpCmd, 3.0f);
    interactor->AddObserver(vtkCommand::LeaveEvent, m_leftUpCmd, 3.0f);
}

// ── Cleanup ───────────────────────────────────────────────────────────────────

void SphereController::Cleanup() {
    for (const auto& e : m_rendererEntries)
        if (e.renderer)
            e.renderer->RemoveActor(e.actor);

    for (auto& interactor : m_interactors) {
        if (!interactor)
            continue;
        interactor->RemoveObserver(m_leftDownCmd);
        interactor->RemoveObserver(m_mouseMoveCmd);
        interactor->RemoveObserver(m_leftUpCmd);
    }

    m_rendererEntries.clear();
    m_interactors.clear();
    m_isDragging = false;
    m_activeRenderer = nullptr;
}

// ── State setters ─────────────────────────────────────────────────────────────

void SphereController::SetPosition(const Vec3& pos) {
    qDebug() << "SphereController::SetPosition: Setting sphere position to:" << pos[0] << pos[1] << pos[2];
    m_sphereSource->SetCenter(pos[0], pos[1], pos[2]);
    m_sphereSource->Update();
    for (auto& entry : m_rendererEntries) {
        entry.actor->Modified();
    }
    emit SphereMoved(pos);
}

void SphereController::SetRadius(double radius) {
    qDebug() << "SphereController::SetRadius: Setting sphere radius to:" << radius;
    m_sphereSource->SetRadius(radius);
    m_sphereSource->Update();
    for (auto& entry : m_rendererEntries) {
        entry.actor->Modified();
    }
}

void SphereController::SetColor(const Vec3& rgb) {
    qDebug() << "SphereController::SetColor: Setting sphere color to:" << rgb[0] << rgb[1] << rgb[2];
    for (auto& entry : m_rendererEntries) {
        entry.actor->GetProperty()->SetColor(rgb[0], rgb[1], rgb[2]);
        entry.actor->Modified();
    }
}

// ── Getters ───────────────────────────────────────────────────────────────────

double SphereController::GetRadius() const {
    return m_sphereSource->GetRadius();
}

SphereController::Vec3 SphereController::GetPosition() const {
    double c[3];
    m_sphereSource->GetCenter(c);
    return {c[0], c[1], c[2]};
}

SphereController::Vec3 SphereController::GetColor() const {
    double rgb[3];
    auto actor = m_rendererEntries.front().actor;
    actor->GetProperty()->GetColor(rgb);
    return {rgb[0], rgb[1], rgb[2]};
}

bool SphereController::IsDragging() const {
    return m_isDragging;
}

vtkActor* SphereController::ActorFor(vtkRenderer* renderer) const {
    for (auto& entry : m_rendererEntries) {
        if (entry.renderer == renderer) {
            return entry.actor;
        }
    }

    return nullptr;
}

// ── Private helpers ───────────────────────────────────────────────────────────

SphereController::DragPlane SphereController::PlaneFor(vtkRenderer* renderer) const {
    for (const auto& e : m_rendererEntries) {
        if (e.renderer.Get() == renderer)
            return e.plane;
    }
    return DragPlane::Axial;
}

void SphereController::SetScheduler(render::RenderScheduler* scheduler) {
    m_scheduler = scheduler;
}

void SphereController::RenderAll() {
    if (m_scheduler) {
        m_scheduler->RequestRenderAll();
        m_scheduler->Flush();
        return;
    }

    // Fallback when no scheduler is wired (should not happen in normal usage).
    std::vector<vtkRenderWindow*> seen;
    for (const auto& e : m_rendererEntries) {
        if (!e.renderer)
            continue;
        auto* win = e.renderer->GetRenderWindow();
        if (!win)
            continue;
        if (std::find(seen.begin(), seen.end(), win) == seen.end()) {
            seen.push_back(win);
            win->Render();
        }
    }
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void SphereController::OnLeftButtonDown(vtkObject* caller, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SphereController*>(clientData);

    self->m_leftDownCmd->SetAbortFlag(0);  // important

    auto* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (!interactor)
        return;

    int pos[2];
    interactor->GetEventPosition(pos);

    // FindPokedRenderer works for BOTH modes:
    //   viewport mode  (1 window, N renderers): returns renderer at mouse pos
    //   multiwindow    (N windows, 1 renderer each): returns the single renderer
    vtkRenderer* renderer = interactor->FindPokedRenderer(pos[0], pos[1]);
    if (!renderer)
        return;

    self->m_picker->Pick(pos[0], pos[1], 0.0, renderer);

    auto* expectedActor = self->ActorFor(renderer);
    if (self->m_picker->GetActor() != expectedActor)
        return;

    self->m_isDragging = true;
    self->m_activeRenderer = renderer;
    self->m_activePlane = self->PlaneFor(renderer);

    // Prevent the interactor style from entering window/level (or any other)
    // interaction state. Without this, the style owns the drag and its
    // OnLeftButtonUp can race with ours, leaving m_isDragging stuck true.
    // NOTE: SetEnabled(0) must NOT be used — it fires DisableEvent which
    // vtkResliceCursorWidget listens to, hiding the DICOM image.
    self->m_leftDownCmd->SetAbortFlag(1);
}

void SphereController::OnMouseMove(vtkObject* caller, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SphereController*>(clientData);
    if (!self->m_isDragging || !self->m_activeRenderer)
        return;

    auto* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (!interactor)
        return;

    int pos[2];
    interactor->GetEventPosition(pos);

    // Build world-space ray from screen position
    self->m_activeRenderer->SetDisplayPoint(pos[0], pos[1], 0.0);
    self->m_activeRenderer->DisplayToWorld();
    double near4[4];
    self->m_activeRenderer->GetWorldPoint(near4);

    self->m_activeRenderer->SetDisplayPoint(pos[0], pos[1], 1.0);
    self->m_activeRenderer->DisplayToWorld();
    double far4[4];
    self->m_activeRenderer->GetWorldPoint(far4);

    if (near4[3] == 0.0 || far4[3] == 0.0)
        return;

    const double rayNear[3] = {near4[0] / near4[3], near4[1] / near4[3], near4[2] / near4[3]};
    const double rayFar[3] = {far4[0] / far4[3], far4[1] / far4[3], far4[2] / far4[3]};

    // Drag plane passes through sphere center; normal is the view axis of the active viewport
    double center[3];
    self->m_sphereSource->GetCenter(center);

    double normal[3] = {0.0, 0.0, 0.0};
    switch (self->m_activePlane) {
        case DragPlane::Axial:
            normal[2] = 1.0;
            break;
        case DragPlane::Coronal:
            normal[1] = 1.0;
            break;
        case DragPlane::Sagittal:
            normal[0] = 1.0;
            break;
    }

    double t = 0.0;
    double intersect[3] = {0.0, 0.0, 0.0};
    if (!vtkPlane::IntersectWithLine(rayNear, rayFar, normal, center, t, intersect))
        return;

    self->SetPosition({intersect[0], intersect[1], intersect[2]});
    // Abort so the style doesn't do window/level simultaneously with sphere drag.
    // SetPosition emits SphereMoved → SliceController::OnSphereUpdated → RenderAll,
    // so the scheduler already has a pending flush; no extra render call needed here.
    // self->m_mouseMoveCmd->SetAbortFlag(1);
}

void SphereController::OnLeftButtonUp(vtkObject*, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SphereController*>(clientData);
    if (!self->m_isDragging)
        return;

    self->m_isDragging = false;
    self->m_activeRenderer = nullptr;
}

}  // namespace controllers
