#include "PasswordGeneratorDialog.h"
#include "ui_PasswordGeneratorDialog.h"

PasswordGeneratorDialog::PasswordGeneratorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordGeneratorDialog)
{
    ui->setupUi(this);

    connect(ui->lengthSlider, &QSlider::valueChanged, ui->lengthSpinBox, &QSpinBox::setValue);
    connect(ui->lengthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->lengthSlider, &QSlider::setValue);
    
    connect(ui->lengthSlider, &QSlider::valueChanged, this, &PasswordGeneratorDialog::updatePreview);
    connect(ui->upperCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::updatePreview);
    connect(ui->lowerCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::updatePreview);
    connect(ui->numbersCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::updatePreview);
    connect(ui->symbolsCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::updatePreview);
    
    connect(ui->generateButton, &QPushButton::clicked, this, &PasswordGeneratorDialog::onGenerateClicked);

    updatePreview();
}

PasswordGeneratorDialog::~PasswordGeneratorDialog()
{
    delete ui;
}

QString PasswordGeneratorDialog::getGeneratedPassword() const
{
    return ui->passwordPreview->text();
}

void PasswordGeneratorDialog::onGenerateClicked()
{
    updatePreview();
}

void PasswordGeneratorDialog::updatePreview()
{
    PasswordGenerator::Options options;
    options.length = ui->lengthSpinBox->value();
    options.useUppercase = ui->upperCheck->isChecked();
    options.useLowercase = ui->lowerCheck->isChecked();
    options.useNumbers = ui->numbersCheck->isChecked();
    options.useSymbols = ui->symbolsCheck->isChecked();

    ui->passwordPreview->setText(PasswordGenerator::generate(options));
}
