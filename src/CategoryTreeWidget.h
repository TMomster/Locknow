#ifndef CATEGORYTREEWIDGET_H
#define CATEGORYTREEWIDGET_H

#include <QTreeWidget>

class CategoryTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CategoryTreeWidget(QWidget *parent = nullptr);
    void refreshCategories();
    int getCurrentCategoryId() const;
    void clearSelection();

signals:
    void categorySelected(int categoryId);

private slots:
    void onItemSelected(QTreeWidgetItem* item, int column);
    void showContextMenu(const QPoint& pos);
    void addCategory();
    void renameCategory();
    void deleteCategory();

private:
    void loadCategories();
    int getCategoryIdFromItem(QTreeWidgetItem* item) const;
    void setCategoryIdToItem(QTreeWidgetItem* item, int id);
};

#endif // CATEGORYTREEWIDGET_H