#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QTextStream>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QLineEdit;
class QTextEdit;
class QPushButton;

namespace QtCharts {
class QLineSeries;
class QDateTimeAxis;
class QValueAxis;
class QChart;
class QChartView;
}

using namespace QtCharts;

class CChartView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    void ResetMinMaxValues();
    void ClearData();

private slots:
    void ChooseFile();
    void DrawChart();
    void PointAdded(int index);
    void LoadMoreData(int nLineCount = 100, bool bFirstLoad = false);

private:
    Ui::MainWindow *ui;
    QLineEdit   *m_pEditFilePath;
    QTextEdit   *m_pEditDebugInfo;
    QPushButton *m_pBtnFileChooser;
    QPushButton *m_pBtnDrawChart;
    QPushButton *m_pBtnToggleDebug;
    CChartView    *m_pChartView;
    QChart        *m_pChart;
    QLineSeries   *m_pLineSeries;
    QDateTimeAxis *m_pAxisX;
    QValueAxis    *m_pAxisY;
    qint64 m_nMinX;
    qint64 m_nMaxX;
    qint64 m_nMaxY;
    qint32 m_nLineCount;
    QString m_strFilePath;
    QString m_strDebugInfo;
    QFile m_fileData;
    QTextStream m_streamData;
};

#endif // MAINWINDOW_H
