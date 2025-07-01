#include "mainwindow.h"
#include "database.h"

#include <QMessageBox>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Database initialization
    DataBase db;
    if (!db.isInitialized()) {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize the database!");
        return -1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
