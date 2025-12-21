#include "WelcomeWidget.h"
#include "ui_WelcomeWidget.h"

WelcomeWidget::WelcomeWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::WelcomeWidget) {
    ui->setupUi(this);

    // Correct IDs from WelcomeWidget.ui
    connect(ui->buttonNewDatabase, &QPushButton::clicked, this, &WelcomeWidget::onNewDatabase);
    connect(ui->buttonOpenDatabase, &QPushButton::clicked, this, &WelcomeWidget::onOpenDatabase);
}

WelcomeWidget::~WelcomeWidget() {
    delete ui;
}

void WelcomeWidget::onNewDatabase() {
    emit createDatabaseRequested();
}

void WelcomeWidget::onOpenDatabase() {
    emit openDatabaseRequested();
}