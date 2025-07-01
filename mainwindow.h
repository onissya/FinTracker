#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <qspinbox.h>
#include <QComboBox>

#include "database.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addTransaction();
    void deleteTransaction();
    void updateBalance();
    void loadTransactions();
    void sortTable();
    void statsTable();

private:
    void setupUI();
    void setupConnections();

    // Виджеты
    QTableWidget *transactionsTable;
    QLineEdit *balanceEdit;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *statsButton;
    QComboBox *sortComboBox;

    DataBase *m_db;
};

#endif // MAINWINDOW_H
