#ifndef COLOREDHEADERDELEGATE_H
#define COLOREDHEADERDELEGATE_H

#include <QHeaderView>
#include <QPainter>
#include <QSet>
#include <QString>

class ColoredHeaderDelegate : public QHeaderView {
    Q_OBJECT
public:
    ColoredHeaderDelegate(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent) {}

    void setColoredRows(const QSet<int>& rows) {
        coloredRows = rows;
        viewport()->update();
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override {
        painter->save();

        if (coloredRows.contains(logicalIndex)) {
            painter->setPen(QColor("#05c7f2")); // 네온블루
        } else {
            painter->setPen(QColor("#ffffff")); // 기본 흰색
        }

        painter->drawText(rect, Qt::AlignCenter, QString::number(logicalIndex + 1));
        painter->restore();
    }

private:
    QSet<int> coloredRows;
};

#endif // COLOREDHEADERDELEGATE_H
