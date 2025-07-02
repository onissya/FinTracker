#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <qspinbox.h>
#include <QComboBox>

#include "database.h"


#include <QTableWidgetItem>

// Добавьте этот класс в mainwindow.h
class TypeTableItem : public QTableWidgetItem {
public:
    TypeTableItem(const QString& text) : QTableWidgetItem(text) {}

    bool operator<(const QTableWidgetItem& other) const override {
        // Специальная логика сортировки для столбца "Type"
        QString thisText = text();
        QString otherText = other.text();

        // Порядок сортировки: Доход -> Расход -> другие значения
        if (thisText == "Доход" && otherText != "Доход") return true;
        if (thisText == "Расход" && otherText == "Доход") return false;

        return QTableWidgetItem::operator<(other);
    }
};

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
