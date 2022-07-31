#include "invmodel.h"
#include "mainwindow.h"

#include <QtGui>
#include <QtSql>

// the order of types must match the one in enum PMInv::InvType
QStringList PMInv::typeList;
QStringList PMInv::dbTypeList;
QStringList PMInv::colList;
QStringList PMInv::dbColList;

PMInv PMInv::sqlRecordToInv(const QSqlRecord &record) {
  PMInv inv;
  inv.id = record.field("id").value().toUInt();
  inv.header = record.field("header").value().toString();
  inv.name = record.field("name").value().toString();
  inv.author = record.field("author").value().toString();
  if (record.field("internal_usage").value().toString() == QString('y')) {
    inv.internal = true;
  } else {
    inv.internal = false;
  }

  inv.coloured = dbColList.indexOf(record.field("coloured").value().toString());
  if (inv.coloured < 0) {
    inv.coloured = 0;
  }

  int n = PMInv::dbTypeList.indexOf(record.field("type").value().toString());
  if (n >= 0) {
    inv.type = (PMInv::InvType)n;
  } else {
    inv.type = (PMInv::InvType)0;
  }
  inv.date = record.field("date").value().toDate();
  inv.comments = record.field("comments").value().toString();

  return inv;
}

QSqlRecord PMInv::invToSqlRecord(const PMInv &inv) {
  QSqlRecord record = mWin->modelInv()->record();
  record.setValue(QString("id"), inv.id);
  record.setValue(QString("header"), inv.header);
  record.setValue(QString("name"), inv.name);
  record.setValue(QString("author"), inv.author);
  record.setValue(QString("internal_usage"),
                  (inv.internal == true ? QString('y') : QString('n')));
  record.setValue(QString("coloured"), (PMInv::dbColList.at(inv.coloured)));
  record.setValue(QString("comments"), inv.comments);
  record.setValue(QString("date"), inv.date);
  record.setValue(QString("type"), (PMInv::dbTypeList.at(inv.type)));

  return record;
}

QString PMInv::typeToStr(int type) {
  if (type < 0 || type >= PMInv::dbTypeList.count()) {
    return PMInv::dbTypeList.at(0);
  }
  return PMInv::dbTypeList.at(type);
}

int PMInv::strToType(const QString &s) {
  int n = PMInv::dbTypeList.indexOf(s);
  if (n < 0) {
    return 0;
  } else {
    return n;
  }
}

QString PMInv::colourToStr(int type) {
  if (type < 0 || type >= PMInv::dbColList.count()) {
    return PMInv::dbColList.at(0);
  }
  return PMInv::dbColList.at(type);
}

int PMInv::strToColour(const QString &s) {
  int n = PMInv::dbColList.indexOf(s);
  if (n < 0) {
    return 0;
  } else {
    return n;
  }
}

QVariant PMQSqlTableModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole && index.column() == 6) {
    return QVariant(PMInv::typeList.at(
        PMInv::strToType(QSqlQueryModel::data(index, role).toString())));
  }
  //  else if(role == Qt::DisplayRole && index.column()==3){
  //    QString str = QSqlQueryModel::data(index, role).toString();
  //    if(str.length()>27){
  //      return QVariant(str.left(25) + "..");
  //    }
  //    else {
  //      return QVariant(str);
  //    }
  //  }
  return QSqlQueryModel::data(index, role);
}
