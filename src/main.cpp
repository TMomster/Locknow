#include <QApplication>
#include <QFontDatabase>
#include "MainWindow.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Locknow");
    app.setOrganizationName("MomsterTech");

    if (!DatabaseManager::instance().initDatabase()) {
        qCritical() << "数据库初始化失败";
        return -1;
    }

    QFont defaultFont("Microsoft YaHei", 9);
    app.setFont(defaultFont);

    MainWindow window;
    window.show();

    return app.exec();
}