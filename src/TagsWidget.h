#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>

class TagsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagsWidget(QWidget *parent = nullptr);
    void refreshTags();
    void clearSelection();

signals:
    void tagSelected(const QString& tag);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* m_listWidget = nullptr;
};

#endif // TAGSWIDGET_H