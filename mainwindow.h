#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
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
    void selectAllTransactions();
    void sortTable(int index);
    void updateBalance();
    void statsTable();

private:
    void setupUI();
    void setupConnections();
    void loadTransactions();

    DataBase *m_db;

    QTableWidget *transactionsTable;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *selectAllButton;
    QPushButton *statsButton;
    QComboBox *sortComboBox;
    QLineEdit *balanceEdit;
};

#endif // MAINWINDOW_H
