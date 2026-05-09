#include "ui/MultiWindowView.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QHBoxLayout>
#include <memory>

#include "controllers/MultiWindowController.hpp"

namespace ui {

MultiWindowView::MultiWindowView(QWidget* parent)
    : IView(parent),
      m_multiWindowController(std::make_unique<controllers::MultiWindowController>()) {
    _setupUI();
}

MultiWindowView::~MultiWindowView() = default;

void MultiWindowView::_setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_vtkWidgets.resize(3);

    for (int i = 0; i < 3; ++i) {
        auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renderWindow->SetMultiSamples(0);

        m_vtkWidgets[i] = new QVTKOpenGLNativeWidget(this);
        m_vtkWidgets[i]->setRenderWindow(renderWindow);

        layout->addWidget(m_vtkWidgets[i]);
    }

    m_multiWindowController->Initialize(m_vtkWidgets);
    connect(m_multiWindowController.get(), &controllers::MultiWindowController::StatusChanged, this, &MultiWindowView::StatusChanged);
}

}  // namespace ui
