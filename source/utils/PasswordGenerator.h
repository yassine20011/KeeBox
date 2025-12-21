#pragma once

#include <QString>

class PasswordGenerator {
public:
    struct Options {
        int length = 16;
        bool useUppercase = true;
        bool useLowercase = true;
        bool useNumbers = true;
        bool useSymbols = true;
    };

    static QString generate(const Options& options);
};
