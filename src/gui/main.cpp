#include <QApplication>
#include <QSystemTrayIcon>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "intelcryotecapp.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            break;
    }
    QFile outFile("intel_cryo_tec_log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << QDateTime::currentDateTime().toString() << " - " << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qCritical() << "System tray not available on this system.";
        return 1;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    IntelCryoTECApp trayIcon;
    trayIcon.show();

    return app.exec();
}
