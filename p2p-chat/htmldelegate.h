#ifndef HTMLDELEGATE_H
#define HTMLDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief The HTMLDelegate class draws an item in a view with HTML formatting.
 * see the QStyledItemDelegate documentation for more information.
 */
class HTMLDelegate : public QStyledItemDelegate {
public:
    HTMLDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // HTMLDELEGATE_H
