#include <QApplication>
#include <QFile>
#include <QStyleFactory>

#include "./gui/MainWindow.h"

void setDarkTheme(QApplication& app) {
    // Set the application style to a dark palette
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Create a dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Text, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    
    // Apply the dark palette
    app.setPalette(darkPalette);
    
    // Set style sheet for the application
    app.setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
        "QMenu::item:selected { background-color: #2a82da; }"
        "QMenuBar::item:selected { background-color: #2a82da; }"
        "QTabBar::tab:selected { background: #2a82da; }"
        "QScrollBar:vertical { background: #2d2d2d; width: 12px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #4a4a4a; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar:horizontal { background: #2d2d2d; height: 12px; margin: 0px; }"
        "QScrollBar::handle:horizontal { background: #4a4a4a; min-width: 20px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
    );
}

int main(int argc, char *argv[]) {
    // Create Qt application instance
    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("KeeBox");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("KeeBox");
    
    // Apply dark theme
    setDarkTheme(app);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    // Start Qt event loop
    return app.exec();
}
