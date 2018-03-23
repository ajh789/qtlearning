#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chartview.h"

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegExp>

#include <QLineSeries>
#include <QChart>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChartView>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ResetMinMaxValues();

    ui->setupUi(this);

    m_pLineSeries = new QLineSeries();
    // Sample data
    m_pLineSeries->append(QDateTime(QDate(2018, 3, 21), QTime(12, 13, 01)).toMSecsSinceEpoch(), 100);
    m_pLineSeries->append(QDateTime(QDate(2018, 3, 21), QTime(12, 13, 11)).toMSecsSinceEpoch(), 15);
    m_pLineSeries->append(QDateTime(QDate(2018, 3, 21), QTime(12, 13, 21)).toMSecsSinceEpoch(), 25);
    m_pLineSeries->append(QDateTime(QDate(2018, 3, 21), QTime(12, 13, 30)).toMSecsSinceEpoch(), 55);

    m_pChart = new QChart();
    m_pChart->addSeries(m_pLineSeries);
    m_pChart->legend()->hide();
    m_pChart->setTitle("Sunspots count (by Space Weather Prediction Center)");

    m_pAxisX = new QDateTimeAxis;
    if (m_pLineSeries->count() < 10) {
        m_pAxisX->setTickCount(10);
    } else {
        m_pAxisX->setTickCount(m_pLineSeries->count());
    }

    m_pAxisX->setFormat("MM-dd HH:mm:ss");
    m_pAxisX->setTitleText("Date");
    m_pChart->addAxis(m_pAxisX, Qt::AlignBottom);
    m_pLineSeries->attachAxis(m_pAxisX);

    m_pAxisY = new QValueAxis;
    m_pAxisY->setLabelFormat("%i");
    m_pAxisY->setTitleText("Value");
    m_pChart->addAxis(m_pAxisY, Qt::AlignLeft);
    m_pLineSeries->attachAxis(m_pAxisY);

    m_pChartView = new CChartView(m_pChart, this);
    m_pChartView->setRenderHint(QPainter::Antialiasing);
//  m_pChartView->setRubberBand(QChartView::HorizontalRubberBand);

    m_pEditFilePath = new QLineEdit(this);
    m_pEditFilePath->setReadOnly(true);
    m_pEditDebugInfo = new QTextEdit(this);
    m_pEditDebugInfo->hide();
    m_pEditDebugInfo->setLineWrapMode(QTextEdit::NoWrap);
    m_pEditDebugInfo->setMinimumWidth(250);
    m_pEditDebugInfo->setMaximumWidth(300);
    m_pBtnFileChooser = new QPushButton("Choose File", this);
    m_pBtnDrawChart   = new QPushButton("Draw", this);
    m_pBtnToggleDebug = new QPushButton("Debug", this);
    m_pBtnToggleDebug->setCheckable(true);
