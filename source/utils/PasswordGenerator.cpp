#include "PasswordGenerator.h"
#include <QRandomGenerator>
#include <vector>

QString PasswordGenerator::generate(const Options& options) {
    QString uppercaseChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString lowercaseChars = "abcdefghijklmnopqrstuvwxyz";
    QString numberChars = "0123456789";
    QString symbolChars = "!@#$%^&*()-_=+[]{}|;:,.<>?/";

    QString pool = "";
    QString manualRequired = ""; // To ensure at least one from each selected set

    if (options.useUppercase) pool += uppercaseChars;
    if (options.useLowercase) pool += lowercaseChars;
    if (options.useNumbers) pool += numberChars;
    if (options.useSymbols) pool += symbolChars;

    if (pool.isEmpty()) return "";

    QString password = "";
    auto* rng = QRandomGenerator::global();

    for (int i = 0; i < options.length; ++i) {
        int index = rng->bounded(pool.length());
        password += pool.at(index);
    }

    // Optional: Ensure at least one of each type if space allows
    // For now, simple random selection is usually sufficient if length is reasonable.
    
    return password;
}
