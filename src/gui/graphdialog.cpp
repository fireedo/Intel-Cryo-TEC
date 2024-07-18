#include "graphdialog.h"
#include <QVBoxLayout>

GraphDialog::GraphDialog(QWidget *parent) : QDialog(parent)
{
    m_plot = new QCustomPlot(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_plot);

    m_temperatureGraph = m_plot->addGraph();
    m_temperatureGraph->setPen(QPen(Qt::blue));
    m_temperatureGraph->setName("Temperature");

    m_wattageGraph = m_plot->addGraph(m_plot->xAxis, m_plot->yAxis2);
    m_wattageGraph->setPen(QPen(Qt::red));
    m_wattageGraph->setName("Wattage");

    m_plot->xAxis->setLabel("Time");
    m_plot->yAxis->setLabel("Temperature (°C)");
    m_plot->yAxis2->setLabel("Wattage (W)");
    m_plot->yAxis2->setVisible(true);

    m_plot->legend->setVisible(true);

    setMinimumSize(600, 400);
}

void GraphDialog::addDataPoint(double temperature, double wattage)
{
    static double time = 0;
    m_temperatureGraph->addData(time, temperature);
    m_wattageGraph->addData(time, wattage);
    time += 1;

    m_plot->rescaleAxes();
    m_plot->replot();
}
