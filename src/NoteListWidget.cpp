#include "NoteListWidget.h"
#include "DatabaseManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>

NoteListWidget::NoteListWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索标题...");
    m_searchEdit->setStyleSheet("padding: 8px; border: none; border-bottom: 1px solid #3c3f45; background: #1e1f22; color: #e4e6eb;");
    layout->addWidget(m_searchEdit);

    QHBoxLayout* sortLayout = new QHBoxLayout();
    sortLayout->setContentsMargins(8, 4, 8, 4);
    m_sortNameBtn = new QPushButton("按名称", this);
    m_sortTimeBtn = new QPushButton("按时间", this);
    m_sortNameBtn->setCheckable(true);
    m_sortTimeBtn->setCheckable(true);
    m_sortTimeBtn->setChecked(true);
    sortLayout->addWidget(m_sortNameBtn);
    sortLayout->addWidget(m_sortTimeBtn);
    sortLayout->addStretch();
    layout->addLayout(sortLayout);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet("QListWidget { background: #2b2d31; }"
                                "QListWidget::item { padding: 12px; border-bottom: 1px solid #3c3f45; }"
                                "QListWidget::item:selected { background-color: #3c3f45; }");
    layout->addWidget(m_listWidget);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &NoteListWidget::onSearchTextChanged);
    connect(m_sortNameBtn, &QPushButton::clicked, this, &NoteListWidget::onSortByName);
    connect(m_sortTimeBtn, &QPushButton::clicked, this, &NoteListWidget::onSortByTime);
    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        int noteId = item->data(Qt::UserRole).toInt();
        emit noteSelected(noteId);
    });

    refreshNotes();
}

void NoteListWidget::refreshNotes()
{
    m_allNotes = DatabaseManager::instance().getAllNotes();
    applyFilterAndSort();
}

void NoteListWidget::clearSelection()
{
    m_listWidget->clearSelection();
}

void NoteListWidget::selectNoteById(int noteId)
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (item && item->data(Qt::UserRole).toInt() == noteId) {
            m_listWidget->setCurrentItem(item);
            m_listWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            break;
        }
    }
}

void NoteListWidget::onSearchTextChanged(const QString& text)
{
    m_keyword = text;
    applyFilterAndSort();
}

void NoteListWidget::onSortByName()
{
    m_sortMode = SortByName;
    m_sortNameBtn->setChecked(true);
    m_sortTimeBtn->setChecked(false);
    applyFilterAndSort();
}

void NoteListWidget::onSortByTime()
{
    m_sortMode = SortByTime;
    m_sortNameBtn->setChecked(false);
    m_sortTimeBtn->setChecked(true);
    applyFilterAndSort();
}

void NoteListWidget::applyFilterAndSort()
{
    int currentSelectedId = -1;
    QListWidgetItem* currentItem = m_listWidget->currentItem();
    if (currentItem) {
        currentSelectedId = currentItem->data(Qt::UserRole).toInt();
    }
    
    QList<NoteData> filtered;
    for (const NoteData& note : m_allNotes) {
        if (m_keyword.isEmpty() || note.title.contains(m_keyword, Qt::CaseInsensitive))
            filtered.append(note);
    }

    if (m_sortMode == SortByName) {
        std::sort(filtered.begin(), filtered.end(), [](const NoteData& a, const NoteData& b) {
            return a.title < b.title;
        });
    } else {
        std::sort(filtered.begin(), filtered.end(), [](const NoteData& a, const NoteData& b) {
            return a.updateTime > b.updateTime;
        });
    }

    m_listWidget->clear();
    for (const NoteData& note : filtered) {
        QListWidgetItem* item = new QListWidgetItem(note.title);
        item->setData(Qt::UserRole, note.id);
        m_listWidget->addItem(item);
        
        if (note.id == currentSelectedId) {
            m_listWidget->setCurrentItem(item);
        }
    }
    
    if (m_listWidget->currentItem() == nullptr && m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}