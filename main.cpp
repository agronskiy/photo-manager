#include <QApplication>
#include <QtGui>

#include "invmodel.h"
#include "mainwindow.h"
#include "pm.h"
#include "pmcache.h"

int pm::ver = 1;
int pm::branch = 2;
int pm::revision = 4;

QString pm::dataDir = "none";
QString pm::host = "none";
QString pm::dbUser = "none";
QString pm::dbPass = "none";
int pm::port = 5432;
int pm::previewSize = 350;
QString pm::cacheDir = "./cache";
int pm::cacheLim = 100;
QString pm::progName = " ";
int pm::itemWidth = 300;
int pm::itemHeight = 350;
int pm::columnNum = 3;
QString pm::version =
    QString("%1.%2.%3 ").arg(pm::ver).arg(pm::branch).arg(pm::revision);
bool pm::client = false;

MainWindow *mWin;

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QTranslator translator;
  translator.load("pm_" + QLocale::system().name(),
                  QDir::toNativeSeparators(QApplication::applicationDirPath() +
                                           "/" + "lang"));
  app.installTranslator(&translator);

  pm::progName = QObject::tr("Photo Manager %1").arg(pm::version);

  PMInv::typeList << QObject::tr("None", 0) << QObject::tr("Report", 0)
                  << QObject::tr("Iconography", 0) << QObject::tr("Artistic", 0)
                  << QObject::tr("Portrait", 0)
                  << QObject::tr("Documentalistic");
  PMInv::dbTypeList << "n"
                    << "r"
                    << "i"
                    << "a"
                    << "p"
                    << "d";

  PMInv::colList << QObject::tr("Undefined", 0) << QObject::tr("Coloured", 0)
                 << QObject::tr("Monochrome", 0);
  PMInv::dbColList << "u"
                   << "y"
                   << "n";

  QPixmapCache::setCacheLimit(15360);

  MainWindow window;
  mWin = &window;
  if (argc > 1) {
    if (QString(argv[1]) == "add" && argc > 2) {
      mWin->addBigimagesToDbFromLocation(QString(argv[2]));
    } else if (QString(argv[1]) == "add" && argc == 2) {
      mWin->addBigimagesToDbFromLocation(QString(""));
    }
  }
  mWin->show();
  mWin->modelCategory()->readDataFromDB();
  mWin->collapseTree();

  return app.exec();
}
