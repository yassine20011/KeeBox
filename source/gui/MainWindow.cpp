#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "WelcomeWidget.h"
#include "VaultWidget.h"
#include "../database/DatabaseManager.h"
#include "../database/CreateDatabaseDialog.h"
#include "../database/OpenDatabaseDialog.h"

#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Use the existing stacked widget from UI
    m_stackedWidget = ui->stackedWidget;
    
    // Clean up any existing pages (just in case, though usually empty or has placeholders)
    while (m_stackedWidget->count() > 0) {
        QWidget* w = m_stackedWidget->widget(0);
        m_stackedWidget->removeWidget(w);
        delete w;
    }

    // Create Page 1: Welcome
    WelcomeWidget* welcomePage = new WelcomeWidget(this);
    m_stackedWidget->addWidget(welcomePage);

    // Connect signals
    connect(welcomePage, &WelcomeWidget::createDatabaseRequested, this, &MainWindow::onCreateDatabaseRequested);
    connect(welcomePage, &WelcomeWidget::openDatabaseRequested, this, &MainWindow::onOpenDatabaseRequested);

    // Initial state
    m_stackedWidget->setCurrentWidget(welcomePage);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onCreateDatabaseRequested() {
    CreateDatabaseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.getFilePath();
        QString password = dialog.getPassword();

        // Strict Requirement: Apply SQLCipher key BEFORE tables created (handled by Manager)
        if (DatabaseManager::instance().createDatabase(path, password)) {
            // Success -> Go to Vault
            VaultWidget* vaultPage = new VaultWidget(this);
            m_stackedWidget->addWidget(vaultPage);
            m_stackedWidget->setCurrentWidget(vaultPage);
        } else {
            QMessageBox::critical(this, "Error", "Failed to create database. check logs.");
        }
    }
}

void MainWindow::onOpenDatabaseRequested() {
    OpenDatabaseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.getFilePath();
        QString password = dialog.getPassword();

        // Attempt unlock
        if (DatabaseManager::instance().openDatabase(path, password)) {
            // Success -> Go to Vault
            VaultWidget* vaultPage = new VaultWidget(this);
            m_stackedWidget->addWidget(vaultPage);
            m_stackedWidget->setCurrentWidget(vaultPage);
        } else {
            // Failure -> Stay on Welcome, Show Error
            QMessageBox::critical(this, "Authentication Failed", "Invalid password or corrupted database.");
        }
    }
}