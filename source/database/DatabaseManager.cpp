#include "DatabaseManager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

void DatabaseManager::ensureRootGroup() {
    if (!m_db) return;
    
    // Check if any group exists
    sqlite3_stmt* stmt;
    const char* query = "SELECT count(*) FROM groups";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return;
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        createGroup("Root", 0);
    }
}

int DatabaseManager::createGroup(const QString& name, int parentId) {
    if (!m_db) return -1;
    
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO groups (name, parent_id) VALUES (?, ?)";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    
    sqlite3_bind_text(stmt, 1, name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    if (parentId > 0) {
        sqlite3_bind_int(stmt, 2, parentId);
    } else {
        sqlite3_bind_null(stmt, 2);
    }
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(m_db);
}

QList<DatabaseManager::Group> DatabaseManager::getGroups(int parentId) {
    QList<Group> list;
    if (!m_db) return list;
    
    sqlite3_stmt* stmt;
    QString sql;
    if (parentId == 0) {
        sql = "SELECT id, name, parent_id FROM groups WHERE parent_id IS NULL";
    } else {
        sql = "SELECT id, name, parent_id FROM groups WHERE parent_id = ?";
    }
    
    if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) != SQLITE_OK) {
        qCritical() << "Failed to prepare getGroups:" << sqlite3_errmsg(m_db);
        return list;
    }
    
    if (parentId > 0) {
        sqlite3_bind_int(stmt, 1, parentId);
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Group g;
        g.id = sqlite3_column_int(stmt, 0);
        g.name = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        g.parentId = sqlite3_column_int(stmt, 2);
        list.append(g);
    }
    
    sqlite3_finalize(stmt);
    return list;
}

QList<DatabaseManager::Entry> DatabaseManager::getEntries(int groupId) {
    QList<Entry> list;
    if (!m_db) return list;
    
    const char* query = "SELECT id, group_id, title, username, password, url, notes FROM entries WHERE group_id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return list;
    
    sqlite3_bind_int(stmt, 1, groupId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Entry e;
        e.id = sqlite3_column_int(stmt, 0);
        e.groupId = sqlite3_column_int(stmt, 1);
        e.title = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 2));
        e.username = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 3));
        e.password = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 4));
        e.url = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 5));
        e.notes = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 6));
        list.append(e);
    }
    
    sqlite3_finalize(stmt);
    return list;
}

