#include "chartview.h"

CChartView::CChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent)
{

}

CChartView::~CChartView()
{}

void CChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        chart()->scroll(10, 0);
        emit LoadMoreData(10);
        break;
    case Qt::Key_Right:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, -10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, 10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}
