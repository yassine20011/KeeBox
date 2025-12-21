#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
  class CreateDatabaseDialog;
}

class CreateDatabaseDialog : public QDialog {
  Q_OBJECT

  public:
    explicit CreateDatabaseDialog(QWidget *parent = nullptr);
    ~CreateDatabaseDialog();

    QString getDatabaseName() const;
    QString getPassword() const;
    QString getFilePath() const;

    private slots:
      void onBrowseButtonClicked();
    void validateInputs();

  private:
    Ui::CreateDatabaseDialog *ui;
};