bool DatabaseManager::updateGroup(int id, const QString& name) {
    if (!m_db) return false;
    
    sqlite3_stmt* stmt;
    const char* query = "UPDATE groups SET name = ? WHERE id = ?";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::deleteGroup(int id) {
    if (!m_db) return false;
    
    sqlite3_stmt* stmt;
    const char* query = "DELETE FROM groups WHERE id = ?";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

int DatabaseManager::createEntry(const Entry& entry) {
    if (!m_db) return -1;
    
    sqlite3_stmt* stmt;
    const char* query = "INSERT INTO entries (group_id, title, username, password, url, notes) VALUES (?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    
    sqlite3_bind_int(stmt, 1, entry.groupId);
    sqlite3_bind_text(stmt, 2, entry.title.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, entry.username.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, entry.password.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, entry.url.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, entry.notes.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(m_db);
}

bool DatabaseManager::updateEntry(const Entry& entry) {
    if (!m_db) return false;
    
    sqlite3_stmt* stmt;
    const char* query = "UPDATE entries SET title = ?, username = ?, password = ?, url = ?, notes = ?, modified_at = CURRENT_TIMESTAMP WHERE id = ?";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, entry.title.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, entry.username.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, entry.password.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, entry.url.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, entry.notes.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, entry.id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::deleteEntry(int id) {
    if (!m_db) return false;
    
    sqlite3_stmt* stmt;
    const char* query = "DELETE FROM entries WHERE id = ?";
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::createDatabase(const QString& path, const QString& password) {
    if (path.isEmpty() || password.isEmpty()) {
        qWarning() << "Database creation failed: Path or password empty";
        return false;
    }
    
    // Ensure we start fresh
    closeDatabase();

    // Remove existing file if it exists to ensure overwrite
    if (QFile::exists(path)) {
        if (!QFile::remove(path)) {
            qCritical() << "Failed to remove existing file:" << path;
            return false;
        }
    }

    // Open/Create the database
    // SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
    int rc = sqlite3_open_v2(path.toUtf8().constData(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to create database file:" << sqlite3_errmsg(m_db);
        closeDatabase(); // Cleans up m_db if it was allocated
        return false;
    }

    // Apply encryption immediately
    QByteArray pwdBytes = password.toUtf8();
    rc = sqlite3_key(m_db, pwdBytes.constData(), pwdBytes.length());
    if (rc != SQLITE_OK) {
         qCritical() << "Failed to set PRAGMA key:" << sqlite3_errmsg(m_db);
         closeDatabase();
         return false;
    }
    
    // Set compatibility (OPTIONAL/RECOMMENDED depending on sqlcipher version)
    // We use exec for pragmas
    char* errMsg = nullptr;
    rc = sqlite3_exec(m_db, "PRAGMA cipher_compatibility = 4;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to set cipher compatibility:" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }

    // Verify encryption works by creating a table
    rc = sqlite3_exec(m_db, "CREATE TABLE creation_check (id INTEGER PRIMARY KEY);", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to create check table (Encryption might be failing):" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }
    
    sqlite3_exec(m_db, "DROP TABLE creation_check;", nullptr, nullptr, nullptr);
    
    errMsg = nullptr;
    rc = sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to enable foreign keys:" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
    }
    
    // Initialize Schema
    QStringList schemaQueries;
    schemaQueries << "CREATE TABLE IF NOT EXISTS groups (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, parent_id INTEGER, FOREIGN KEY(parent_id) REFERENCES groups(id) ON DELETE CASCADE);"
                  << "CREATE TABLE IF NOT EXISTS entries (id INTEGER PRIMARY KEY AUTOINCREMENT, group_id INTEGER NOT NULL, title TEXT NOT NULL, username TEXT, password TEXT, url TEXT, notes TEXT, created_at DATETIME DEFAULT CURRENT_TIMESTAMP, modified_at DATETIME DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY(group_id) REFERENCES groups(id) ON DELETE CASCADE);";

    for (const QString& q : schemaQueries) {
        rc = sqlite3_exec(m_db, q.toUtf8().constData(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            qCritical() << "Failed to execute schema query:" << q << "Error:" << (errMsg ? errMsg : "Unknown error");
            sqlite3_free(errMsg);
            closeDatabase();
            if (QFile::exists(path)) QFile::remove(path);
            return false; // Fail complete
        }
    }

    // SECURITY VERIFICATION:
    // Close the DB and check the file header to ensure it's encrypted.
    closeDatabase();
    
    if (!openDatabase(path, password)) {
        return false;
    }
    
    ensureRootGroup();

    return true;
}

bool DatabaseManager::openDatabase(const QString& path, const QString& password) {
    if (!QFile::exists(path)) {
        qWarning() << "Database file does not exist:" << path;
        return false;
    }

    // SECURITY CHECK: Reject plain text SQLite files
    {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray header = file.read(16);
            file.close();
            if (header.startsWith("SQLite format 3")) {
                 qCritical() << "SECURITY ALERT: Attempted to open a PLAIN TEXT database. Access Denied.";
                 return false;
            }
        }
    }

    closeDatabase();

    int rc = sqlite3_open_v2(path.toUtf8().constData(), &m_db, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to open database:" << (m_db ? sqlite3_errmsg(m_db) : "Unknown error");
        closeDatabase();
        return false;
    }

    QByteArray pwdBytes = password.toUtf8();
    rc = sqlite3_key(m_db, pwdBytes.constData(), pwdBytes.length());
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to set key:" << sqlite3_errmsg(m_db);
        closeDatabase();
        return false;
    }

    char* errMsg = nullptr;
    rc = sqlite3_exec(m_db, "PRAGMA cipher_compatibility = 4;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qCritical() << "Failed to set compatibility:" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }

    rc = sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to enable foreign keys:" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
    }

    // Verify correctness: Try to read sqlite_master
    rc = sqlite3_exec(m_db, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Verification failed (Wrong Password?):" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }
    
    ensureRootGroup();
    
    return true;
}

void DatabaseManager::closeDatabase() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool DatabaseManager::isOpen() const {
    return m_db != nullptr;
}