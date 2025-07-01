#include "mainwindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPalette>
#include <QMessageBox>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QSqlQuery>
#include <QTextBlock>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_db(new DataBase(this))
{
    setupUI();
    setupConnections();

    if (m_db->isInitialized()) {
        loadTransactions(); // Загружаем существующие транзакции
        updateBalance();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
    }
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    // Главный контейнер
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Таблица транзакций
    transactionsTable = new QTableWidget(this);
    transactionsTable->setColumnCount(4);
    transactionsTable->setHorizontalHeaderLabels({"Name", "Type", "Price", "Date"});
    transactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Панель кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add", this);
    deleteButton = new QPushButton("Delete", this);
    sortComboBox = new QComboBox(this);
    sortComboBox->addItem("Позже",0);
    sortComboBox->addItem("Раньше",1);
    sortComboBox->addItem("По убыванию цен",2);
    sortComboBox->addItem("По возрастанию цен",3);
    statsButton = new QPushButton("Statistic", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(sortComboBox);
    buttonLayout->addWidget(statsButton);

    // Поле баланса
    balanceEdit = new QLineEdit(this);
    balanceEdit->setReadOnly(true);
    balanceEdit->setAlignment(Qt::AlignRight);
    balanceEdit->setStyleSheet("font: 14pt; background: #f8f8f8; border: 1px solid #ccc;");
    balanceEdit->setText("Баланс: 0.00 $");

    // Компоновка
    mainLayout->addWidget(transactionsTable);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(balanceEdit);

    setCentralWidget(centralWidget);
    resize(800, 600);
}

void MainWindow::setupConnections()
{
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTransaction);
    connect(sortComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::sortTable);
    connect(statsButton, &QPushButton::clicked, this, &MainWindow::statsTable);
}

void MainWindow::addTransaction()
{
    // 1. Создаем диалог для ввода данных
    QDialog dialog(this);
    QFormLayout form(&dialog);

    QLineEdit nameEdit;
    QComboBox typeCombo;
    typeCombo.addItems({"Доход", "Расход"});
    QDoubleSpinBox amountEdit;
    amountEdit.setRange(-1e12, 1e12);  // Диапазон от -1 триллиона до +1 триллиона
    amountEdit.setDecimals(2);         // 2 знака после запятой
    amountEdit.setPrefix("$ ");        // Добавляем символ валюты
    QDateEdit dateEdit(QDate::currentDate());

    form.addRow("Наименование:", &nameEdit);
    form.addRow("Тип:", &typeCombo);
    form.addRow("Сумма:", &amountEdit);
    form.addRow("Дата:", &dateEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // 2. Если пользователь ввел данные
    if (dialog.exec() == QDialog::Accepted) {
        // 3. Добавляем в БД
        QSqlQuery query;
        query.prepare("INSERT INTO transactions (date, amount, category, type) "
                      "VALUES (:date, :amount, :category, :type)");
        query.bindValue(":date", dateEdit.date().toString("yyyy-MM-dd"));
        query.bindValue(":amount", amountEdit.value());
        query.bindValue(":category", nameEdit.text());
        query.bindValue(":type", typeCombo.currentText() == "Доход" ? "income" : "expense");

        if (query.exec()) {
            // 4. Обновляем интерфейс
            loadTransactions();
            updateBalance();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить транзакцию");
        }
    }
}

void MainWindow::loadTransactions()
{
    transactionsTable->setRowCount(0); // Очищаем таблицу

    QSqlQuery query("SELECT date, amount, category, type FROM transactions ORDER BY date DESC");

    while (query.next()) {
        int row = transactionsTable->rowCount();
        transactionsTable->insertRow(row);

        // Преобразуем тип для отображения
        QString typeDisplay = query.value("type").toString() == "income"
                                  ? "Доход" : "Расход";

        // Заполняем строку
        transactionsTable->setItem(row, 0, new QTableWidgetItem(query.value("category").toString()));
        transactionsTable->setItem(row, 1, new QTableWidgetItem(typeDisplay));
        transactionsTable->setItem(row, 2, new QTableWidgetItem(QString::number(query.value("amount").toDouble(), 'f', 2)));
        transactionsTable->setItem(row, 3, new QTableWidgetItem(QDate::fromString(query.value("date").toString(), "yyyy-MM-dd").toString("dd.MM.yyyy")));
    }
}

void MainWindow::deleteTransaction()
{
    QModelIndexList selected = transactionsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    int row = selected.first().row();
    QString date = transactionsTable->item(row, 3)->text();
    QString amount = transactionsTable->item(row, 2)->text();

    QSqlQuery query;
    query.prepare("DELETE FROM transactions WHERE date = :date AND amount = :amount");
    query.bindValue(":date", QDate::fromString(date, "dd.MM.yyyy").toString("yyyy-MM-dd"));
    query.bindValue(":amount", amount.toDouble());

    if (query.exec()) {
        loadTransactions();
        updateBalance();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить транзакцию");
    }
}

void MainWindow::sortTable() {
    int sorting=sortComboBox->currentIndex();
    transactionsTable->setSortingEnabled(true);
    if(sorting==0) {
        transactionsTable->sortByColumn(3,Qt::AscendingOrder);
    }
    if(sorting==1) {
        transactionsTable->sortByColumn(3,Qt::DescendingOrder);
    }
    if(sorting==2) {
        transactionsTable->sortByColumn(2,Qt::DescendingOrder);
    }
    if(sorting==3) {
        transactionsTable->sortByColumn(2,Qt::AscendingOrder);
    }
}

void MainWindow::updateBalance()
{
    QSqlQuery query("SELECT SUM(CASE WHEN type='income' THEN amount ELSE -amount END) FROM transactions");
    double balance = 0.0;

    if (query.next()) {
        balance = query.value(0).toDouble();
    }

    balanceEdit->setText(QString("Баланс: %1 $").arg(balance, 0, 'f', 2));

    QPalette palette = balanceEdit->palette();
    palette.setColor(QPalette::Text, balance < 0 ? Qt::red : Qt::black);
    balanceEdit->setPalette(palette);
}



void MainWindow::statsTable() {
    QMessageBox statsTable;
    statsTable.setText("Тратьте меньше денег");
    statsTable.setIcon(QMessageBox::NoIcon);
    statsTable.setStandardButtons(QMessageBox::Close);
    statsTable.setDefaultButton(QMessageBox::Close);
    statsTable.exec();
}