# XQMprViewer

A Qt Widgets + VTK based MPR (Multi-Planar Reconstruction) viewer demonstrating
how to build a production-quality medical imaging application with a clean,
layered architecture.

The project is split into two independently buildable parts: a reusable viewport management **library** (`lib/XQVtkViewport`) and a full-featured **demo application** (`src/`) that consumes it.

---

## Features

### Library (XQVtkViewport)

- Multi-viewport management over a single `vtkRenderWindow`
- Normalized [0, 1] viewport coordinate system (VTK convention)
- Built-in layout factory: horizontal split, vertical split, arbitrary grid
- Per-viewport render statistics (FPS, frame time, render time)
- VTK text-actor overlay for live stats display
- Observer interface for viewport lifecycle events
- pImpl-isolated public API; Qt and VTK headers do not leak into consumer code

### Application (XQMprViewer)

- Axial, coronal, and sagittal MPR views via `vtkResliceImageViewer`
- Asynchronous DICOM directory scan and series loading (background thread)
- DICOM-to-RAS reorientation on load
- Two display modes selectable at runtime:
    - **Viewport Mode** — three planes in a single shared `vtkRenderWindow`
    - **Multi-Window Mode** — three independent `vtkRenderWindow` instances
- Runtime layout switching (horizontal, vertical, grid) with per-pane plane assignment
- Draggable sphere annotation synchronized across all three planes
- Per-plane orientation marker overlay (L/R/A/P/S/I labels)
- Per-plane corner annotation overlay (patient / series metadata)
- Per-plane FPS overlay
- Centralized render scheduler with per-frame deduplication
- DICOM metadata panel (series-level tag browser)
- Controller panel with grouped controls for DICOM, sphere, and overlays

---

## Architecture

