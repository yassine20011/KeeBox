#pragma once

#include <QWidget>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
  class WelcomeWidget;
}
QT_END_NAMESPACE

class WelcomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);
    ~WelcomeWidget();

signals:
    void createDatabaseRequested();
    void openDatabaseRequested();

private slots:
    void onNewDatabase();
    void onOpenDatabase();

private:
    Ui::WelcomeWidget *ui;
};