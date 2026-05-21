#ifndef XQVtkViewport_UI_ABOUTDIALOG_HPP
#define XQVtkViewport_UI_ABOUTDIALOG_HPP

#include <QDialog>

namespace ui {

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    void _BuildUi();
};

}  // namespace ui

#endif  // XQVtkViewport_UI_ABOUTDIALOG_HPP