```
┌──────────────────────────────────────────────────────────────────────┐
│                         Demo Application                             │
│                                                                      │
│  ui/           Qt Widgets views, panels, layout selectors            │
│     │                                                                │
│     ▼                                                                │
│  controllers/  Interaction logic, DICOM loading, render coordination │
│     │                                                                │
│     ├──► adapters/   Data / layout bridge layer                      │
│     │                                                                │
│     ├──► render/     Centralized render scheduling and targets       │
│     │                                                                │
│     └──► overlays/   Per-viewport VTK and widget overlays            │
│               │                                                      │
│               ▼                                                      │
└──────────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌──────────────────────────────────────────────────────────────────────┐
│                    Library: XQVtkViewport (qvv::)                    │
│                                                                      │
│  ViewportManager   multi-viewport lifecycle on one render window     │
│  ViewportLayout    factory for common viewport arrangements          │
│  ViewportConfig    normalized position and appearance descriptor     │
│  RenderStats       per-frame timing metrics                          │
│  RenderStatsOverlay  live VTK text-actor stats display               │
│  IViewportObserver   observer interface for viewport events          │
└──────────────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer              | Namespace     | Responsibility                                               |
| ------------------ | ------------- | ------------------------------------------------------------ |
| `lib/`             | `qvv`         | Reusable viewport and stats library; no UI dependencies      |
| `src/ui/`          | `ui`          | Qt Widgets views, panels, layout selectors                   |
| `src/controllers/` | `controllers` | DICOM I/O, slice and sphere interaction, render coordination |
| `src/adapters/`    | `adapters`    | Bridges layout definitions, overlays, and metadata           |
| `src/render/`      | `render`      | Render scheduler; deduplicates per-event render requests     |
| `src/overlays/`    | `overlays`    | FPS, orientation marker, corner annotation overlays          |

### Key Design Patterns

- **pImpl** — library public headers expose no VTK or Qt types beyond forward declarations
- **IViewController broadcast** — a static instance list ensures `SetImageData`, `AddSphere`, etc. propagate to every active controller without the UI enumerating them
- **RenderScheduler** — multiple `RequestRender()` calls within one logical event collapse into a single `window->Render()` call at `Flush()` time
- **OverlayLayoutAdapter** — combines two independent visibility axes (user toggle, layout visibility) so an overlay is shown only when both are active
- **ILayoutTarget** — views implement this interface so `ViewportLayoutManager` can push layout changes without knowing concrete types

---

## Tech Stack

| Component | Version                     |
| --------- | --------------------------- |
| C++       | 17                          |
| CMake     | >= 3.20                     |
| Qt        | >= 6.4 (Core, Gui, Widgets) |
| VTK       | >= 9.2                      |
| vtk-dicom | 9.6                         |

VTK components required: `RenderingOpenGL2`, `RenderingAnnotation`, `RenderingCore`, `GUISupportQt`, `InteractionWidgets`, `InteractionStyle`, `InteractionImage`, `IOImage`, `FiltersCore`, `FiltersSources`.

---

## Project Structure

```
XQMprViewer/
├── CMakeLists.txt                      Root build configuration
│
├── lib/                                Reusable viewport management library
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── XQVtkViewport/
│   │       ├── IViewportObserver.hpp   Observer interface for viewport events
│   │       ├── RenderStats.hpp         Per-frame timing metrics struct
│   │       ├── RenderStatsOverlay.hpp  VTK text-actor stats overlay
│   │       ├── ViewportConfig.hpp      Normalized viewport descriptor
│   │       ├── ViewportLayout.hpp      Layout factory (split, grid)
│   │       ├── ViewportManager.hpp     Core multi-viewport manager
│   │       └── XQVtkViewport_export.hpp  CMake-generated symbol export macros
│   └── src/
│       ├── RenderStats.cpp
│       ├── RenderStatsOverlay.cpp
│       ├── ViewportLayout.cpp
│       └── ViewportManager.cpp
│
└── src/                                Demo application
    ├── CMakeLists.txt
    │
    ├── app/
    │   └── src/
    │       └── main.cpp                Entry point; sets up QApplication and MainWindow
    │
    ├── adapters/                       Data and layout bridge layer
    │   ├── include/adapters/
    │   │   ├── ColorAdapter.hpp        Color conversion utilities
    │   │   ├── DicomMetaDataAdapter.hpp  Extracts and formats DICOM tag data
    │   │   └── OverlayLayoutAdapter.hpp  Syncs overlay visibility with layout changes
    │   └── src/
    │       ├── DicomMetaDataAdapter.cpp
    │       └── OverlayLayoutAdapter.cpp
    │
    ├── controllers/                    Interaction logic and rendering coordination
    │   ├── include/controllers/
    │   │   ├── IControllerBase.hpp     QObject base for all controllers
    │   │   ├── IViewController.hpp     Abstract base; owns RenderScheduler, broadcast
    │   │   ├── DicomController.hpp     Async DICOM scan and series load
    │   │   ├── SliceController.hpp     Manages vtkResliceImageViewer (1 or 3 widgets)
    │   │   ├── SphereController.hpp    Draggable sphere; emits SphereMoved
    │   │   ├── ViewportController.hpp  Single-window multi-viewport controller
    │   │   ├── MultiWindowController.hpp  Three-window controller (benchmark mode)
    │   │   ├── ViewportInteractorStyle.hpp  Mouse routing for shared render window
    │   │   └── ResliceImageViewerInteractorStyle.hpp  Custom slice interaction
    │   └── src/
    │       ├── IViewController.cpp
    │       ├── DicomController.cpp
    │       ├── SliceController.cpp
    │       ├── SphereController.cpp
    │       ├── ViewportController.cpp
    │       ├── MultiWindowController.cpp
    │       ├── ViewportInteractorStyle.cpp
    │       └── ResliceImageViewerInteractorStyle.cpp
    │
    ├── overlays/                       Per-viewport visual overlays
    │   ├── include/overlays/
    │   │   ├── IOverlay.hpp            Base interface; position, enable/disable
    │   │   ├── FPSOverlay.hpp          Live frame-rate display
    │   │   ├── CornerAnnotationOverlay.hpp  Patient and series metadata at corners
    │   │   └── OrientationMarkerOverlay.hpp  L/R/A/P/S/I anatomical labels
    │   └── src/
    │       ├── IOverlay.cpp
    │       ├── FPSOverlay.cpp
    │       ├── CornerAnnotationOverlay.cpp
    │       └── OrientationMarkerOverlay.cpp
    │
    ├── render/                         Centralized render scheduling
    │   ├── include/render/
    │   │   ├── IRenderTarget.hpp       Abstract render target interface
    │   │   ├── RenderScheduler.hpp     Request/Flush deduplication scheduler
    │   │   ├── RenderTypes.hpp         Shared type aliases (TargetHandle)
    │   │   ├── RivRenderTarget.hpp     vtkResliceImageViewer render target
    │   │   └── WindowRenderTarget.hpp  Plain vtkRenderWindow render target
    │   └── src/
    │       ├── IRenderTarget.cpp
    │       ├── RenderScheduler.cpp
    │       ├── RivRenderTarget.cpp
    │       └── WindowRenderTarget.cpp
    │
    └── ui/                             Qt Widgets UI layer
        ├── include/ui/
        │   ├── IView.hpp               Base interface for view widgets
        │   ├── ILayoutTarget.hpp       Interface for objects that accept layout changes
        │   ├── MainWindow.hpp          Top-level application window
        │   ├── ViewportView.hpp        Single-window viewport mode (IView + ILayoutTarget)
        │   ├── MultiWindowView.hpp     Three-window mode view
        │   ├── ControllerPanel.hpp     Sidebar panel aggregating control items
        │   ├── ControllerPanelDicomItem.hpp         DICOM load section
        │   ├── ControllerPanelSphereItem.hpp         Sphere controls section
        │   ├── ControllerPanelFPSOverlayItem.hpp     FPS overlay toggle section
        │   ├── ControllerPanelCornerAnnotationItem.hpp  Corner annotation toggle
        │   ├── ControllerPanelOrientationMarkerItem.hpp  Orientation marker toggle
        │   ├── DicomMetaDataModel.hpp  Qt table model for DICOM tag data
        │   ├── DicomMetaDataPanel.hpp  DICOM metadata browser panel
        │   ├── ViewportLayoutManager.hpp  Runtime layout orchestrator
        │   ├── ViewportLayoutSelector.hpp  Toolbar widget for selecting layouts
        │   ├── ViewportLayoutItem.hpp  Individual layout option widget
        │   └── ViewportLayoutTypes.hpp  Layout type enum and factory functions
        └── src/
            ├── MainWindow.cpp
            ├── ViewportView.cpp
            ├── MultiWindowView.cpp
            ├── ControllerPanel.cpp
            ├── ControllerPanelDicomItem.cpp
            ├── ControllerPanelSphereItem.cpp
            ├── ControllerPanelFPSOverlayItem.cpp
            ├── ControllerPanelCornerAnnotationItem.cpp
            ├── ControllerPanelOrientationMarkerItem.cpp
            ├── DicomMetaDataModel.cpp
            ├── DicomMetaDataPanel.cpp
            ├── ViewportLayoutManager.cpp
            ├── ViewportLayoutSelector.cpp
            ├── ViewportLayoutItem.cpp
            └── ViewportLayoutTypes.cpp
```

---

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The binary is placed at `build/src/XQMprViewer_Widgets`.

If VTK or Qt are installed to a non-standard prefix, pass it via `CMAKE_PREFIX_PATH`:

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/vtk;/path/to/qt6"
```

RPATH is embedded at build time, so the binary runs without setting
`LD_LIBRARY_PATH` as long as the dependencies are at the paths used during the
build.

---

## Documentation

API documentation is generated with [Doxygen](https://www.doxygen.nl).
A `Doxyfile` is provided at the project root.

```bash
doxygen Doxyfile
```

Output is written to `docs/doxygen/html/`. Open `docs/doxygen/html/index.html`
in a browser to browse the generated reference.

Class collaboration and call graphs require the `dot` tool from
[Graphviz](https://graphviz.org). If Graphviz is not installed, Doxygen falls
back to plain HTML diagrams automatically.
