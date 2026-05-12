#include "overlays/IOverlay.hpp"

#include <QObject>

namespace overlays {
QString QStringFromOverlayPosition(const OverlayPosition& pos) {
    switch (pos) {
        case OverlayPosition::TopLeft:
            return QObject::tr("Top-Left");
        case OverlayPosition::TopRight:
            return QObject::tr("Top-Right");
        case OverlayPosition::TopCenter:
            return QObject::tr("Top-Center");
        case OverlayPosition::BottomLeft:
            return QObject::tr("Bottom-Left");
        case OverlayPosition::BottomRight:
            return QObject::tr("Bottom-Right");
        case OverlayPosition::BottomCenter:
            return QObject::tr("Bottom-Center");
    }

    return "";
}

OverlayPosition OverlayPositionFromQString(const QString& str) {
    if (str == QObject::tr("Top-Left")) {
        return OverlayPosition::TopLeft;
    } else if (str == QObject::tr("Top-Right")) {
        return OverlayPosition::TopRight;
    } else if (str == QObject::tr("Top-Center")) {
        return OverlayPosition::TopCenter;
    } else if (str == QObject::tr("Bottom-Left")) {
        return OverlayPosition::BottomLeft;
    } else if (str == QObject::tr("Bottom-Right")) {
        return OverlayPosition::BottomRight;
    } else if (str == QObject::tr("Bottom-Center")) {
        return OverlayPosition::BottomCenter;
    }

    return OverlayPosition::TopLeft;  // Default fallback
}

QList<QString> GetOverlayPositionNames() {
    return {
        QObject::tr("Top-Left"),
        QObject::tr("Top-Right"),
        QObject::tr("Top-Center"),
        QObject::tr("Bottom-Left"),
        QObject::tr("Bottom-Right"),
        QObject::tr("Bottom-Center")};
}

}  // namespace overlays
