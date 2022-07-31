#include "threads.h"
#include "mainwindow.h"

PMAddImagesFromDirThread::PMAddImagesFromDirThread(MainWindow *mainWindow,
                                                   QString dirPath,
                                                   QObject *parent)
    : QThread(parent) {

  this->dirPath = dirPath;
  this->mw = mainWindow;
}

void PMAddImagesFromDirThread::run() { return; }

PMSearchThread::PMSearchThread(MainWindow *mainWindow, QString str,
                               QObject *parent)
    : QThread(parent) {
  this->str = str;
  this->mw = mainWindow;
}

void PMSearchThread::run() {
  QList<quint64> idList;
  QSqlQuery query;
  query.exec("SELECT id FROM pm_images WHERE lower(description) @@ "
             "to_tsquery(lower('" +
             str + "'))");

  int dots = 0;

  while (query.next()) {
    dots++;
    idList.append(query.value(0).toUInt());
    emit searchMsg((dots / 3) % 10);
  }

  query.clear();
  query.exec(
      "SELECT id FROM pm_images WHERE lower(name) @@ to_tsquery(lower('" + str +
      "'))");
  while (query.next()) {
    if (!idList.contains(query.value(0).toUInt())) {
      dots++;
      idList.append(query.value(0).toUInt());
      emit searchMsg((dots / 3) % 10);
    }
  }
  query.clear();
  query.exec(
      "SELECT id FROM pm_images WHERE lower(header) @@ to_tsquery(lower('" +
      str + "'))");
  while (query.next()) {
    if (!idList.contains(query.value(0).toUInt())) {
      dots++;
      idList.append(query.value(0).toUInt());
      emit searchMsg((dots / 3) % 10);
    }
  }

  if (idList.isEmpty()) {
    QStringList list = str.split("&");
    str = "";
    foreach (QString s, list) {
      if (!str.isEmpty()) {
        str += " AND ";
      }
      str += " lower(description) ~ lower('" + s + "') ";
    }
    query.clear();
    query.exec("SELECT id FROM pm_images WHERE " + str);
    while (query.next()) {
      if (!idList.contains(query.value(0).toUInt())) {
        dots++;
        idList.append(query.value(0).toUInt());
        emit searchMsg((dots / 3) % 10);
      }
    }

    str = "";
    foreach (QString s, list) {
      if (!str.isEmpty()) {
        str += " AND ";
      }
      str += " lower(name) ~ lower('" + s + "') ";
    }
    query.clear();
    query.exec("SELECT id FROM pm_images WHERE " + str);
    while (query.next()) {
      if (!idList.contains(query.value(0).toUInt())) {
        dots++;
        idList.append(query.value(0).toUInt());
        emit searchMsg((dots / 3) % 10);
      }
    }

    str = "";
    foreach (QString s, list) {
      if (!str.isEmpty()) {
        str += " AND ";
      }
      str += " lower(header) ~ lower('" + s + "') ";
    }
    query.clear();
    query.exec("SELECT id FROM pm_images WHERE " + str);
    while (query.next()) {
      if (!idList.contains(query.value(0).toUInt())) {
        dots++;
        idList.append(query.value(0).toUInt());
        emit searchMsg((dots / 3) % 10);
      }
    }
  }
  mw->modelImage()->setSearchResults(idList);
  return;
}

//////////////////////////////////////////////////////////////////
// PMLoadImages::PMLoadImages(QObject* parent) : QThread(parent){
//
// }

void PMLoadImages::run() {
  QByteArray b;
  PMTransaction transact;
  QSqlQuery q(db);
  forever {
    mutex.lock();
    if (trans.isEmpty()) {
      mutex.unlock();
      break;
    }

    transact = trans.dequeue();
    mutex.unlock();

    q.clear();
    q.exec("SELECT preview FROM pm_images WHERE id=" +
           QString::number(transact.id));
    q.next();
    b = q.value(0).toByteArray();
    emit finishedImage(transact.row, b);
  }
}

void PMLoadImages::addTransaction(PMTransaction t) {
  QMutexLocker locker(&mutex);
  trans.enqueue(t);
  if (!isRunning())
    start();
}

void PMLoadImages::clearTransactions() {
  QMutexLocker locker(&mutex);
  trans.clear();
}
PMLoadImages::~PMLoadImages() { db = QSqlDatabase(); }
