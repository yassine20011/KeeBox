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

    // Verify correctness: Try to read sqlite_master
    rc = sqlite3_exec(m_db, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Verification failed (Wrong Password?):" << (errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }
    
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