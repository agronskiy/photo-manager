#ifndef PMCACHE_H_INCLUDED
#define PMCACHE_H_INCLUDED
#include "pm.h"
#include <QtGui>

class PMCache : public QObject {
  Q_OBJECT
public:
  PMCache(const QString &path, int maxLim = 200, QObject *parent = 0);

  void addImage(quint64 id, const QPixmap &pix);
  void setLimit(int maxLimit);
  void setCacheDir(const QString &path);

  QPixmap loadImage(quint64 id) const;

  bool contains(quint64 id) const { return imageIds.contains(id); }
  bool hasValidDir() const { return validDir; }

  int cacheSize() const { return imageIds.count(); }

  enum Error { NoErr = 0, RemoveErr, AddErr };

public slots:
  void clear(int number = -1);

protected:
  int limit;

  bool validDir;

  Error err;

  QDir cacheDir;

  QSet<quint64> imageIds;
  QStack<quint64> idsStack;
};

#endif // PMCACHE_H_INCLUDED
