#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QDateTime>

struct NoteData {
    int id = -1;
    QString title;
    QString content;
    QDateTime createTime;
    QDateTime updateTime;
};

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();
    bool initDatabase(const QString& path = "");

    QList<NoteData> getAllNotes();
    NoteData getNoteById(int id);
    NoteData getNoteByTitle(const QString& title);
    bool saveNote(NoteData& note);
    bool deleteNote(int id);
    bool isTitleDuplicate(const QString& title, int excludeId = -1);
    int getTotalNotesCount();

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    bool createTables();
    bool execQuery(const QString& sql);
};

#endif // DATABASEMANAGER_H