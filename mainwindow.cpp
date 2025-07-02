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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_db(new DataBase(this))
{
    setupUI();
    setupConnections();

    if (m_db->isInitialized()) {
        loadTransactions();
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
    transactionsTable->setColumnCount(5);
    transactionsTable->setHorizontalHeaderLabels({"", "Name", "Type", "Price", "Date"});
    transactionsTable->setColumnWidth(0, 30);
    transactionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    transactionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    transactionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    transactionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    transactionsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    // Панель кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add", this);
    deleteButton = new QPushButton("Delete", this);
    selectAllButton = new QPushButton("Select All", this);
    sortComboBox = new QComboBox(this);
    statsButton = new QPushButton("Statistic", this);

    sortComboBox->addItem("Newest first", 0);
    sortComboBox->addItem("Oldest first", 1);
    sortComboBox->addItem("Price high to low", 2);
    sortComboBox->addItem("Price low to high", 3);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(sortComboBox);
    buttonLayout->addWidget(statsButton);

    // Поле баланса
    balanceEdit = new QLineEdit(this);
    balanceEdit->setReadOnly(true);
    balanceEdit->setAlignment(Qt::AlignRight);
    balanceEdit->setStyleSheet("font: 14pt; background: #f8f8f8; border: 1px solid #ccc;");
    balanceEdit->setText("Balance: 0.00 $");

    // Компоновка
    mainLayout->addWidget(transactionsTable);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(balanceEdit);

    setCentralWidget(centralWidget);
    resize(800, 600);

    // Стилизация
    transactionsTable->setStyleSheet(
        "QTableWidget {"
        "   selection-background-color: #E6F3FA;"
        "   selection-color: black;"
        "}"
        "QTableWidget::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
        "QTableWidget::indicator:checked {"
        "   background-color: #4CAF50;"
        "}");
}

void MainWindow::setupConnections()
{
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTransaction);
    connect(selectAllButton, &QPushButton::clicked, this, &MainWindow::selectAllTransactions);
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::sortTable);
    connect(statsButton, &QPushButton::clicked, this, &MainWindow::statsTable);
}

void MainWindow::loadTransactions()
{
    transactionsTable->setRowCount(0);
    QSqlQuery query("SELECT id, date, amount, category, type FROM transactions ORDER BY date DESC");

    while (query.next()) {
        int row = transactionsTable->rowCount();
        transactionsTable->insertRow(row);

        // Checkbox column
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        transactionsTable->setItem(row, 0, checkItem);
        checkItem->setData(Qt::UserRole, query.value("id"));

        // Name column
        transactionsTable->setItem(row, 1, new QTableWidgetItem(query.value("category").toString()));

        // Type column
        QString type = query.value("type").toString() == "income" ? "Income" : "Expense";
        transactionsTable->setItem(row, 2, new QTableWidgetItem(type));

        // Price column
        transactionsTable->setItem(row, 3, new QTableWidgetItem(QString::number(query.value("amount").toDouble(), 'f', 2)));

        // Date column
        QDate date = QDate::fromString(query.value("date").toString(), "yyyy-MM-dd");
        transactionsTable->setItem(row, 4, new QTableWidgetItem(date.toString("dd.MM.yyyy")));
    }
}

void MainWindow::addTransaction()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);

    QLineEdit nameEdit;
    QComboBox typeCombo;
    typeCombo.addItems({"Income", "Expense"});

    QDoubleSpinBox amountEdit;
    amountEdit.setRange(-1e12, 1e12);
    amountEdit.setDecimals(2);
    amountEdit.setPrefix("$ ");

    QDateEdit dateEdit(QDate::currentDate());

    form.addRow("Name:", &nameEdit);
    form.addRow("Type:", &typeCombo);
    form.addRow("Amount:", &amountEdit);
    form.addRow("Date:", &dateEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.prepare("INSERT INTO transactions (date, amount, category, type) "
                      "VALUES (:date, :amount, :category, :type)");
        query.bindValue(":date", dateEdit.date().toString("yyyy-MM-dd"));
        query.bindValue(":amount", amountEdit.value());
        query.bindValue(":category", nameEdit.text());
        query.bindValue(":type", typeCombo.currentText().toLower());

        if (query.exec()) {
            loadTransactions();
            updateBalance();
        } else {
            QMessageBox::warning(this, "Error", "Failed to add transaction");
        }
    }
}