//  m_pBtnToggleDebug->setChecked(true);

    QHBoxLayout *pHeaderLayout = new QHBoxLayout(this);
    pHeaderLayout->addWidget(new QLabel("File path:", this));
    pHeaderLayout->addWidget(m_pEditFilePath);
    pHeaderLayout->addWidget(m_pBtnFileChooser);
    pHeaderLayout->addWidget(m_pBtnDrawChart);
    pHeaderLayout->addWidget(m_pBtnToggleDebug);

    QHBoxLayout *pBodyLayout = new QHBoxLayout(this);
    pBodyLayout->addWidget(m_pChartView);
    pBodyLayout->addWidget(m_pEditDebugInfo);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addLayout(pHeaderLayout);
    pLayout->addLayout(pBodyLayout);

    ui->centralWidget->setLayout(pLayout);

    connect(m_pBtnFileChooser, SIGNAL(pressed()), this, SLOT(ChooseFile()));
    connect(m_pBtnDrawChart,   SIGNAL(pressed()), this, SLOT(DrawChart()));
    connect(m_pBtnToggleDebug, SIGNAL(toggled(bool)), m_pEditDebugInfo, SLOT(setVisible(bool)));
    connect(m_pLineSeries, SIGNAL(pointAdded(int)), this, SLOT(PointAdded(int)));
    connect(m_pChartView, SIGNAL(LoadMoreData(int)), this, SLOT(LoadMoreData(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
    m_fileData.close();
}

void MainWindow::ResetMinMaxValues()
{
    m_nMinX = 0x7FFFFFFFFFFFFFFF; // Max of int64
    m_nMaxX = 0;
    m_nMaxY = 0;
}

void MainWindow::ClearData()
{
    ResetMinMaxValues();
    m_nLineCount = 0;
    m_pLineSeries->clear();
    m_pEditDebugInfo->clear();
}

void MainWindow::ChooseFile()
{
    m_strFilePath = QFileDialog::getOpenFileName(this, "Choose file");
    m_pEditFilePath->setText(m_strFilePath);
}

void MainWindow::DrawChart()
{
    m_fileData.setFileName(m_strFilePath);
    if (!m_fileData.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

//  m_pLineSeries->clear();
    ClearData();
//  int count = 0;
    m_streamData.setDevice(&m_fileData);
    LoadMoreData(200, true);
    m_pChartView->setFocus();
    /*
    QString strBuffer;

    while (!m_streamData.atEnd()) {
        QString strLine = m_streamData.readLine();
        strLine.replace('"', '\''); // " -> '
        QRegExp re("(\\d+-\\d+-\\d+T\\d+:\\d+:\\d+)[:.]{1}\\d+Z\\s+"
                   "\\{'name':'ActLineNumber','value':'(\\d+)'\\}");
        if (re.indexIn(strLine) != -1) {
            QStringList list = re.capturedTexts();
//          strBuffer += QString("%1\t%2\t%3\n").arg(QDateTime::fromString(list[1], "yyyy-MM-ddTHH:mm:ss").toString(), list[2]).arg(count+1);
            strBuffer += QString("%1\t%2\t%3\n").arg(list[1], list[2]).arg(count+1);
            m_pLineSeries->append(QDateTime::fromString(list[1], "yyyy-MM-ddTHH:mm:ss").toMSecsSinceEpoch(), list[2].toInt());
            count++;
            if (count == 200) break;
        }
    }

    if (m_streamData.atEnd()) {
        m_fileData.close();
    }

    m_pEditDebugInfo->setText(strBuffer);

    m_pAxisX->setRange(QDateTime::fromMSecsSinceEpoch(m_nMinX), QDateTime::fromMSecsSinceEpoch(m_nMaxX));
    m_pAxisY->setRange(0, m_nMaxY);
    */
//  if (m_pLineSeries->count() < 10) {
//      m_pAxisX->setTickCount(10);
//  } else {
//      m_pAxisX->setTickCount(m_pLineSeries->count());
//  }
}

void MainWindow::PointAdded(int index)
{
    qreal x = m_pLineSeries->at(index).x();
    qreal y = m_pLineSeries->at(index).y();
    if (m_nMaxY < y) {
        m_nMaxY = y;
//      m_pAxisY->setRange(0, y);
//      m_pChart->axisY()->setRange(0, y);
    }
    if (x < m_nMinX || x > m_nMaxX) {
        if (x < m_nMinX) {
            m_nMinX = x;
        }
        if (x > m_nMaxX) {
            m_nMaxX = x;
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
//  switch (event->key()) {
//  case Qt::Key_Left:
//      LoadMoreData();
//      break;
//  case Qt::Key_Right:
//      break;
//  case Qt::Key_Up:
//      break;
//  case Qt::Key_Down:
//      break;
//  default:
//      break;
//  }
    QWidget::keyPressEvent(event);
}


void MainWindow::LoadMoreData(int nLineCount, bool bFirstLoad)
{
    if (!m_streamData.atEnd()) {
        int count = 0;
//      QString strBuffer = m_pEditDebugInfo->toPlainText();
        while (!m_streamData.atEnd()) {
            QString strLine = m_streamData.readLine();
            strLine.replace('"', '\''); // " -> '
            QRegExp re("(\\d+-\\d+-\\d+T\\d+:\\d+:\\d+)[:.]{1}\\d+Z\\s+"
                       "\\{'name':'ActLineNumber','value':'(\\d+)'\\}");
            if (re.indexIn(strLine) != -1) {
                QStringList list = re.capturedTexts();
//              strBuffer += QString("%1\t%2\t%3\n").arg(list[1], list[2]).arg(m_nLineCount+1);
                m_pLineSeries->append(QDateTime::fromString(list[1], "yyyy-MM-ddTHH:mm:ss").toMSecsSinceEpoch(), list[2].toInt());
                ++count; m_nLineCount++;
                if (count == nLineCount) break;
            }
        }
        if (m_streamData.atEnd()) {
            QMessageBox::information(this, "Info", "Reaches end of file!");
            m_fileData.close();
        }
//      m_pEditDebugInfo->setText(strBuffer);
        if (bFirstLoad) {
            m_pAxisX->setRange(QDateTime::fromMSecsSinceEpoch(m_nMinX), QDateTime::fromMSecsSinceEpoch(m_nMaxX));
        } else {
            m_pAxisX->setMax(QDateTime::fromMSecsSinceEpoch(m_nMaxX));
        }
        m_pAxisY->setRange(0, m_nMaxY);
    }
}
