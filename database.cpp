#include "database.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DataBase::DataBase(QObject *parent)
    : QObject(parent)
{
    if (!initializeDatabase()) {
        qCritical() << "Database initialization failed";
    }
}

bool DataBase::initializeDatabase()
{
    const QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dbPath)) {
        qCritical() << "Cannot create database directory";
        return false;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath + "/finance.db");

    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    const QString createTableQuery =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "date TEXT NOT NULL, "
        "amount REAL NOT NULL, "
        "category TEXT NOT NULL, "
        "type TEXT CHECK(type IN ('income', 'expense'))"
        ")";

    if (!query.exec(createTableQuery)) {
        qCritical() << "Cannot create table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database created at:" << m_db.databaseName();
    return true;
}

bool DataBase::isInitialized() const
{
    return m_db.isOpen();
}
