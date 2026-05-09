#include "ui/ViewportView.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QHBoxLayout>

#include "controllers/ViewportController.hpp"

namespace ui {

ViewportView::ViewportView(QWidget* parent)
    : IView(parent),
      m_viewportController(std::make_unique<controllers::ViewportController>()) {
    _setupUI();
}

ViewportView::~ViewportView() = default;

void ViewportView::_setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->SetMultiSamples(0);

    auto* widget = new QVTKOpenGLNativeWidget(this);
    widget->setRenderWindow(renderWindow);
    layout->addWidget(widget);

    m_vtkWidgets = {widget};

    m_viewportController->Initialize(m_vtkWidgets);
    connect(m_viewportController.get(), &controllers::ViewportController::StatusChanged,
            this, &ViewportView::StatusChanged);
}

}  // namespace ui
