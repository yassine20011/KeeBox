#pragma once

#include <QWidget>

namespace Ui {
class VaultWidget;
}

class VaultWidget : public QWidget {
    Q_OBJECT

public:
    explicit VaultWidget(QWidget *parent = nullptr);
    ~VaultWidget();

private:
    Ui::VaultWidget *ui;
};