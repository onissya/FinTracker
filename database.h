#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class DataBase : public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QObject *parent = nullptr);
    bool isInitialized() const;

private:
    QSqlDatabase m_db;
    bool initializeDatabase();
};

#endif // DATABASE_H
