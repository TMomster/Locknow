#include "CategoryTreeWidget.h"
#include "DatabaseManager.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

CategoryTreeWidget::CategoryTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setIndentation(12);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &CategoryTreeWidget::showContextMenu);
    connect(this, &QTreeWidget::itemClicked, this, &CategoryTreeWidget::onItemSelected);
    refreshCategories();
}

void CategoryTreeWidget::refreshCategories()
{
    loadCategories();
}

void CategoryTreeWidget::loadCategories()
{
    clear();
    auto categories = DatabaseManager::instance().getAllCategories();

    QMap<int, QTreeWidgetItem*> itemMap;
    for (const auto& cat : categories) {
        auto item = new QTreeWidgetItem();
        item->setText(0, cat.name);
        setCategoryIdToItem(item, cat.id);
        itemMap[cat.id] = item;
    }
    for (const auto& cat : categories) {
        auto item = itemMap[cat.id];
        if (cat.parentId == 0 || !itemMap.contains(cat.parentId)) {
            addTopLevelItem(item);
        } else {
            itemMap[cat.parentId]->addChild(item);
        }
    }
    expandAll();
}

int CategoryTreeWidget::getCurrentCategoryId() const
{
    auto* item = currentItem();
    if (!item) return -1;
    return getCategoryIdFromItem(item);
}

void CategoryTreeWidget::clearSelection()
{
    setCurrentItem(nullptr);
}

void CategoryTreeWidget::onItemSelected(QTreeWidgetItem* item, int)
{
    if (!item) return;
    int id = getCategoryIdFromItem(item);
    emit categorySelected(id);
}

void CategoryTreeWidget::showContextMenu(const QPoint& pos)
{
    auto* item = itemAt(pos);
    QMenu menu;
    if (item) {
        menu.addAction("添加子分类", this, &CategoryTreeWidget::addCategory);
        menu.addAction("重命名", this, &CategoryTreeWidget::renameCategory);
        int catId = getCategoryIdFromItem(item);
        if (catId != 1)
            menu.addAction("删除", this, &CategoryTreeWidget::deleteCategory);
    } else {
        menu.addAction("添加根分类", this, &CategoryTreeWidget::addCategory);
    }
    menu.exec(viewport()->mapToGlobal(pos));
}

void CategoryTreeWidget::addCategory()
{
    auto* parentItem = currentItem();
    int parentId = 0;
    if (parentItem)
        parentId = getCategoryIdFromItem(parentItem);

    QString name = QInputDialog::getText(this, "新建分类", "分类名称:");
    if (name.isEmpty()) return;
    if (DatabaseManager::instance().addCategory(name, parentId)) {
        refreshCategories();
    } else {
        QMessageBox::warning(this, "错误", "创建分类失败");
    }
}

void CategoryTreeWidget::renameCategory()
{
    auto* item = currentItem();
    if (!item) return;
    int id = getCategoryIdFromItem(item);
    QString newName = QInputDialog::getText(this, "重命名", "新名称:", QLineEdit::Normal, item->text(0));
    if (newName.isEmpty() || newName == item->text(0)) return;
    if (DatabaseManager::instance().updateCategory(id, newName)) {
        item->setText(0, newName);
    } else {
        QMessageBox::warning(this, "错误", "重命名失败");
    }
}

void CategoryTreeWidget::deleteCategory()
{
    auto* item = currentItem();
    if (!item) return;
    int id = getCategoryIdFromItem(item);
    if (id == 1) {
        QMessageBox::warning(this, "提示", "无法删除默认分类“未分类”");
        return;
    }
    int ret = QMessageBox::question(this, "确认删除", "删除分类不会删除其中的笔记，笔记将移至“未分类”。确定删除？");
    if (ret != QMessageBox::Yes) return;
    if (DatabaseManager::instance().deleteCategory(id)) {
        refreshCategories();
    } else {
        QMessageBox::warning(this, "错误", "删除失败");
    }
}

int CategoryTreeWidget::getCategoryIdFromItem(QTreeWidgetItem* item) const
{
    return item->data(0, Qt::UserRole).toInt();
}

void CategoryTreeWidget::setCategoryIdToItem(QTreeWidgetItem* item, int id)
{
    item->setData(0, Qt::UserRole, id);
}