#pragma once

#include <QObject>
#include <sqlite3.h>
#include <QString>

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    static DatabaseManager& instance();

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