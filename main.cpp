#include "mainwindow.h"
#include "database.h"


#include <QApplication>
#include <QMessageBox>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // 1. Initializing the database
    DataBase db;
    if (!db.isInitialized()) {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize the database!");
        return -1;
    }


    MainWindow w;
    w.show();
    return a.exec();
}
