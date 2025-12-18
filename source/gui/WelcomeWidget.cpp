#include "./WelcomeWidget.h"
#include "./ui_WelcomeWidget.h"
#include "../database/CreateDatabaseDialog.h"

#include <QMessageBox>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WelcomeWidget)
{
    ui->setupUi(this);

    // Connect UI signals
    connect(ui->buttonNewDatabase, &QPushButton::clicked, this, &WelcomeWidget::onNewDatabase);
    connect(ui->buttonOpenDatabase, &QPushButton::clicked, this, &WelcomeWidget::onOpenDatabase);
    connect(ui->recentListWidget, &QListWidget::itemClicked, this, &WelcomeWidget::onRecentItemClicked);

    // Example recent databases
    ui->recentListWidget->addItem("ExampleDB1.kdbx");
    ui->recentListWidget->addItem("ExampleDB2.kdbx");
}

WelcomeWidget::~WelcomeWidget()
{
    delete ui;
}

void WelcomeWidget::onNewDatabase()
{
    CreateDatabaseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString filePath = dialog.getFilePath();
        QString password = dialog.getPassword();
        
        if (createNewDatabase(filePath, password)) {
            QMessageBox::information(this, tr("Success"), 
                tr("Database created successfully at:\n%1").arg(filePath));
            
            // Add to recent databases
            QString fileName = QFileInfo(filePath).fileName();
            ui->recentListWidget->insertItem(0, fileName);
        } else {
            QMessageBox::critical(this, tr("Error"), 
                tr("Failed to create database. Please try again."));
        }
    }
}

bool WelcomeWidget::createNewDatabase(const QString &filePath, const QString &password)
{
    // Remove the database if it already exists
    if (QFile::exists(filePath)) {
        if (!QFile::remove(filePath)) {
            return false;
        }
    }

    // Create parent directory if it doesn't exist
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.absolutePath());

    // Set up the database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath);
    
    // Set the encryption key (password)
    db.setPassword(password);
    
    // Open the database
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    // Set SQLCipher encryption
    QSqlQuery query;
    if (!query.exec(QString("PRAGMA key = '%1';").arg(password))) {
        qWarning() << "Failed to set encryption key:" << query.lastError().text();
        db.close();
        return false;
    }

    // Rekey the database (this is important for SQLCipher)
    if (!query.exec(QString("PRAGMA rekey = '%1';").arg(password))) {
        qWarning() << "Failed to rekey database:" << query.lastError().text();
        db.close();
        return false;
    }

    // Create necessary tables
    QStringList createTableQueries = {
        "CREATE TABLE IF NOT EXISTS entries ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "username TEXT, "
        "password TEXT NOT NULL, "
        "url TEXT, "
        "notes TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")",
        
        "CREATE TRIGGER IF NOT EXISTS update_entries_timestamp "
        "AFTER UPDATE ON entries "
        "BEGIN "
        "   UPDATE entries SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id; "
        "END;"
    };

    for (const QString &sql : createTableQueries) {
        if (!query.exec(sql)) {
            qWarning() << "Failed to create table:" << query.lastError().text();
            db.close();
            return false;
        }
    }

    db.close();
    return true;
}

void WelcomeWidget::onOpenDatabase()
{
    QMessageBox::information(this, "Open Database", "Open Database clicked");
}

void WelcomeWidget::onImportFile()
{
    QMessageBox::information(this, "Import File", "Import File clicked");
}

void WelcomeWidget::onRecentItemClicked(QListWidgetItem *item)
{
    QMessageBox::information(this, "Recent Database", "Selected: " + item->text());
}
