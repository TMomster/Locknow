#include "TagsWidget.h"
#include "DatabaseManager.h"
#include <QLabel>
#include <QVBoxLayout>

TagsWidget::TagsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 8, 4, 8);
    QLabel* title = new QLabel("标签", this);
    title->setStyleSheet("font-weight: bold; color: #9aa0a9; font-size:12px; margin-bottom: 4px;");
    layout->addWidget(title);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet("QListWidget::item { padding: 4px 8px; border-radius: 4px; }"
                                "QListWidget::item:selected { background-color: #404249; }");
    m_listWidget->setContextMenuPolicy(Qt::NoContextMenu);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::itemClicked, this, &TagsWidget::onItemClicked);
    refreshTags();
}

void TagsWidget::refreshTags()
{
    m_listWidget->clear();
    auto tagCount = DatabaseManager::instance().getTagsWithCount();
    for (auto it = tagCount.begin(); it != tagCount.end(); ++it) {
        QString text = QString("%1 (%2)").arg(it.key()).arg(it.value());
        m_listWidget->addItem(text);
        m_listWidget->item(m_listWidget->count()-1)->setData(Qt::UserRole, it.key());
    }
}

void TagsWidget::clearSelection()
{
    m_listWidget->clearSelection();
}

void TagsWidget::onItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    QString tag = item->data(Qt::UserRole).toString();
    emit tagSelected(tag);
}