#ifndef NOTELISTWIDGET_H
#define NOTELISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "DatabaseManager.h"

class NoteListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoteListWidget(QWidget *parent = nullptr);
    void refreshNotes();
    void clearSelection();
    void selectNoteById(int noteId);

signals:
    void noteSelected(int noteId);

private slots:
    void onSearchTextChanged(const QString& text);
    void onSortByName();
    void onSortByTime();

private:
    void applyFilterAndSort();

    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_sortNameBtn = nullptr;
    QPushButton* m_sortTimeBtn = nullptr;
    QListWidget* m_listWidget = nullptr;

    QList<NoteData> m_allNotes;
    QString m_keyword;
    enum SortMode { SortByName, SortByTime };
    SortMode m_sortMode = SortByTime;
};

#endif // NOTELISTWIDGET_H