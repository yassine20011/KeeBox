#include "./MainWindow.h"
#include "./ui_MainWindow.h"
#include "./WelcomeWidget.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  WelcomeWidget *welcome = new WelcomeWidget(this);
  ui->stackedWidget->addWidget(welcome);
  ui->stackedWidget->setCurrentWidget(welcome);
}

MainWindow::~MainWindow() {
  delete ui;
}
