#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtXml>

#include "categorymodel.h"
#include "catwidget.h"
#include "dbimagemodel.h"
#include "pmcache.h"

class PMSearchThread;
class PMView;
class PMInvWidget;
class PMMetaInfoWidget;
class PMFlagTreeWidget;
class PMQSqlTableModel;
class PMCatWidget;
class PMInv;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();

  PMCatModel *modelCategory() const { return catModel; }
  PMDBImageModel *modelImage() const { return imageModel; }
  PMQSqlTableModel *modelInv() const { return invModel; }
  PMCache *imageCache() const { return cache; }

  QItemSelectionModel *catSelectionModel() const { return catSelection; }
  QItemSelectionModel *imageSelectionModel() const { return imageSelection; }
  QItemSelectionModel *invSelectionModel() const { return invSelection; }
  // image management in DB API
  static void addImageToDB(PMDBImage &image, QPixmap &bigPixmap);
  static void saveInventaryToDisk(const QModelIndex &index,
                                  const QString &dirName);
  static void saveImageToDisk(quint64 id, const QString &dirPath,
                              const QString &ext = "jpg",
                              const QString &type = "JPEG");
  static void readImageFile(PMDBImage &newImage, QPixmap &destPix,
                            const QString &fromDir, const QString &fileName,
                            const QString &description, const QString &header,
                            const QString &cd, quint64 inv, const QDate &date,
                            bool &ok);

  static PMDBImage *loadImageFromDB(quint64 id, pm::LoadFlags = pm::NoPreview);

  static quint64 findImageInDB(const QString &absolutePath);

  static bool updateImageInDB(PMDBImage &image,
                              pm::UpdateFlags flag = pm::NoPreview);
  static bool removeImageFromDB(PMDBImage &image);
  static bool isClient() {
    return pm::client;
  } // switch between Editor and Client modes
  static bool isEditor() { return !isClient(); }

  static QPixmap loadPreview(const PMDBImage &image);
  static QPixmap loadPreview(const quint64 id);

  static QPixmap loadFullImage(const QString &location);
  static QPixmap loadFullImage(const quint64);

  static QString getDbVar(const QString &varName);

  QString invQuery;

  QLabel *sizeLabel;
  QLabel *dimensionsLabel;
  QLabel *dpiLabel;
  QLabel *nameLabel;

public slots:
  void createCategory();
  void removeCategory();
  void renameCategory();
  void changeCatParent();
  void addImagesFromDir();
  void setDbSettings();
  void setViewerSettings();
  void setCacheSettings();
  void connectToDb();
  void showOrphanImages();
  void showLastAddedImages();
  void searchImages();
  void searchImagesById();
  void removeSelectedImages();
  void importFromAcdseeXml();
  void addNewInventaryItem();
  void removeSelectedInventary();
  void saveSelectedInventary();
  void about();
  void quit();
  void collapseTree() { catWidget->treeView()->collapseAll(); }
  void saveSelectedImagesToLocal();
  void addBigimagesToDbFromLocation(const QString &dataDir);
  void clearCache();
  void organizeInvWidget();

protected slots:
  void endAddingImages();
  void endSearching();
  void showMessageNotLoaded(QString &str);
  void showPercentageLoaded(int i, QString path);
  void showSearching(int i);

protected:
  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void readSettings();
  void writeSettings();
  void addNewInventaryItem2(QDir &dir, PMInv &inv, int createInv, int depth,
                            int maxDepth, QString &commonDescription,
                            QString path);
  int processAcdseeXmlAsset(const QDomNode &asset, const QDir &dir,
                            quint64 number);

  QString nameLatinToUpper(const QString name);

  quint64 processAcdseeXmlAssetCat(const QString &assetCat);

  static bool createConnection();

  QMenu *fileMenu;
  QMenu *viewMenu;
  QMenu *settingsMenu;
  QMenu *sortMenu;

  QAction *quitAction;
  QAction *createNewCategoryAction;
  QAction *removeCategoryAction;
  QAction *renameCategoryAction;
  QAction *changeCatParentAction;
  QAction *addImageFromDirAction;
  QAction *dbSettingsAction;
  QAction *connectToDbAction;
  QAction *showOrphanImagesAction;
  QAction *clearCacheAction;
  QAction *refreshImageView;
  QAction *searchAction;
  QAction *searchByIdAction;
  QAction *removeSelectedImagesAction;
  QAction *aboutAction;
  QAction *sortByIdAction;
  QAction *sortByDateAction;
  QAction *sortByHeaderAction;
  QAction *viewerSettingsAction;
  QAction *cacheSettingsAction;
  QAction *refreshCatViewAction;
  QAction *showLastAddedAction;
  QAction *importFromAcdseeXmlAction;
  QAction *addNewInventaryAction;
  QAction *removeInventaryAction;
  QAction *saveSelectedImage;
  QAction *saveSelectedInventaryAction;

  QItemSelectionModel *catSelection;
  QItemSelectionModel *imageSelection;
  QItemSelectionModel *invSelection;

  PMCatModel *catModel;
  PMDBImageModel *imageModel;
  PMQSqlTableModel *invModel;

  PMView *imageView;
  PMMetaInfoWidget *metaInfoWidget;
  PMFlagTreeWidget *changeCatWidget;
  PMInvWidget *invWidget;
  PMCatWidget *catWidget;

  static PMCache *cache;

  PMSearchThread *searchThr;
};

#endif // MAINWINDOW_H_INCLUDED
