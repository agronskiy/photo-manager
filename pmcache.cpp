#include "pmcache.h"

PMCache::PMCache(const QString &dirPath, int maxLim, QObject *parent)
    : QObject(parent) {
  err = NoErr;
  limit = maxLim;
  QString path = dirPath;
  if (QDir::isRelativePath(path)) {
    path = qApp->applicationDirPath() + QDir::separator() + path;
  }
  validDir = cacheDir.cd(QDir::cleanPath(QDir::toNativeSeparators(path)));
  if (hasValidDir()) {
    cacheDir.setSorting(QDir::Time | QDir::Reversed);
    cacheDir.setFilter(QDir::Files | QDir::Readable);

    QStringList fileList = cacheDir.entryList();
    QRegExp rx("^[0-9]+\\.(pm)$");
    foreach (QString name, fileList) {
      if (rx.exactMatch(name)) {
        imageIds.insert(name.split(".").first().toUInt());
        idsStack.push(name.split(".").first().toUInt());
      }
    }
  }
}

void PMCache::addImage(quint64 id, const QPixmap &pix) {
  if (imageIds.contains(id)) {
    return;
  }

  if (imageIds.count() >= limit) {
    clear(10);
  }

  imageIds.insert(id);
  idsStack.push(id);

  pix.save(cacheDir.absolutePath() + QDir::separator() + QString::number(id) +
               ".pm",
           "JPG");
}

void PMCache::clear(int number) {
  if (number == -1) {
    number = limit;
  }
  if (number < 0) {
    return;
  }

  while (number > 0 && idsStack.count() > 0) {
    quint64 id = idsStack.pop();
    imageIds.remove(id);
    bool ok = cacheDir.remove(QString::number(id) + ".pm");
    if (!ok) {
      err = RemoveErr;
    }
    number--;
  }
}

void PMCache::setLimit(int maxLimit) { limit = maxLimit; }

void PMCache::setCacheDir(const QString &dirPath) {
  QString path = dirPath;
  if (QDir::isRelativePath(path)) {
    path = qApp->applicationDirPath() + QDir::separator() + path;
  }

  validDir = cacheDir.cd(QDir::cleanPath(QDir::toNativeSeparators(path)));

  imageIds.clear();
  idsStack.clear();

  if (hasValidDir()) {
    cacheDir.setSorting(QDir::Time | QDir::Reversed);
    cacheDir.setFilter(QDir::Files | QDir::Readable);

    QStringList fileList = cacheDir.entryList();
    QRegExp rx("^[0-9]+\\.(pm)$");
    foreach (QString name, fileList) {
      if (rx.exactMatch(name)) {
        imageIds.insert(name.split(".").first().toUInt());
        idsStack.push(name.split(".").first().toUInt());
      }
    }
  }
}

QPixmap PMCache::loadImage(quint64 id) const {
  if (!contains(id)) {
    return QPixmap();
  }

  QPixmap pix;
  QString pixPath = QDir::toNativeSeparators(cacheDir.absolutePath() + "/" +
                                             QString::number(id) + ".pm");
  pix.load(pixPath, "JPG");
  return pix;
}
