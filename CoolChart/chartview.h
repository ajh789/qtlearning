#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>

using namespace QtCharts;

class CChartView : public QChartView
{
    Q_OBJECT

public:
    explicit CChartView(QChart *chart, QWidget *parent = 0);
    ~CChartView();

signals:
    void LoadMoreData(int nLineCount) const;

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // CHARTVIEW_H
