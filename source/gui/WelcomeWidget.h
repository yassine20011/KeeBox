#pragma once

#include <QWidget>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class WelcomeWidget; }
QT_END_NAMESPACE

class WelcomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);
    ~WelcomeWidget();

private slots:
    void onNewDatabase();
    void onOpenDatabase();
    void onImportFile();
    void onRecentItemClicked(QListWidgetItem *item);

private:
    Ui::WelcomeWidget *ui;
};
