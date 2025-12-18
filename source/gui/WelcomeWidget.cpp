#include "WelcomeWidget.h"
#include "ui_WelcomeWidget.h"

#include <QMessageBox>
#include <QListWidgetItem>

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
    QMessageBox::information(this, "Create Database", "Create Database clicked");
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
