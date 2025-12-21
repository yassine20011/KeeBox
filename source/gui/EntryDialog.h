#pragma once

#include <QDialog>
#include "../database/DatabaseManager.h"

namespace Ui {
class EntryDialog;
}

class EntryDialog : public QDialog {
    Q_OBJECT

public:
    explicit EntryDialog(QWidget *parent = nullptr);
    ~EntryDialog();

    void setEntry(const DatabaseManager::Entry& entry);
    DatabaseManager::Entry getEntry() const;

private slots:
    void onShowPasswordToggled(bool checked);
    void onGeneratePasswordClicked();
    void validateInput();

private:
    Ui::EntryDialog *ui;
    int m_id = -1;
    int m_groupId = -1;
};
