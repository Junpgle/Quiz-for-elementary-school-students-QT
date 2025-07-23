#include "mainwindow.h"
#include <QApplication>
#include <QDir> // 确保包含目录操作头文件

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 创建用户目录
    QDir().mkdir("data");

    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow;
        mainWindow.setStudent(loginDialog.getStudent());
        mainWindow.show();
        return app.exec();
    }

    return 0;
}