void MainWindow::deleteTransaction()
{
    QVector<int> idsToDelete;

    // Collect checked transaction IDs
    for (int row = 0; row < transactionsTable->rowCount(); ++row) {
        QTableWidgetItem *item = transactionsTable->item(row, 0);
        if (item && item->checkState() == Qt::Checked) {
            idsToDelete.append(item->data(Qt::UserRole).toInt());
        }
    }

    if (idsToDelete.isEmpty()) {
        QMessageBox::warning(this, "Error", "No transactions selected");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm",
        QString("Delete %1 selected transactions?").arg(idsToDelete.size()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) return;

    // Delete in transaction
    QSqlDatabase::database().transaction();
    bool success = true;

    for (int id : idsToDelete) {
        QSqlQuery query;
        query.prepare("DELETE FROM transactions WHERE id = ?");
        query.addBindValue(id);

        if (!query.exec()) {
            success = false;
            qCritical() << "Delete error:" << query.lastError().text();
            break;
        }
    }

    if (success) {
        QSqlDatabase::database().commit();
        loadTransactions();
        updateBalance();
        QMessageBox::information(this, "Success", "Transactions deleted");
    } else {
        QSqlDatabase::database().rollback();
        QMessageBox::critical(this, "Error", "Failed to delete transactions");
    }
}

void MainWindow::selectAllTransactions()
{
    bool allChecked = true;

    // Check if all are already checked
    for (int row = 0; row < transactionsTable->rowCount(); ++row) {
        if (QTableWidgetItem *item = transactionsTable->item(row, 0)) {
            if (item->checkState() != Qt::Checked) {
                allChecked = false;
                break;
            }
        }
    }

    // Toggle check state
    Qt::CheckState newState = allChecked ? Qt::Unchecked : Qt::Checked;
    for (int row = 0; row < transactionsTable->rowCount(); ++row) {
        if (QTableWidgetItem *item = transactionsTable->item(row, 0)) {
            item->setCheckState(newState);
        }
    }
}

void MainWindow::sortTable(int index)
{
    QString orderBy;
    Qt::SortOrder order;

    switch (index) {
    case 0: orderBy = "date"; order = Qt::DescendingOrder; break; // Newest first
    case 1: orderBy = "date"; order = Qt::AscendingOrder; break;  // Oldest first
    case 2: orderBy = "amount"; order = Qt::DescendingOrder; break; // High to low
    case 3: orderBy = "amount"; order = Qt::AscendingOrder; break;  // Low to high
    default: return;
    }

    QSqlQuery query;
    query.prepare(QString("SELECT id, date, amount, category, type FROM transactions ORDER BY %1 %2")
                      .arg(orderBy)
                      .arg(order == Qt::DescendingOrder ? "DESC" : "ASC"));

    if (query.exec()) {
        transactionsTable->setRowCount(0);
        while (query.next()) {
            int row = transactionsTable->rowCount();
            transactionsTable->insertRow(row);

            // Checkbox column
            QTableWidgetItem *checkItem = new QTableWidgetItem();
            checkItem->setCheckState(Qt::Unchecked);
            checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            transactionsTable->setItem(row, 0, checkItem);
            checkItem->setData(Qt::UserRole, query.value("id"));

            // Other columns
            transactionsTable->setItem(row, 1, new QTableWidgetItem(query.value("category").toString()));

            QString type = query.value("type").toString() == "income" ? "Income" : "Expense";
            transactionsTable->setItem(row, 2, new QTableWidgetItem(type));

            transactionsTable->setItem(row, 3, new QTableWidgetItem(QString::number(query.value("amount").toDouble(), 'f', 2)));

            QDate date = QDate::fromString(query.value("date").toString(), "yyyy-MM-dd");
            transactionsTable->setItem(row, 4, new QTableWidgetItem(date.toString("dd.MM.yyyy")));
        }
    }
}

void MainWindow::updateBalance()
{
    QSqlQuery query("SELECT SUM(CASE WHEN type='income' THEN amount ELSE -amount END) FROM transactions");
    double balance = query.next() ? query.value(0).toDouble() : 0.0;

    balanceEdit->setText(QString("Balance: %1 $").arg(balance, 0, 'f', 2));

    QPalette palette = balanceEdit->palette();
    palette.setColor(QPalette::Text, balance < 0 ? Qt::red : Qt::black);
    balanceEdit->setPalette(palette);
}

void MainWindow::statsTable()
{
    QSqlQuery incomeQuery("SELECT SUM(amount) FROM transactions WHERE type='income'");
    QSqlQuery expenseQuery("SELECT SUM(amount) FROM transactions WHERE type='expense'");

    double income = incomeQuery.next() ? incomeQuery.value(0).toDouble() : 0.0;
    double expense = expenseQuery.next() ? expenseQuery.value(0).toDouble() : 0.0;
    double balance = income - expense;

    QString stats = QString(
                        "Financial Statistics:\n\n"
                        "Total Income: %1 $\n"
                        "Total Expense: %2 $\n"
                        "Current Balance: %3 $\n\n"
                        "Recommendation: %4"
                        ).arg(income, 0, 'f', 2)
                        .arg(expense, 0, 'f', 2)
                        .arg(balance, 0, 'f', 2)
                        .arg(balance < 0 ? "Reduce your expenses!" : "Good financial health!");

    QMessageBox::information(this, "Statistics", stats);
}
