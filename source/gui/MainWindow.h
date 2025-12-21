#pragma once

#include <QMainWindow>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
  class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateDatabaseRequested();
    void onOpenDatabaseRequested();

private:
    Ui::MainWindow *ui;
    QStackedWidget *m_stackedWidget;
};