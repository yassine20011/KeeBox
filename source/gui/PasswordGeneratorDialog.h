#pragma once

#include <QDialog>
#include "../utils/PasswordGenerator.h"

namespace Ui {
class PasswordGeneratorDialog;
}

class PasswordGeneratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordGeneratorDialog(QWidget *parent = nullptr);
    ~PasswordGeneratorDialog();

    QString getGeneratedPassword() const;

private slots:
    void onGenerateClicked();
    void updatePreview();

private:
    Ui::PasswordGeneratorDialog *ui;
};
