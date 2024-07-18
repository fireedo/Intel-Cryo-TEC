#pragma once

#include <QDialog>
#include "qcustomplot.h"

class GraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphDialog(QWidget *parent = nullptr);
    void addDataPoint(double temperature, double wattage);

private:
    QCustomPlot *m_plot;
    QCPGraph *m_temperatureGraph;
    QCPGraph *m_wattageGraph;
};
