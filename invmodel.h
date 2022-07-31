#ifndef INVMODEL_H_INCLUDED
#define INVMODEL_H_INCLUDED

#include <QtGui>
#include <QtSql>

class PMInv {
public:
  PMInv() {}
  PMInv(QString &name) { this->name = name; }
  static PMInv sqlRecordToInv(const QSqlRecord &record);
  static QSqlRecord invToSqlRecord(const PMInv &inv);
  static QString typeToStr(int type);
  static int strToType(const QString &s);
  static QString colourToStr(int type);
  static int strToColour(const QString &s);

  enum InvType { None = 0, Report, Icon };

  enum { Undefined = 0, Coloured, Monohrome };

  QString header;
  QString author;
  QString name;
  quint64 id;
  bool internal;
  int coloured;
  QString comments;
  QDate date;
  int type; // PMInv::InvType
  static QStringList typeList;
  static QStringList dbTypeList;
  static QStringList colList;
  static QStringList dbColList;
};

class PMQSqlTableModel : public QSqlQueryModel {
  Q_OBJECT
public:
  PMQSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase())
      : QSqlQueryModel(parent) {}
  ~PMQSqlTableModel() {}
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};
#endif // INVMODEL_H_INCLUDED
