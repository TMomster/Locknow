#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) m_db.close();
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initDatabase(const QString& path)
{
    QString dbPath = path;
    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/MomsterTech/Locknow/kbase";
        QDir dir;
        if (!dir.exists(dataDir))
            dir.mkpath(dataDir);
        dbPath = dataDir + "/knowledge.db";
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    QSqlQuery pragma("PRAGMA case_sensitive_like = OFF;");
    pragma.exec();
    return createTables();
}

bool DatabaseManager::createTables()
{
    QString sqlNotes = R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL COLLATE NOCASE UNIQUE,
            content TEXT NOT NULL,
            create_time INTEGER NOT NULL,
            update_time INTEGER NOT NULL
        )
    )";
    return execQuery(sqlNotes);
}

bool DatabaseManager::execQuery(const QString& sql)
{
    QSqlQuery query;
    if (!query.exec(sql)) {
        qWarning() << "SQL error:" << query.lastError().text() << "\nSQL:" << sql;
        return false;
    }
    return true;
}

QList<NoteData> DatabaseManager::getAllNotes()
{
    QSqlQuery query("SELECT id, title, content, create_time, update_time FROM notes ORDER BY update_time DESC");
    QList<NoteData> notes;
    while (query.next()) {
        NoteData note;
        note.id = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.content = query.value(2).toString();
        note.createTime = QDateTime::fromSecsSinceEpoch(query.value(3).toLongLong());
        note.updateTime = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        notes.append(note);
    }
    return notes;
}

NoteData DatabaseManager::getNoteById(int id)
{
    QSqlQuery query;
    query.prepare("SELECT id, title, content, create_time, update_time FROM notes WHERE id = ?");
    query.bindValue(0, id);
    if (query.exec() && query.next()) {
        NoteData note;
        note.id = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.content = query.value(2).toString();
        note.createTime = QDateTime::fromSecsSinceEpoch(query.value(3).toLongLong());
        note.updateTime = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        return note;
    }
    return {};
}

NoteData DatabaseManager::getNoteByTitle(const QString& title)
{
    QSqlQuery query;
    query.prepare("SELECT id, title, content, create_time, update_time FROM notes WHERE title = ? COLLATE NOCASE");
    query.bindValue(0, title);
    if (query.exec() && query.next()) {
        NoteData note;
        note.id = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.content = query.value(2).toString();
        note.createTime = QDateTime::fromSecsSinceEpoch(query.value(3).toLongLong());
        note.updateTime = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        return note;
    }
    return {};
}

bool DatabaseManager::saveNote(NoteData& note)
{
    bool isNew = (note.id == -1);
    QDateTime now = QDateTime::currentDateTime();
    if (isNew) {
        note.createTime = now;
        note.updateTime = now;
    } else {
        note.updateTime = now;
    }

    QSqlQuery query;
    if (isNew) {
        query.prepare("INSERT INTO notes (title, content, create_time, update_time) VALUES (?, ?, ?, ?)");
        query.bindValue(0, note.title);
        query.bindValue(1, note.content);
        query.bindValue(2, note.createTime.toSecsSinceEpoch());
        query.bindValue(3, note.updateTime.toSecsSinceEpoch());
        if (!query.exec()) return false;
        note.id = query.lastInsertId().toInt();
    } else {
        query.prepare("UPDATE notes SET title=?, content=?, update_time=? WHERE id=?");
        query.bindValue(0, note.title);
        query.bindValue(1, note.content);
        query.bindValue(2, note.updateTime.toSecsSinceEpoch());
        query.bindValue(3, note.id);
        if (!query.exec()) return false;
    }
    return true;
}

bool DatabaseManager::deleteNote(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE id = ?");
    query.bindValue(0, id);
    return query.exec();
}

bool DatabaseManager::isTitleDuplicate(const QString& title, int excludeId)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM notes WHERE title = ? COLLATE NOCASE");
    query.bindValue(0, title);
    if (query.exec() && query.next()) {
        int id = query.value(0).toInt();
        return (id != excludeId);
    }
    return false;
}

int DatabaseManager::getTotalNotesCount()
{
    QSqlQuery query("SELECT COUNT(*) FROM notes");
    if (query.next()) return query.value(0).toInt();
    return 0;
}