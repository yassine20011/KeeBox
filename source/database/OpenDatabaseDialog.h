#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
  class OpenDatabaseDialog;
}

class OpenDatabaseDialog : public QDialog {
  Q_OBJECT

  public:
    explicit OpenDatabaseDialog(QWidget *parent = nullptr);
    ~OpenDatabaseDialog();

    QString getFilePath() const;
    QString getPassword() const;

    private slots:
      void on_browseButton_clicked();
    void validateInput();

  private:
    Ui::OpenDatabaseDialog *ui;
};
