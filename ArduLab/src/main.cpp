#include "ui/MainWindow.h"
#include <QApplication>
#include <QTimer>
#include <QPixmap>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("ArduLab");
    QApplication::setApplicationName("ArduLab");
    QApplication::setApplicationVersion("0.1.0");

    ardulab::MainWindow window;
    window.show();

    // Optional layout capture for headless verification (offscreen platform).
    const QByteArray shot = qgetenv("ARDULAB_SCREENSHOT");
    if (!shot.isEmpty()) {
        window.resize(1280, 820);
        QTimer::singleShot(1200, [&]() {
            window.grab().save(QString::fromLocal8Bit(shot));
            QApplication::quit();
        });
    }
    return app.exec();
}
