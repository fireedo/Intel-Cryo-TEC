#include "intelcryotecapp.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

IntelCryoTECApp::IntelCryoTECApp()
: QSystemTrayIcon()
{
    setIcon(QIcon(":/images/blue.png"));
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &IntelCryoTECApp::refresh);
    m_refreshTimer->start(1000); // Refresh every second

    m_graphDialog = new GraphDialog();

    createMenu();
    refresh(); // Initial refresh
}

IntelCryoTECApp::~IntelCryoTECApp()
{
    m_refreshTimer->stop();
    delete m_graphDialog;
}

void IntelCryoTECApp::refresh()
{
    readStatusFile();
}

void IntelCryoTECApp::readStatusFile()
{
    QFile file("/var/run/intel_cryo_tec/status.json");
    if (!file.open(QIODevice::ReadOnly)) {
        setErrorState("Error reading the Intel Cryo TEC status file.");
        qCritical() << "Failed to open status file:" << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        setErrorState("Error parsing the Intel Cryo TEC status file.");
        qCritical() << "Failed to parse JSON data";
        return;
    }

    QJsonObject jsonObject = doc.object();
    updateAppStatus(jsonObject);

    m_rawJsonData = QString::fromUtf8(QJsonDocument(jsonObject).toJson(QJsonDocument::Indented));
}

void IntelCryoTECApp::updateAppStatus(const QJsonObject &data)
{
    qint64 timestamp = data["timestamp"].toString().toLongLong();
    qint64 currentTimestamp = QDateTime::currentSecsSinceEpoch();

    if ((currentTimestamp - timestamp) > 5) {
        setErrorState("Service not running");
        qWarning() << "Service not running. Last update:" << QDateTime::fromSecsSinceEpoch(timestamp).toString();
        return;
    }

    QJsonObject heartbeat = data["heartbeat"].toObject();
    bool isPidRunning = heartbeat["PID is running"].toBool();

    if (!isPidRunning) {
        setErrorState("Cooler is in standby");
        qWarning() << "Cooler is in standby";
        return;
    }

    double voltage = data["voltage"].toDouble();
    double current = data["current"].toDouble();
    double wattage = voltage * current;
    double dewpoint = data["dewpoint"].toDouble();
    double temperature = data["temperature"].toDouble();

    QString status = QString("Dewpoint: %1°C\nTemperature: %2°C\nVoltage: %3V\nCurrent: %4A\nWattage: %5W")
    .arg(dewpoint, 0, 'f', 2)
    .arg(temperature, 0, 'f', 2)
    .arg(voltage, 0, 'f', 2)
    .arg(current, 0, 'f', 2)
    .arg(wattage, 0, 'f', 2);

    setOkayState(status);

    m_graphDialog->addDataPoint(temperature, wattage);

    qInfo() << "Status updated - Dewpoint:" << dewpoint
    << "Temperature:" << temperature
    << "Voltage:" << voltage
    << "Current:" << current
    << "Wattage:" << wattage;
}

void IntelCryoTECApp::setErrorState(const QString &message)
{
    m_statusMessage = message;
    setIcon(QIcon(":/images/red.png"));
    setToolTip("Intel Cryo TEC: Error\n" + message);
    qWarning() << "Error state:" << message;
}

void IntelCryoTECApp::setOkayState(const QString &message)
{
    m_statusMessage = message;
    setIcon(QIcon(":/images/blue.png"));
    setToolTip("Intel Cryo TEC: OK\n" + message);
}

void IntelCryoTECApp::createMenu()
{
    m_menu = new QMenu();

    QAction *showMessageAction = new QAction("Show Status", this);
    connect(showMessageAction, &QAction::triggered, this, &IntelCryoTECApp::showMessage);
    m_menu->addAction(showMessageAction);

    QAction *showGraphAction = new QAction("Show Graph", this);
    connect(showGraphAction, &QAction::triggered, this, &IntelCryoTECApp::showGraph);
    m_menu->addAction(showGraphAction);

    QAction *copyRawDataAction = new QAction("Copy Raw Data", this);
    connect(copyRawDataAction, &QAction::triggered, this, [this]() {
        QApplication::clipboard()->setText(m_rawJsonData);
        qInfo() << "Raw data copied to clipboard";
    });
    m_menu->addAction(copyRawDataAction);

    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    m_menu->addAction(quitAction);

    setContextMenu(m_menu);
}

void IntelCryoTECApp::showMessage()
{
    QMessageBox::information(nullptr, "Intel Cryo TEC Status", m_statusMessage);
    qInfo() << "Status message displayed";
}

void IntelCryoTECApp::showGraph()
{
    m_graphDialog->show();
    qInfo() << "Graph dialog displayed";
}
