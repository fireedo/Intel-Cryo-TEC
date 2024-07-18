#pragma once

#include <QSystemTrayIcon>
#include <QTimer>
#include <QJsonObject>
#include "graphdialog.h"

class QMenu;

class IntelCryoTECApp : public QSystemTrayIcon
{
    Q_OBJECT

public:
    IntelCryoTECApp();
    ~IntelCryoTECApp();

private slots:
    void refresh();
    void showMessage();
    void showGraph();

private:
    QTimer *m_refreshTimer;
    QString m_statusMessage;
    QString m_rawJsonData;
    QMenu *m_menu;
    GraphDialog *m_graphDialog;

    void readStatusFile();
    void updateAppStatus(const QJsonObject &data);
    void setErrorState(const QString &message);
    void setOkayState(const QString &message);
    void createMenu();
};
