#pragma once

#include <QObject>
#include <sqlite3.h>
#include <QString>
#include <QList>

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    static DatabaseManager& instance();

    struct Group {
        int id;
        QString name;
        int parentId;
    };

    struct Entry {
        int id;
        int groupId;
        QString title;
        QString username;
        QString password;
        QString url;
        QString notes;
    };

    // Database Logic
    void ensureRootGroup();
    QList<Group> getGroups(int parentId = 0);
    QList<Entry> getEntries(int groupId);
    QList<Entry> searchEntries(const QString& query);
    int createGroup(const QString& name, int parentId = 0);
    bool updateGroup(int id, const QString& name);
    bool deleteGroup(int id);

    int createEntry(const Entry& entry);
    bool updateEntry(const Entry& entry);
    bool deleteEntry(int id);

    bool createDatabase(const QString& path, const QString& password);
    bool openDatabase(const QString& path, const QString& password);
    void closeDatabase();
    bool isOpen() const;

private:
    DatabaseManager();
    ~DatabaseManager() override;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    sqlite3* m_db = nullptr;
};