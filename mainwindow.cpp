// implementation of MainWindow class

#include <QApplication>
#include <QHeaderView>
#include <QListView>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QTableView>
#include <QTreeView>

#include "categorymodel.h"
#include "catwidget.h"
#include "dbsettingsdialog.h"
#include "dialogs.h"
#include "flagtreeview.h"
#include "helpdialog.h"
#include "imageview.h"
#include "invmodel.h"
#include "invwidget.h"
#include "mainwindow.h"
#include "metainfowidget.h"
#include "threads.h"

PMCache *MainWindow::cache;

MainWindow::MainWindow() {
  setWindowState(Qt::WindowMaximized);
  readSettings();
  createConnection();
  cache = new PMCache(pm::cacheDir, pm::cacheLim);

  QSplitter *splitter = new QSplitter(Qt::Horizontal);
  QTabWidget *leftTabWidget = new QTabWidget;
  leftTabWidget->setTabPosition(QTabWidget::West);
  leftTabWidget->setTabShape(QTabWidget::Triangular);
  splitter->addWidget(leftTabWidget);

  invModel = new PMQSqlTableModel(this, QSqlDatabase::database());

  invQuery = "SELECT id, name, author, header, coloured, internal_usage, type, "
             "comments, date FROM pm_inv ORDER BY name, type";
  //  invQuery = "SELECT id, name, author, header, coloured, internal_usage,
  //  type, comments, date FROM pm_inv ORDER BY cast(substring(name from
  //  '[0-9]+') as int), name, type";

  invModel->setQuery(QSqlQuery(invQuery));
  invModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
  invModel->setHeaderData(6, Qt::Horizontal, tr("Type"));
  invModel->setHeaderData(3, Qt::Horizontal, tr("Header"));

  invSelection = new QItemSelectionModel(invModel);

  catModel = new PMCatModel;

  imageModel = new PMDBImageModel;
  imageModel->setCatModel(catModel);

  catSelection = new QItemSelectionModel(catModel);
  catWidget = new PMCatWidget(this, this);
  catWidget->treeView()->setModel(catModel);
  catWidget->treeView()->setSelectionModel(catSelection);
  catWidget->treeView()->setUniformRowHeights(true);
  catWidget->treeView()->setColumnWidth(0, 200);
  catWidget->treeView()->setAlternatingRowColors(true);
  catWidget->treeView()->header()->setStretchLastSection(true);
  catWidget->treeView()->header()->setVisible(false);

  leftTabWidget->addTab(catWidget, tr("Categories"));

  metaInfoWidget = new PMMetaInfoWidget(this);
  leftTabWidget->addTab(metaInfoWidget, tr("Meta"));

  changeCatWidget = new PMFlagTreeWidget(catModel, imageModel, this);
  leftTabWidget->addTab(changeCatWidget, tr("Set category"));

  invWidget = new PMInvWidget(this);
  invWidget->treeView()->setModel(invModel);
  invWidget->treeView()->setSelectionModel(invSelection);
  invWidget->treeView()->setSelectionMode(QAbstractItemView::SingleSelection);
  invWidget->treeView()->setSelectionBehavior(QAbstractItemView::SelectRows);
  organizeInvWidget();
  invWidget->treeView()->setEditTriggers(QAbstractItemView::NoEditTriggers);
  invWidget->treeView()->setRootIsDecorated(false);
  leftTabWidget->addTab(invWidget, tr("Folders"));

  imageSelection = new QItemSelectionModel(imageModel);

  imageView = new PMView(this);
  imageView->setModel(imageModel);
  imageView->setSelectionModel(imageSelection);
  imageView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  imageView->setSelectionBehavior(QAbstractItemView::SelectItems);
  imageView->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  splitter->addWidget(imageView);
  splitter->setStretchFactor(1, 1);

  connect(catWidget->treeView(), SIGNAL(clicked(QModelIndex)), imageModel,
          SLOT(loadCategoryContents(QModelIndex)));
  connect(catWidget->treeView(), SIGNAL(clicked(QModelIndex)), imageSelection,
          SLOT(clearSelection()));
  connect(catWidget->treeView()->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)), catWidget,
          SLOT(recalculateButtons()));
  connect(imageSelection,
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          metaInfoWidget, SLOT(imageSelectionChanged()));
  connect(imageSelection,
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          changeCatWidget, SLOT(imageSelectionChanged()));
  connect(
      imageModel,
      SIGNAL(selectImage(QItemSelection, QItemSelectionModel::SelectionFlags)),
      imageSelection,
      SLOT(select(QItemSelection, QItemSelectionModel::SelectionFlags)));
  connect(imageModel, SIGNAL(modelReset()), imageSelection, SLOT(clear()));
  //  connect(catModel, SIGNAL(modelReset()), tree, SLOT(expandAll()));
  connect(invWidget->treeView(), SIGNAL(clicked(QModelIndex)), imageModel,
          SLOT(loadInventaryContents(QModelIndex)));
  connect(invSelection,
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)), invWidget,
          SLOT(recalculateButton()));

  connect(catModel, SIGNAL(modelReset()), catSelection, SLOT(clear()));
  connect(imageModel, SIGNAL(modelReset()), imageSelection, SLOT(clear()));

  setCentralWidget(splitter);
  setWindowState(windowState() | Qt::WindowMaximized);
  if (isClient()) {
    pm::progName += " " + tr("(Client edition)");
  } else {
    pm::progName += " " + tr("Editor edition");
  }
  setWindowTitle(pm::progName);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();

  if (!cache->hasValidDir()) {
    QMessageBox::warning(this, tr("Not valid cache directory"),
                         tr("Please set a valid cache directory"));
    setCacheSettings();
  }
}

MainWindow::~MainWindow() {
  writeSettings();
  imageView->loadThr()->terminate();
  imageView->loadThr()->db = QSqlDatabase();
  QSqlDatabase::database().close();
  QSqlDatabase::database("second").close();
  // QSqlDatabase::database("second").close();
}

void MainWindow::quit() {
  writeSettings();

  //  imageView->loadThr()->terminate();
  //  imageView->loadThr()->db = QSqlDatabase();
  //  QSqlDatabase::database().close();
  //  QSqlDatabase::database("second").close();
  this->close();
}

void MainWindow::about() {
  QMessageBox::about(
      this, pm::progName,
      tr("<h2>%1</h2>"
         "<p>&copy; Alex. Y Gronsky, Moscow, 2008</p>"
         "<p>Photo management system with network database support. </p>"
         "<p>gronsky_a@mail.ru</p>")
          .arg(pm::progName));
}

void MainWindow::createActions() {
  aboutAction = new QAction(tr("About"), this);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  createNewCategoryAction = new QAction(tr("New Category"), this);
  createNewCategoryAction->setShortcut(tr("Ctrl+N"));
  connect(createNewCategoryAction, SIGNAL(triggered()), this,
          SLOT(createCategory()));

  removeCategoryAction = new QAction(tr("Remove Categories"), this);
  removeCategoryAction->setShortcut(tr("Ctrl+Shift+D"));
  connect(removeCategoryAction, SIGNAL(triggered()), this,
          SLOT(removeCategory()));

  renameCategoryAction = new QAction(tr("Rename Category"), this);
  connect(renameCategoryAction, SIGNAL(triggered()), this,
          SLOT(renameCategory()));

  changeCatParentAction = new QAction(tr("Change Category Parent"), this);
  connect(changeCatParentAction, SIGNAL(triggered()), this,
          SLOT(changeCatParent()));

  addImageFromDirAction =
      new QAction(tr("Add images to an existing folder"), this);
  addImageFromDirAction->setShortcut(tr("Ctrl+O"));
  connect(addImageFromDirAction, SIGNAL(triggered()), this,
          SLOT(addImagesFromDir()));

  dbSettingsAction = new QAction(tr("DB and Data Settings"), this);
  connect(dbSettingsAction, SIGNAL(triggered()), this, SLOT(setDbSettings()));

  connectToDbAction = new QAction(tr("Connect to DB"), this);
  connect(connectToDbAction, SIGNAL(triggered()), this, SLOT(connectToDb()));

  showOrphanImagesAction = new QAction(tr("Show images with no parent"), this);
  connect(showOrphanImagesAction, SIGNAL(triggered()), this,
          SLOT(showOrphanImages()));

  clearCacheAction = new QAction(tr("Clear cache"), this);
  connect(clearCacheAction, SIGNAL(triggered()), this, SLOT(clearCache()));

  refreshImageView = new QAction(tr("Refresh images"), this);
  refreshImageView->setShortcut(tr("Ctrl+R"));
  connect(refreshImageView, SIGNAL(triggered()), modelImage(),
          SLOT(reloadContents()));
  connect(refreshImageView, SIGNAL(triggered()), imageSelectionModel(),
          SLOT(clearSelection()));

  searchAction = new QAction(tr("Find images"), this);
  searchAction->setShortcut(tr("Ctrl+F"));
  connect(searchAction, SIGNAL(triggered()), this, SLOT(searchImages()));

  searchByIdAction = new QAction(tr("Find images by ID"), this);
  searchByIdAction->setShortcut(tr("Ctrl+Shift+F"));
  connect(searchByIdAction, SIGNAL(triggered()), this,
          SLOT(searchImagesById()));

  removeSelectedImagesAction =
      new QAction(tr("Remove selected images from DB"), this);
  removeSelectedImagesAction->setShortcut(tr("Ctrl+D"));
  connect(removeSelectedImagesAction, SIGNAL(triggered()), this,
          SLOT(removeSelectedImages()));

  viewerSettingsAction = new QAction(tr("Viewer settings"), this);
  connect(viewerSettingsAction, SIGNAL(triggered()), this,
          SLOT(setViewerSettings()));

  cacheSettingsAction = new QAction(tr("Cache settings"), this);
  connect(cacheSettingsAction, SIGNAL(triggered()), this,
          SLOT(setCacheSettings()));

  refreshCatViewAction = new QAction(tr("Refresh categories"), this);
  refreshCatViewAction->setShortcut(tr("Ctrl+Shift+R"));
  connect(refreshCatViewAction, SIGNAL(triggered()), catModel,
          SLOT(readDataFromDB()));

  quitAction = new QAction(tr("Quit"), this);
  quitAction->setShortcut(tr("Ctrl+Q"));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

  showLastAddedAction = new QAction(tr("Show last added images"), this);
  connect(showLastAddedAction, SIGNAL(triggered()), this,
          SLOT(showLastAddedImages()));

  importFromAcdseeXmlAction = new QAction(tr("Import from ACDSee XML"), this);
  connect(importFromAcdseeXmlAction, SIGNAL(triggered()), this,
          SLOT(importFromAcdseeXml()));

  addNewInventaryAction = new QAction(tr("Load a new folder"), this);
  connect(addNewInventaryAction, SIGNAL(triggered()), this,
          SLOT(addNewInventaryItem()));

  removeInventaryAction = new QAction(tr("Remove selected folder"), this);
  connect(removeInventaryAction, SIGNAL(triggered()), this,
          SLOT(removeSelectedInventary()));

  saveSelectedImage =
      new QAction(tr("Save selected image to local disk"), this);
  connect(saveSelectedImage, SIGNAL(triggered()), this,
          SLOT(saveSelectedImagesToLocal()));

  saveSelectedInventaryAction =
      new QAction(tr("Save selected folder to local disk"), this);
  connect(saveSelectedInventaryAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedInventary()));

  if (isClient()) {
    createNewCategoryAction->setEnabled(false);
    removeCategoryAction->setEnabled(false);
    renameCategoryAction->setEnabled(false);
    addImageFromDirAction->setEnabled(false);
    removeSelectedImagesAction->setEnabled(false);
    importFromAcdseeXmlAction->setEnabled(false);
    addNewInventaryAction->setEnabled(false);
    removeInventaryAction->setEnabled(false);
    changeCatParentAction->setEnabled(false);
  }
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(saveSelectedImage);
  fileMenu->addAction(saveSelectedInventaryAction);
  fileMenu->addSeparator();
  fileMenu->addAction(removeSelectedImagesAction);
  fileMenu->addSeparator();
  fileMenu->addAction(createNewCategoryAction);
  fileMenu->addAction(removeCategoryAction);
  fileMenu->addAction(renameCategoryAction);
  fileMenu->addAction(changeCatParentAction);
  fileMenu->addSeparator();
  fileMenu->addAction(addNewInventaryAction);
  fileMenu->addAction(removeInventaryAction);
  fileMenu->addAction(addImageFromDirAction);
  fileMenu->addSeparator();
  fileMenu->addAction(importFromAcdseeXmlAction);
  fileMenu->addSeparator();
  fileMenu->addAction(quitAction);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(showOrphanImagesAction);
  viewMenu->addAction(showLastAddedAction);
  viewMenu->addSeparator();
  viewMenu->addAction(refreshImageView);
  viewMenu->addAction(refreshCatViewAction);
  viewMenu->addSeparator();
  sortMenu = viewMenu->addMenu(tr("Sort images"));
  sortMenu->addActions(modelImage()->sortGroup->actions());
  viewMenu->addSeparator();
  viewMenu->addAction(searchAction);
  viewMenu->addAction(searchByIdAction);

  settingsMenu = menuBar()->addMenu(tr("&Options"));
  settingsMenu->addAction(dbSettingsAction);
  settingsMenu->addAction(viewerSettingsAction);
  settingsMenu->addAction(cacheSettingsAction);
  settingsMenu->addSeparator();
  settingsMenu->addAction(clearCacheAction);
  settingsMenu->addSeparator();
  settingsMenu->addAction(connectToDbAction);

  menuBar()->addAction(aboutAction);
}

void MainWindow::createToolBars() {}

bool MainWindow::createConnection() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
  db.setHostName(pm::host);
  db.setPort(pm::port);
  db.setDatabaseName("pm_database");
  db.setUserName(pm::dbUser);
  db.setPassword(pm::dbPass);

  QSqlDatabase db1 = QSqlDatabase::cloneDatabase(db, "second");

  bool ok = db.open() && db1.open();

  if (!ok) {
    QMessageBox::warning(0, tr("Cannot connect to database"),
                         db.lastError().text() + db1.lastError().text());
    return false;
  } else
    return true;
}

void MainWindow::createCategory() {
  PMNewCatDialog dialog(this);
  dialog.exec();
}

void MainWindow::changeCatParent() {
  if (catSelectionModel()->selectedIndexes().isEmpty()) {
    QMessageBox::warning(0, tr("Select a category"),
                         tr("No category selected"));
    return;
  }
  PMChangeCatParDialog dialog(this);
  dialog.exec();
}

void MainWindow::removeCategory() {
  PMRemoveCatDialog dialog(this);
  dialog.exec();
}

void MainWindow::addImagesFromDir() {
  if (invSelectionModel()->selectedIndexes().isEmpty()) {
    QMessageBox::warning(this, tr("No folder selected"),
                         tr("Please select an folder"));
    return;
  }

  QModelIndex curr = invSelectionModel()->selectedIndexes().first();

  QSqlRecord rec = mWin->modelInv()->record(curr.row());
  PMInv currInv = PMInv::sqlRecordToInv(rec);
  QString dirPath = QFileDialog::getExistingDirectory(
      this, tr("Choose the directory"), QString(), QFileDialog::ShowDirsOnly);
  if (dirPath.isEmpty()) {
    return;
  }

  dirPath = QDir::fromNativeSeparators(QDir::cleanPath(dirPath));
  QDir dir(QDir::toNativeSeparators(dirPath));

  statusBar()->showMessage(tr("Loading..."));
  this->setEnabled(false);

  QStringList filters;
  filters << "*.jpg"
          << "*.jpeg";

  QStringList dirFilters;
  QStringList fileList = dir.entryList(filters, QDir::Files | QDir::Readable);
  QStringList dirList = dir.entryList(dirFilters, QDir::Dirs | QDir::Readable |
                                                      QDir::NoDotAndDotDot);
  QString location = dirPath;

  location.remove(pm::dataDir, Qt::CaseInsensitive); // location in pm::dataDir
  while (location.startsWith("/")) {
    location.remove(0, 1);
  }

  QPixmap newPixmap, bigPix;
  PMDBImage newImage;
  newImage.pid.append(0);
  for (int i = 0; i < fileList.count(); i++) {
    bool ok;
    bigPix = QPixmap(QDir::toNativeSeparators(dirPath) + QDir::separator() +
                     fileList.at(i));
    readImageFile(newImage, bigPix, dirPath, fileList.at(i), QString(""),
                  currInv.header, QString(""), currInv.id, currInv.date, ok);

    addImageToDB(newImage, bigPix);
    //    emit percentageLoaded(i*100/fileList.count());
    showPercentageLoaded(i * 100 / fileList.count(), dir.absolutePath());
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  foreach (QString s, dirList) {
    if (s == "." || s == "..") {
      continue;
    }
    dir.cd(QDir::toNativeSeparators(dir.absolutePath()) + QDir::separator() +
           s);
    fileList = dir.entryList(filters, QDir::Files | QDir::Readable);
    for (int i = 0; i < fileList.count(); i++) {
      bool ok;
      bigPix = QPixmap(QDir::toNativeSeparators(dir.canonicalPath()) +
                       QDir::separator() + fileList.at(i));
      readImageFile(newImage, bigPix, dir.canonicalPath(), fileList.at(i),
                    QString(""), currInv.header, s, currInv.id, currInv.date,
                    ok);

      addImageToDB(newImage, bigPix);
      showPercentageLoaded(i * 100 / fileList.count(), dir.absolutePath());
      qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    dir.cd("..");
  }

  this->setEnabled(true);
  statusBar()->clearMessage();
  modelImage()->loadInventaryContents(currInv.id);
}

void MainWindow::addImageToDB(PMDBImage &image, QPixmap &bigPixmap) {
  QPixmap pixmap = image.preview;
  QByteArray bytes, bytesBig;
  QBuffer buffer(&bytes);
  buffer.open(QIODevice::WriteOnly);
  pixmap.save(&buffer, "JPG");
  buffer.close();

  QBuffer bigBuffer(&bytesBig);
  bigBuffer.open(QIODevice::WriteOnly);
  bigPixmap.save(&bigBuffer, "JPG");
  bigBuffer.close();
  //  MyDialog dialog;
  //  dialog.pix->setPixmap(pixmap);
  //  dialog.exec();

  QSqlQuery query, query2;
  QString queryText =
      QString("INSERT INTO ") +
      "pm_images (name, header, description, inventary, location, cd, pid, "
      "date, preview," +
      "width, height, width_orig, height_orig, size, x_dpi, y_dpi)" + " " +
      "VALUES (:name, :header, :description, :inventary, :location,:cd, :pid, "
      ":date, E:preview," +
      ":w, :h, :w_orig, :h_orig, :size, :x_dpi, :y_dpi)";
  query.prepare(queryText);
  query.bindValue(":name", QVariant(image.name), QSql::InOut);
  if (image.header.isEmpty()) {
    query.bindValue(":header", QVariant(""));
  } else {
    query.bindValue(":header", QVariant(image.header));
  }
  if (image.description.isEmpty()) {
    query.bindValue(":description", QVariant(""));
  } else {
    query.bindValue(":description", QVariant(image.description));
  }
  query.bindValue(":description", QVariant(image.description));
  query.bindValue(":inventary", QVariant(image.inventary));
  query.bindValue(":location", QVariant(image.location));
  query.bindValue(":cd", QVariant(image.cd));
  query.bindValue(":date", QVariant(image.date));
  query.bindValue(":pid", PMDBImageModel::qVariantFromQList(image.pid));
  if (!bytes.isEmpty()) {
    query.bindValue(":preview", QVariant(bytes));
  } else {
    query.bindValue(":preview", QVariant(" "));
  }
  query.bindValue(":w", QVariant(image.width));
  query.bindValue(":h", QVariant(image.height));
  query.bindValue(":w_orig", QVariant(image.origWidth));
  query.bindValue(":h_orig", QVariant(image.origHeight));
  query.bindValue(":size", QVariant(image.origSize));
  query.bindValue(":x_dpi", QVariant(image.xDensityDpi));
  query.bindValue(":y_dpi", QVariant(image.yDensityDpi));

  QSqlDatabase::database().transaction();
  bool ok = query.exec();
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), query.lastError().text());
    QSqlDatabase::database().rollback();
  } else {
    QSqlDatabase::database().commit();
    query.clear();
    query.exec("SELECT MAX(id) FROM pm_images");
    if (query.next()) {
      image.id = query.value(0).toUInt();
      query2.prepare(
          "INSERT INTO pm_bigimages (id, image) VALUES (:id, E:preview)");
      query2.bindValue(":id", QVariant(image.id));
      if (!bytesBig.isEmpty()) {
        query2.bindValue(":preview", QVariant(bytesBig));
      } else {
        query2.bindValue(":preview", QVariant(" "));
      }
      ok = query2.exec();
      if (!ok) {
        QMessageBox::warning(
            0, tr("Database error"),
            tr("Full image not loaded for %1").arg(image.name));
      }
    } else {
      QMessageBox::warning(0, tr("Database error"), query.lastError().text());
    }
  }
}

quint64 MainWindow::findImageInDB(const QString &absolutePath) {
  QSqlQuery query;
  QString normalized =
      QDir::fromNativeSeparators(QDir::cleanPath(absolutePath));
  bool ok = query.exec("SELECT id FROM pm_images WHERE location='" +
                       normalized + "'");
  if (!ok) {
    return 0;
  } else if (query.next()) {
    return query.value(0).toUInt();
  } else
    return 0;
}

PMDBImage *MainWindow::loadImageFromDB(quint64 id, pm::LoadFlags flags) {
  QSqlQuery query;
  QString previewQuery = " ";
  if (flags & pm::WithPreview) {
    previewQuery = ", preview ";
  }

  query.exec(QString("SELECT id, name, header, description, inventary, "
                     "location, cd, pid, date, ") +
             "width, height, size, width_orig, height_orig, x_dpi, y_dpi" +
             previewQuery + QString("FROM pm_images WHERE id=") +
             QString::number(id));
  if (query.next()) {
    PMDBImage *image = new PMDBImage;
    image->id = id;
    image->name = query.value(1).toString();
    image->header = query.value(2).toString();
    image->description = query.value(3).toString();
    image->inventary = query.value(4).toUInt();
    image->location = query.value(5).toString();
    image->cd = query.value(6).toString();
    image->pid = PMDBImageModel::qListFromQVariant(query.value(7));
    image->date = query.value(8).toDate();
    if (query.value(9).isValid()) {
      image->width = query.value(9).toInt();
    } else {
      image->width = 0;
    }
    if (query.value(10).isValid()) {
      image->height = query.value(10).toInt();
    } else {
      image->height = 0;
    }
    if (query.value(11).isValid()) {
      image->origSize = query.value(11).toDouble();
    } else {
      image->origSize = 0.0;
    }
    if (query.value(12).isValid()) {
      image->origWidth = query.value(12).toInt();
    } else {
      image->origWidth = 0;
    }
    if (query.value(13).isValid()) {
      image->origHeight = query.value(13).toInt();
    } else {
      image->origHeight = 0;
    }
    image->xDensityDpi = query.value(14).toInt();
    image->yDensityDpi = query.value(15).toInt();

    if (flags & pm::WithPreview) {
      QByteArray arr = query.value(16).toByteArray();
      image->preview.loadFromData(arr, "JPG");
    }
    return image;
  } else {
    return static_cast<PMDBImage *>(0);
  }
}

bool MainWindow::updateImageInDB(PMDBImage &image, pm::UpdateFlags flag) {
  QByteArray bytes;
  if (flag & pm::WithPreview) {
    QPixmap pixmap = image.preview;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPG");
    buffer.close();
  }

  QSqlQuery query;
  if (flag & pm::NoPreview) {
    query.prepare(
        QString("UPDATE pm_images SET name=:name, header=:header, "
                "description=:description,") +
        "inventary=:inventary, location=:location, cd=:cd, date=:date, "
        "pid=:pid, " +
        "width=:w, height=:h, width_orig=:w_orig, height_orig=:h_orig, "
        "size=:size, x_dpi=:x_dpi, y_dpi=:y_dpi" +
        " WHERE id=" + QString::number(image.id));
  } else {
    query.prepare(
        QString("UPDATE pm_images SET name=:name, header=:header, "
                "description=:description,") +
        "inventary=:inventary, location=:location, cd=:cd, date=:date, "
        "pid=:pid, preview=:preview, " +
        "width=:w, height=:h, width_orig=:w_orig, height_orig=:h_orig, "
        "size=:size, x_dpi=:x_dpi, y_dpi=:y_dpi" +
        "WHERE id=" + QString::number(image.id));
    if (!bytes.isEmpty()) {
      query.bindValue(":preview", QVariant(bytes));
    } else {
      query.bindValue(":preview", QVariant(" "));
    }
  }
  query.bindValue(":name", QVariant(image.name), QSql::InOut);
  query.bindValue(":header", QVariant(image.header));
  query.bindValue(":description", QVariant(image.description));
  query.bindValue(":inventary", QVariant(image.inventary));
  query.bindValue(":location", QVariant(image.location));
  query.bindValue(":cd", QVariant(image.cd));
  query.bindValue(":date", QVariant(image.date));
  query.bindValue(":pid", PMDBImageModel::qVariantFromQList(image.pid));
  query.bindValue(":w", QVariant(image.width));
  query.bindValue(":h", QVariant(image.height));
  query.bindValue(":w_orig", QVariant(image.origWidth));
  query.bindValue(":h_orig", QVariant(image.origHeight));
  query.bindValue(":size", QVariant(image.origSize));
  query.bindValue(":x_dpi", QVariant(image.xDensityDpi));
  query.bindValue(":y_dpi", QVariant(image.yDensityDpi));

  QSqlDatabase::database().transaction();
  bool ok = query.exec();
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), query.lastError().text());
    QSqlDatabase::database().rollback();
    return false;
  } else {
    QSqlDatabase::database().commit();
    return true;
  }
}

void MainWindow::createStatusBar() {
  sizeLabel = new QLabel();
  dimensionsLabel = new QLabel();
  dpiLabel = new QLabel();
  nameLabel = new QLabel();
  nameLabel->setMinimumWidth(200);
  nameLabel->setIndent(4);
  dimensionsLabel->setMinimumWidth(100);
  dimensionsLabel->setIndent(4);
  dpiLabel->setMinimumWidth(100);
  dpiLabel->setIndent(2);
  sizeLabel->setMinimumWidth(100);
  sizeLabel->setIndent(2);

  int a, b, c, d;
  statusBar()->getContentsMargins(&a, &b, &c, &d);
  statusBar()->setContentsMargins(a + 5, b, c, d);
  statusBar()->addWidget(nameLabel, 4);
  statusBar()->addWidget(dimensionsLabel, 1);
  statusBar()->addWidget(sizeLabel, 1);
  statusBar()->addWidget(dpiLabel, 1);
}

void MainWindow::setDbSettings() {
  PMDBSettingsDialog dialog(this);
  dialog.exec();
}

void MainWindow::connectToDb() {
  QSqlDatabase db = QSqlDatabase::database();
  db.close();
  db.setHostName(pm::host);
  db.setPort(pm::port);
  db.setDatabaseName("pm_database");
  db.setUserName(pm::dbUser);
  db.setPassword(pm::dbPass);

  QSqlDatabase db1 = QSqlDatabase::database("second");
  db1.close();
  db1.setHostName(pm::host);
  db1.setPort(pm::port);
  db1.setDatabaseName("pm_database");
  db1.setUserName(pm::dbUser);
  db1.setPassword(pm::dbPass);

  bool ok = db.open() && db1.open();
  if (!ok) {
    QMessageBox::warning(0, tr("Cannot connect to database"),
                         db.lastError().text() + " " + db1.lastError().text());
    return;
  } else {
    catModel->readDataFromDB();
    invModel->setQuery(invQuery);
    organizeInvWidget();
  }
}

void MainWindow::renameCategory() {
  QModelIndex index = catSelectionModel()->selectedIndexes().first();
  if (!index.isValid()) {
    QMessageBox::warning(0, tr("No category selected"),
                         tr("Please select category first"));
    return;
  }
  QString name = modelCategory()->data(index, Qt::NameRole).toString();
  name = QInputDialog::getText(this, tr("Enter name"), tr("Category name:"),
                               QLineEdit::Normal, name);

  if (!name.isEmpty()) {
    modelCategory()->setData(index, QVariant(name), Qt::NameRole);
  }
  return;
}

void MainWindow::showOrphanImages() {
  imageSelectionModel()->clearSelection();
  modelImage()->loadCategoryContents(QModelIndex());
}

QPixmap MainWindow::loadPreview(const PMDBImage &image) {
  quint64 id = image.id;
  if (cache->contains(id)) {
    return cache->loadImage(id);
  } else {
    QPixmap preview;
    QSqlQuery query;
    query.exec("SELECT preview FROM pm_images WHERE id=" + QString::number(id));
    if (query.next()) {
      QByteArray arr = query.value(0).toByteArray();
      preview.loadFromData(arr, "JPG");

      cache->addImage(id, preview);
      return preview;
    } else {
      return preview;
    }
  }
}

QPixmap MainWindow::loadPreview(quint64 id) {
  if (cache->contains(id)) {
    return cache->loadImage(id);
  } else {
    QPixmap preview;
    QSqlQuery query;
    query.exec("SELECT preview FROM pm_images WHERE id=" + QString::number(id));
    if (query.next()) {
      QByteArray arr = query.value(0).toByteArray();
      preview.loadFromData(arr, "JPG");

      cache->addImage(id, preview);
      return preview;
    } else {
      return preview;
    }
  }
}

void MainWindow::searchImages() {
  imageModel->clearModel();
  imageSelection->clear();

  QStringList *searchList = new QStringList();

  PMSearchDialog *dialog = new PMSearchDialog(searchList);
  dialog->exec();

  if (searchList->isEmpty()) {
    delete searchList;
    delete dialog;
    return;
  }

  QString str = searchList->join("&");

  statusBar()->showMessage("Searching...");
  this->setEnabled(false);

  searchThr = new PMSearchThread(this, str);

  connect(searchThr, SIGNAL(finished()), searchThr, SLOT(deleteLater()));
  connect(searchThr, SIGNAL(finished()), this, SLOT(endSearching()));
  connect(searchThr, SIGNAL(searchMsg(int)), this, SLOT(showSearching(int)),
          Qt::DirectConnection);

  searchThr->start();
}

void MainWindow::endAddingImages() {
  this->setEnabled(true);
  statusBar()->clearMessage();
  modelImage()->loadCategoryContents();
}

void MainWindow::showMessageNotLoaded(QString &str) {
  QMessageBox::warning(
      0, tr("Some files already exist!"),
      tr("Following files already exist in database and won't be added: <br>") +
          str);
}

void MainWindow::showPercentageLoaded(int i, QString path) {
  statusBar()->showMessage(
      tr("Loaded %1 %% from %2").arg(QString::number(i)).arg(path));
}

void MainWindow::endSearching() {
  this->setEnabled(true);
  modelImage()->loadSearchResults();
  statusBar()->clearMessage();
  statusBar()->showMessage(
      "Found " + QString::number(modelImage()->rowCount(QModelIndex())) +
          " images",
      5000);
}

void MainWindow::showSearching(int i) {
  QString str = "Searching";
  for (int j = 0; j < i; j++)
    str += ".";
}

void MainWindow::searchImagesById() {
  QString str = QInputDialog::getText(0, tr("Enter ID"), tr("ID: "));
  if (str.isEmpty()) {
    return;
  }
  bool ok = false;
  quint64 id = str.toUInt(&ok);
  if (!ok) {
    return;
  }

  modelImage()->clearModel();
  imageSelectionModel()->clearSelection();
  modelImage()->addImageToModel(id);
}

QPixmap MainWindow::loadFullImage(const QString &location) {
  QPixmap pix;

  pix.load(QDir::toNativeSeparators(pm::dataDir + "/" + location));
  return pix;
}

QPixmap MainWindow::loadFullImage(const quint64 id) {
  QPixmap preview;
  QSqlQuery query;
  query.exec("SELECT image FROM pm_bigimages WHERE id=" + QString::number(id));
  if (query.next()) {
    QByteArray arr = query.value(0).toByteArray();
    preview.loadFromData(arr, "JPG");

    return preview;
  } else {
    return preview;
  }
}

bool MainWindow::removeImageFromDB(PMDBImage &image) {
  QSqlQuery query;
  bool ok =
      query.exec("DELETE FROM pm_images WHERE id=" + QString::number(image.id));
  return ok;
}

void MainWindow::removeSelectedImages() {
  QModelIndexList selectedList = imageSelectionModel()->selectedIndexes();
  if (selectedList.isEmpty()) {
    QMessageBox::warning(0, tr("Selection is empty"),
                         tr("Please select images you wish to remove"));
    return;
  } else if (QMessageBox::question(
                 0, tr("Are you sure?"),
                 tr("Are you sure you want to remove selected image(s)?"),
                 QMessageBox::Yes | QMessageBox::Cancel,
                 QMessageBox::Cancel) == QMessageBox::Yes) {

    bool ok = true;
    statusBar()->showMessage("Removing...");
    foreach (QModelIndex index, selectedList) {
      ok = ok && modelImage()->removeImageFromModelDB(index);
    }
    if (!ok) {
      QMessageBox::warning(
          0, tr("Error removing images from database"),
          tr("Some error occured while removing images from database. ") +
              tr("Not all of them were removed"));
    }
    statusBar()->clearMessage();
  }
}

void MainWindow::readSettings() {
  QString s = "Photo Manager";
  if (isClient()) {
    s += " (client)";
  }
  QSettings settings("Alex Y. Gronsky", s);
  pm::host = settings.value("host", "localhost").toString();
  pm::port = settings.value("port", 5432).toInt();
  pm::dbUser = settings.value("user", "postgres").toString();
  pm::dbPass = settings.value("password", "1234").toString();
  pm::dataDir = settings.value("datadir", pm::dataDir).toString();
  pm::cacheDir = settings.value("cachedir", pm::cacheDir).toString();
  pm::cacheLim = settings.value("cachelim", pm::cacheLim).toInt();
  pm::itemWidth = settings.value("itemwidth", pm::itemWidth).toInt();
  pm::itemHeight = settings.value("itemheight", pm::itemHeight).toInt();
  pm::columnNum = settings.value("columnnum", pm::columnNum).toInt();
}

void MainWindow::writeSettings() {
  QSettings settings("Alex Y. Gronsky", "Photo Manager");
  settings.setValue("host", pm::host);
  settings.setValue("port", pm::port);
  settings.setValue("user", pm::dbUser);
  settings.setValue("password", pm::dbPass);
  settings.setValue("datadir", pm::dataDir);
  settings.setValue("cachedir", pm::cacheDir);
  settings.setValue("cachelim", pm::cacheLim);
  settings.setValue("itemwidth", pm::itemWidth);
  settings.setValue("itemheight", pm::itemHeight);
  settings.setValue("columnnum", pm::columnNum);
  //  qDebug() << "z";
}

void MainWindow::setViewerSettings() {
  PMViewerSettingsDialog dialog(this);
  connect(&dialog, SIGNAL(updatedVals()), imageView, SLOT(metricsUpdated()));
  connect(&dialog, SIGNAL(updatedVals()), this, SLOT(clearCache()));
  dialog.exec();
}

void MainWindow::setCacheSettings() {
  PMCacheSettingsDialog dialog(cache, this);
  dialog.exec();
}

void MainWindow::showLastAddedImages() {
  QString str = QInputDialog::getText(0, tr("Enter number of images to show"),
                                      tr("Number of images to show: "));
  if (str.isEmpty()) {
    return;
  }
  bool ok = false;
  int number = str.toInt(&ok);
  if (!ok) {
    return;
  }

  if (number < 0) {
    return;
  }

  imageSelectionModel()->clearSelection();
  modelImage()->loadLastAdded(number);
}

void MainWindow::importFromAcdseeXml() {
  quint64 added = 0, skipped = 0;
  int res;
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                  qApp->applicationDirPath(),
                                                  tr("XML files (*.xml)"));
  QFile file(fileName);
  QDomDocument doc("ACDSee info");

  if (!file.open(QIODevice::ReadOnly))
    return;
  if (!doc.setContent(&file)) {
    qDebug() << "Bad XML file!";
    file.close();
    return;
  }

  QString dirPath = QFileDialog::getExistingDirectory(
      0, tr("Choose an ACDSee data directory"), pm::dataDir,
      QFileDialog::ShowDirsOnly);
  if (dirPath.isEmpty()) {
    return;
  }

  dirPath = QDir::fromNativeSeparators(QDir::cleanPath(dirPath));

  QDomElement root = doc.documentElement();
  if (root.tagName() != "ACDDB") {
    return;
  }

  QDomNode assetList = root.namedItem("AssetList");
  if (assetList.isNull()) {
    return;
  }

  this->setEnabled(false);

  QDir dir(QDir::toNativeSeparators(dirPath));
  QDomNode asset = assetList.firstChild();
  int i = 0;
  int allNodesNumber = assetList.childNodes().count();

  while (!asset.toElement().isNull()) {
    if (asset.toElement().tagName() == "Asset") {
      res = processAcdseeXmlAsset(asset, dir, (quint64)i);
      if (res) {
        skipped++;
      } else {
        added++;
      }
      showPercentageLoaded((++i) * 100 / allNodesNumber, dir.absolutePath());
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    asset = asset.nextSibling();
  }

  this->setEnabled(true);
  qDebug() << "XML import: added " << added << ". Skipped " << skipped << ".";
  return;
}

int MainWindow::processAcdseeXmlAsset(const QDomNode &asset, const QDir &dir,
                                      quint64 number) {
  QDomNode current;
  QString name, location, header, description, fileType;
  QDate date(1970, 1, 1);
  QList<quint64> pids;

  enum {
    AssetOk,
    AssetName,
    AssetFolder,
    AssetCaption,
    AssetNotes,
    AssetLocationExists,
    AssetNoFile
  } res;

  res = AssetOk;

  current = asset.namedItem("Name");
  if (current.toElement().isNull()) {
    res = AssetName;
    qDebug() << QString("Rec. No. %1. Lack of <Name> tag. Skip").arg(number);
    return 1;
  }
  name = current.toElement().text();

  current = asset.namedItem("Folder");
  if (current.toElement().isNull()) {
    res = AssetFolder;
    qDebug() << QString("Rec. No. %1. Lack of <Folder> tag. Skip").arg(number);
    return 1;
  }
  location = current.toElement().text();
  QStringList list =
      location.split(QRegExp("(/|\\\\)"), QString::SkipEmptyParts);
  location = "";
  bool foundRoot = false;
  foreach (QString s, list) {
    if (foundRoot) {
      location += s + "/";
    }
    if (s == dir.dirName()) {
      foundRoot = true;
    }
  }
  location = QDir::fromNativeSeparators(QDir::cleanPath(location));

  current = asset.namedItem("Caption");
  if (current.toElement().isNull()) {
    res = AssetCaption;
    qDebug()
        << QString("Rec. No. %1. Lack of <Caption> tag. Continue").arg(number);
    header = "";
  } else {
    header = current.toElement().text();
  }

  current = asset.namedItem("Notes");
  if (current.toElement().isNull()) {
    res = AssetNotes;
    qDebug()
        << QString("Rec. No. %1. Lack of <Notes> tag. Continue").arg(number);
    description = "";
  } else {
    description = current.toElement().text();

    QRegExp dateRx("\\d{2}\\.\\d{2}\\.(\\d{2}|\\d{4})");
    int pos = dateRx.indexIn(description);

    QStringList captured = dateRx.capturedTexts();
    if (pos >= 0) {
      QString dateStr = captured.first();
      if (dateStr.length() > 8) {
        date = QDate::fromString(dateStr, "dd.MM.yyyy");
      } else {
        date = QDate::fromString(dateStr, "dd.MM.yy");
        if (date.year() < 1990) {
          date = date.addYears(100);
        }
      }

      if (!date.isValid()) {
        date = QDate(1970, 1, 1);
      }
    }
  }

  current = asset.namedItem("AssetCategoryList");
  if (!current.toElement().isNull()) {
    QDomNodeList assetCatList = current.toElement().childNodes();
    for (int i = 0; i < assetCatList.count(); i++) {
      QDomNode assetCat = assetCatList.at(i);
      QString categPath = assetCat.toElement().text();
      quint64 id = processAcdseeXmlAssetCat(categPath);
      if (id) {
        pids.append(id);
      }
    }
  }
  if (pids.isEmpty()) {
    pids.append(0);
  }

  QDir currDir(QDir::toNativeSeparators(dir.canonicalPath() + "/" + location));
  if (findImageInDB(location + "/" + name)) {
    res = AssetLocationExists;
    qDebug() << QString("Rec. No. %1. File: ").arg(number)
             << location + "/" + name << "already exists. Skip.";
    return 1;
  }

  QPixmap newPixmap = QPixmap(QDir::toNativeSeparators(
      dir.canonicalPath() + "/" + location + "/" + name));
  QPixmap newBigPix = newPixmap;
  if (newPixmap.isNull()) {
    res = AssetNoFile;
    qDebug() << "Rec. No. " << number
             << " File doesn't exist and won't be added to DB: "
             << QDir::toNativeSeparators(pm::dataDir + "/" + location + "/" +
                                         name)
             << "! Skip";
    return 1;
  }

  if (newPixmap.width() > newPixmap.height()) {
    newPixmap =
        newPixmap.scaledToWidth(pm::previewSize, Qt::SmoothTransformation);
  } else {
    newPixmap =
        newPixmap.scaledToHeight(pm::previewSize, Qt::SmoothTransformation);
  }
  PMDBImage newImage;
  newImage.name = name;
  newImage.description = description;
  newImage.header = header;
  newImage.pid = pids;
  newImage.preview = newPixmap;
  if (QRegExp("\\d+").exactMatch(currDir.dirName())) {
    newImage.cd = currDir.dirName().toInt();
    currDir.cd("..");

    currDir.cd(QDir::toNativeSeparators(pm::dataDir + "/" + location));
  } else {
    newImage.cd = "";
  }
  newImage.location = location + "/" + name;
  newImage.date = date;

  addImageToDB(newImage, newBigPix);
  return 0;
}

quint64 MainWindow::processAcdseeXmlAssetCat(const QString &catPath) {
  QStringList splitPath =
      catPath.split(QRegExp("\\\\|/"), QString::SkipEmptyParts);
  if (splitPath.isEmpty()) {
    return 0;
  }

  QString name, prevName;
  quint64 pid;

  int i = splitPath.count() - 1;

  name = splitPath.at(i);

  QSqlQuery query, query2;
  bool ok =
      query.exec("SELECT id, pid FROM pm_categories WHERE lower(name)=lower('" +
                 name + "')");
  if (ok && query.size() <= 0) {
    return 0;
  }
  if (query.size() == 1) {
    query.next();
    return query.value(0).toUInt();
  } else {
    splitPath.removeLast();
    QString newPath = splitPath.join("/");
    pid = processAcdseeXmlAssetCat(newPath);
    while (query.next()) {
      if (pid == query.value(1).toUInt()) {
        return query.value(0).toUInt();
      }
    }
    return 0;
  }
}

QString MainWindow::nameLatinToUpper(const QString name) {
  QString result;
  for (int i = 0; i < name.length(); i++) {
    QChar ch = name.at(i);
    if (ch <= QChar('z') && ch >= QChar('a')) {
      result.append(ch.toUpper());
    } else {
      result.append(ch);
    }
  }
  return result;
}

void MainWindow::addNewInventaryItem() {
  QString dirPath, commonDescription;
  bool ok = false, insertOk;
  int maxDepth = 1;
  int createInv = 0;
  PMInv newInv;
  PMAddInvDialog dialog(&newInv, &ok, &dirPath, &commonDescription, &createInv,
                        &maxDepth, this);
  dialog.exec();
  if (!ok) {
    return;
  }
  QSqlRecord rec = PMInv::invToSqlRecord(newInv);

  QSqlQuery q;
  q.prepare(
      "INSERT INTO pm_inv (name, author, header, coloured, internal_usage, type, date, comments) \
            values(:name, :author, :header, :coloured, :internal_usage, :type, :date, :comments)");
  q.bindValue(":name", rec.value("name"));
  q.bindValue(":author", rec.value("author"));
  q.bindValue(":header", rec.value("header"));
  q.bindValue(":coloured", rec.value("coloured"));
  q.bindValue(":internal_usage", rec.value("internal_usage"));
  q.bindValue(":type", rec.value("type"));
  q.bindValue(":date", rec.value("date"));
  q.bindValue(":comments", rec.value("comments"));

  insertOk = q.exec();

  if (!insertOk) {
    QMessageBox::warning(this, tr("Error adding new folder!"),
                         tr("Database error occured.") +
                             modelInv()->lastError().text());
    return;
  }
  QSqlQuery q2("SELECT MAX(id) FROM pm_inv");
  if (q2.next()) {
    newInv.id = q2.value(0).toUInt();
  } else {
    QMessageBox::warning(this, tr("Error adding new folder!"),
                         tr("Database error occured."));
    return;
  }

  dirPath = QDir::fromNativeSeparators(QDir::cleanPath(dirPath));

  statusBar()->showMessage(tr("Loading..."));
  this->setEnabled(false);

  QDir dir(QDir::toNativeSeparators(dirPath));

  QStringList filters;
  filters << "*.jpg"
          << "*.jpeg";
  QStringList dirFilters;
  QStringList fileList = dir.entryList(filters, QDir::Files | QDir::Readable);
  QStringList dirList = dir.entryList(dirFilters, QDir::Dirs | QDir::Readable |
                                                      QDir::NoDotAndDotDot);
  QString location = dirPath;
  location.remove(pm::dataDir, Qt::CaseInsensitive); // location in pm::dataDir
  while (location.startsWith("/")) {
    location.remove(0, 1);
  }

  QPixmap newPixmap, bigPix;
  PMDBImage newImage;
  newImage.pid.append(0);
  for (int i = 0; i < fileList.count(); i++) {
    bool ok;
    QPixmap bigPix;
    readImageFile(newImage, bigPix, dirPath, fileList.at(i), commonDescription,
                  newInv.header, QString(""), newInv.id, newInv.date, ok);

    addImageToDB(newImage, bigPix);
    //    emit percentageLoaded(i*100/fileList.count());
    showPercentageLoaded(i * 100 / fileList.count(), dir.absolutePath());
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  if (maxDepth == 1 && createInv == 0) {
    foreach (QString s, dirList) {
      if (s == "." || s == "..") {
        continue;
      }
      dir.cd(QDir::toNativeSeparators(dir.absolutePath()) + QDir::separator() +
             s);
      fileList = dir.entryList(filters, QDir::Files | QDir::Readable);
      for (int i = 0; i < fileList.count(); i++) {
        bool ok;
        QPixmap bigPix = QPixmap(QDir::toNativeSeparators(dir.canonicalPath()) +
                                 QDir::separator() + fileList.at(i));
        readImageFile(newImage, bigPix, dir.canonicalPath(), fileList.at(i),
                      commonDescription, newInv.header, s, newInv.id,
                      newInv.date, ok);

        addImageToDB(newImage, bigPix);
        showPercentageLoaded(i * 100 / fileList.count(), dir.absolutePath());
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
      dir.cd("..");
    }
  } else {
    QString rootName = newInv.name;
    foreach (QString s, dirList) {
      if (s == "." || s == "..") {
        continue;
      }
      bool ok = dir.cd(QDir::toNativeSeparators(dir.absolutePath()) +
                       QDir::separator() + s);
      if (ok) {
        addNewInventaryItem2(dir, newInv, createInv, 1, maxDepth,
                             commonDescription, rootName);
        dir.cd("..");
      }
    }
  }

  modelInv()->setQuery(mWin->invQuery);
  this->setEnabled(true);
  statusBar()->clearMessage();
  modelImage()->loadInventaryContents(newInv.id);
}

void MainWindow::addNewInventaryItem2(QDir &dir, PMInv &inv, int createInv,
                                      int depth, int maxDepth,
                                      QString &commonDescription,
                                      QString path) {
  if (depth > maxDepth) {
    return;
  }
  QString newPath;
  if (createInv) {
    QString s = dir.dirName();
    if (s == "") {
      inv.name = path + "/" + "NA";
    } else {
      inv.name = path + "/" + s;
    }
    newPath = inv.name;
    QSqlRecord rec = PMInv::invToSqlRecord(inv);

    QSqlQuery q;
    q.prepare(
        "INSERT INTO pm_inv (name, author, header, coloured, internal_usage, type, date, comments) \
              values(:name, :author, :header, :coloured, :internal_usage, :type, :date, :comments)");
    q.bindValue(":name", rec.value("name"));
    q.bindValue(":author", rec.value("author"));
    q.bindValue(":header", rec.value("header"));
    q.bindValue(":coloured", rec.value("coloured"));
    q.bindValue(":internal_usage", rec.value("internal_usage"));
    q.bindValue(":type", rec.value("type"));
    q.bindValue(":date", rec.value("date"));
    q.bindValue(":comments", rec.value("comments"));

    bool insertOk = q.exec();

    if (!insertOk) {
      QMessageBox::warning(this, tr("Error adding new folder!"),
                           tr("Database error occured.") +
                               modelInv()->lastError().text());
      return;
    }
    QSqlQuery q2("SELECT MAX(id) FROM pm_inv");
    if (q2.next()) {
      inv.id = q2.value(0).toUInt();
    } else {
      QMessageBox::warning(this, tr("Error adding new folder!"),
                           tr("Database error occured."));
      return;
    }
  }

  QStringList filters;
  filters << "*.jpg"
          << "*.jpeg";
  QStringList dirFilters;
  QStringList fileList = dir.entryList(filters, QDir::Files | QDir::Readable);
  QStringList dirList = dir.entryList(dirFilters, QDir::Dirs | QDir::Readable |
                                                      QDir::NoDotAndDotDot);
  QString location = dir.absolutePath();
  //    location.remove(pm::dataDir, Qt::CaseInsensitive); // location in
  //    pm::dataDir
  while (location.startsWith("/")) {
    location.remove(0, 1);
  }

  QPixmap newPixmap, bigPix;
  PMDBImage newImage;
  newImage.pid.append(0);
  for (int i = 0; i < fileList.count(); i++) {
    bool ok;
    QPixmap bigPix;
    readImageFile(newImage, bigPix, dir.absolutePath(), fileList.at(i),
                  commonDescription, inv.header, QString(""), inv.id, inv.date,
                  ok);

    addImageToDB(newImage, bigPix);
    //    emit percentageLoaded(i*100/fileList.count());
    showPercentageLoaded(i * 100 / fileList.count(), dir.absolutePath());
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  foreach (QString s, dirList) {
    if (s == "." || s == "..") {
      continue;
    }
    bool ok = dir.cd(QDir::toNativeSeparators(dir.absolutePath()) +
                     QDir::separator() + s);
    if (ok) {
      addNewInventaryItem2(dir, inv, createInv, depth + 1, maxDepth,
                           commonDescription, newPath);
      dir.cd("..");
    }
  }
}

void MainWindow::removeSelectedInventary() {
  if (invSelection->selectedIndexes().isEmpty()) {
    QMessageBox::warning(0, tr("No folders selected"),
                         tr("Please select an folder first"));
  } else if (QMessageBox::question(
                 0, tr("Are you sure?"),
                 tr("Are you sure you want to remove selected folder?"),
                 QMessageBox::Yes | QMessageBox::Cancel,
                 QMessageBox::Cancel) == QMessageBox::Yes) {
    this->statusBar()->showMessage(tr("Removing folder item... Please wait."));
    this->setEnabled(false);
    QModelIndex index = invSelection->selectedIndexes().first();
    index = invModel->index(index.row(), 0);
    QSqlQuery q1("SELECT pid FROM pm_images WHERE inventary=" +
                 invModel->data(index).toString());
    QSet<quint64> catIds;
    catIds += 0;
    while (q1.next()) {
      QList<quint64> l = PMDBImageModel::qListFromQVariant(q1.value(0));
      catIds += catIds.fromList(l);
    }
    QString amount = "0";
    foreach (quint64 i, catIds) {
      amount += "," + QString::number(i);
    }
    QSqlDatabase::database().transaction();
    QSqlQuery q2("UPDATE pm_categories SET outdated='1' WHERE id IN(" + amount +
                 ")");
    QSqlQuery q("DELETE FROM pm_inv WHERE id=" +
                invModel->data(index).toString());
    QSqlDatabase::database().commit();
    invModel->setQuery(mWin->invQuery);
    this->setEnabled(true);
    this->statusBar()->clearMessage();
  }
}

void MainWindow::saveSelectedInventary() {
  if (invSelection->selectedIndexes().isEmpty()) {
    QMessageBox::warning(0, tr("No folders selected"),
                         tr("Please select an folder first"));
    return;
  }

  QString dirName = QFileDialog::getExistingDirectory(
      this, tr("Select destination directory"), QString());
  if (dirName.trimmed().isEmpty()) {
    return;
  }

  this->statusBar()->showMessage(tr("Saving folder... Please wait."));
  this->setEnabled(false);

  QModelIndex index = invSelection->selectedIndexes().first();
  saveInventaryToDisk(index, dirName);
  this->setEnabled(true);
  this->statusBar()->clearMessage();
}

void MainWindow::saveSelectedImagesToLocal() {
  QModelIndexList selectedList = imageSelectionModel()->selectedIndexes();
  if (selectedList.isEmpty()) {
    QMessageBox::warning(0, tr("Selection is empty"),
                         tr("Please select an image you wish to save"));
    return;
  } else {
    QString dirName = QFileDialog::getExistingDirectory(
        this, tr("Select destination folder"), QString());
    if (dirName.trimmed().isEmpty()) {
      return;
    }
    statusBar()->showMessage(tr("Saving file(s)..."));
    this->setEnabled(false);

    QDir binDir(dirName);
    dirName = binDir.canonicalPath();

    foreach (QModelIndex index, selectedList) {
      quint64 id = modelImage()->data(index, Qt::IdRole).toUInt();
      saveImageToDisk(id, dirName);
    }
    statusBar()->clearMessage();

    this->setEnabled(true);
  }
}

void MainWindow::addBigimagesToDbFromLocation(const QString &dataDir) {
  QSqlQuery q, q2;
  q.exec("SELECT id, location FROM pm_images");
  QString loc, name, dataDir1;
  if (dataDir.isEmpty()) {
  } else {
    dataDir1 = dataDir + "/";
  }
  quint64 id;

  while (q.next()) {
    id = q.value(0).toUInt();
    loc = q.value(1).toString();

    QPixmap newBigPix = QPixmap(QDir::toNativeSeparators(dataDir1 + loc));
    if (newBigPix.isNull()) {
      qDebug() << "Rec. No. " << id
               << " File doesn't exist and won't be added to DB: "
               << QDir::toNativeSeparators(dataDir1 + loc) << "! Skip";
    } else {
      q2.clear();
      QByteArray bytesBig;
      QBuffer bigBuffer(&bytesBig);
      bigBuffer.open(QIODevice::WriteOnly);
      newBigPix.save(&bigBuffer, "JPG");
      bigBuffer.close();

      q2.prepare(
          "INSERT INTO pm_bigimages (id, image) VALUES (:id, E:preview)");
      q2.bindValue(":id", QVariant(id));
      if (!bytesBig.isEmpty()) {
        q2.bindValue(":preview", QVariant(bytesBig));
      } else {
        q2.bindValue(":preview", QVariant(" "));
      }
      q2.exec();
    }
  }
}

void MainWindow::clearCache() {
  cache->clear();
  QPixmapCache::clear();
}

void MainWindow::saveInventaryToDisk(const QModelIndex &index,
                                     const QString &dirName) {
  if (!index.isValid()) {
    qDebug() << "Invalid index in saveInventaryToDisk";
    return;
  }

  PMInv inv;

  QDir dir(dirName);
  if (!dir.exists()) {
    qDebug() << "Dir not exist in saveInventaryToDisk";
    QMessageBox::warning(mWin, tr("Not exisiting directory"),
                         tr("Not existing directory. Aborted."));
    return;
  }

  QSqlRecord rec = mWin->modelInv()->record(index.row());
  inv = PMInv::sqlRecordToInv(rec);

  bool ok = dir.mkdir(inv.name + "_" + QString::number(inv.id));
  if (!ok) {
    qDebug() << "Couldn't create directory: " + inv.name + "_" +
                    QString::number(inv.id);
    QMessageBox::warning(mWin, tr("Error"),
                         tr("Unable to create directory. Aborted."));
    return;
  }

  dir.cd(inv.name + "_" + QString::number(inv.id));

  QSqlQuery q, q2;
  QStringList subFolders;

  ok = q.exec("SELECT DISTINCT cd FROM pm_images WHERE inventary=" +
              QString::number(inv.id));
  if (!ok) {
    QMessageBox::warning(mWin, tr("Database error"), q.lastError().text());
    return;
  }
  while (q.next()) {
    subFolders << q.value(0).toString();
  }
  //  qDebug() << subFolders;
  if (!subFolders.isEmpty()) {
    foreach (QString s, subFolders) {
      if (!s.isEmpty()) {
        ok = dir.mkdir(s);
        if (!ok) {
          qDebug() << "Couldn't create directory: " + s;
          QMessageBox::warning(mWin, tr("Error"),
                               tr("Unable to create directory. Aborted."));
          return;
        }
      }
      q2.clear();
      ok = q2.exec("SELECT id FROM pm_images WHERE inventary=" +
                   QString::number(inv.id) + " AND cd='" + s + "'");
      while (q2.next()) {
        if (!s.isEmpty()) {
          saveImageToDisk(q2.value(0).toUInt(),
                          dir.absolutePath() + QDir::separator() + s);
        } else {
          saveImageToDisk(q2.value(0).toUInt(), dir.absolutePath());
        }

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
      }
    }
  }
}

void MainWindow::saveImageToDisk(quint64 id, const QString &dirPath,
                                 const QString &ext, const QString &type) {
  QDir dir(dirPath);
  if (!dir.exists()) {
    qDebug() << "Dir: " << dirPath << " doesn't exist.";
    return;
  }

  QPixmap pix = loadFullImage(id);
  QString saveSetting;
  if (isEditor()) {
    saveSetting = getDbVar("editor_saving");
  } else {
    saveSetting = getDbVar("client_saving");
  }

  bool permission = true;

  if (!saveSetting.isEmpty()) {
    QStringList l = saveSetting.split(" ", QString::SkipEmptyParts);
    if (l.at(0) == "1") {
      int w = l.at(1).toInt();
      int h = l.at(2).toInt();
      if (pix.width() > w) {
        pix = pix.scaledToWidth(w, Qt::SmoothTransformation);
      }
      if (pix.height() > h) {
        pix = pix.scaledToHeight(h, Qt::SmoothTransformation);
      }
    } else if (l.at(0) == "-1") {
      permission = false;
    }
  }

  pix.save(dir.absolutePath() + QDir::separator() + "ID_" +
               QString::number(id) + "." + ext,
           type.toAscii().constData(), 100);
}

QString MainWindow::getDbVar(const QString &varName) {
  QSqlQuery q;
  bool ok = q.exec("SELECT value FROM pm_aux WHERE name='" + varName + "'");
  if (!ok) {
    return QString();
  }

  if (q.next()) {
    if (q.value(0).toString() == "")
      return QString();
    else
      return q.value(0).toString();
  }

  return QString();
}

void MainWindow::readImageFile(PMDBImage &newImage, QPixmap &destPix,
                               const QString &fromDir, const QString &fileName,
                               const QString &description,
                               const QString &header, const QString &cd,
                               quint64 inv, const QDate &date, bool &ok) {
  ok = true;
  QFile file(fromDir + QDir::separator() + fileName);
  file.open(QIODevice::ReadOnly);
  if (file.error()) {
    qDebug() << "Error opening file: "
             << fromDir + QDir::separator() + fileName + " "
             << (int)(file.error());
    QMessageBox::warning(mWin, tr("Error opening file"),
                         fromDir + QDir::separator() + fileName);
    ok = false;
    return;
  }
  QByteArray rawData = file.readAll();
  QByteArray hex = rawData.toHex();
  hex.truncate(30);
  //  qDebug() << hex;
  newImage.origSize = ((double)rawData.size()) / 1024;

  QDataStream stream(rawData);
  QBuffer buf(&rawData);
  const char app0[] = {0xFF, 0xE0, 0x00};
  const char app1[] = {0xFF, 0xE1, 0x00};
  int iter = rawData.indexOf(app0);
  newImage.xDensityDpi = newImage.yDensityDpi = 0;
  char c;
  if (rawData.indexOf(app1) >= 0) {
    //    qDebug() << "!";
    iter = rawData.indexOf(app1);
    iter += 10;
    c = rawData.at(iter);
    //    qDebug() << c;
    QDataStream::ByteOrder type;
    if (c == 'I') {
      type = QDataStream::LittleEndian;
    } else {
      type = QDataStream::BigEndian;
    }

    stream.setByteOrder(type);

    stream.skipRawData(iter + 4);
    quint32 offset;
    stream >> offset;
    //    qDebug() << offset << " offset";
    stream.skipRawData(offset - 8);
    quint16 num;
    stream >> num;
    //    qDebug() << num << " num";

    quint32 xResOffset = 0, yResOffset = 0;
    int unit = -1;
    for (unsigned int i = 0; i < num; i++) {
      quint16 tag, unitVal;
      quint32 val;
      stream >> tag;
      if (tag == 0x011a) {
        //        qDebug() << 1;
        stream.skipRawData(6);
        stream >> val;
        xResOffset = val;
        //        qDebug() << xResOffset;
      } else if (tag == 0x011b) {
        //        qDebug() << 2;
        stream.skipRawData(6);
        stream >> val;
        yResOffset = val;
        //        qDebug() << yResOffset;
      } else if (tag == 0x0128) {
        //        qDebug() << 3;
        stream.skipRawData(6);
        stream >> unitVal;
        unit = unitVal;
      } else {
        stream.skipRawData(10);
      }
    }

    if (xResOffset * yResOffset == 0) {
      //      qDebug() << "bad";
      newImage.xDensityDpi = newImage.yDensityDpi = 0;
    } else {
      //      qDebug() << "good";
      stream.unsetDevice();
      buf.close();
      buf.open(QIODevice::ReadOnly);
      buf.seek(0);
      stream.setDevice(&buf);
      stream.skipRawData(iter + xResOffset);
      char t;
      buf.getChar(&t);
      buf.ungetChar(t);
      quint32 a, b;
      stream >> a >> b;
      if (b == 0) {
        newImage.xDensityDpi = 0;
      } else {
        newImage.xDensityDpi = (int)((double)a / b);
      }

      stream.unsetDevice();
      buf.close();
      buf.open(QIODevice::ReadOnly);
      buf.seek(0);
      stream.setDevice(&buf);
      stream.skipRawData(iter + yResOffset);
      stream >> a >> b;
      if (b == 0) {
        newImage.yDensityDpi = 0;
      } else {
        newImage.yDensityDpi = (int)(a / (double)b);
      }
      if (unit == 3) {
        newImage.xDensityDpi *= -1;
        newImage.yDensityDpi *= -1;
      } else if (unit == 1) {
        newImage.xDensityDpi *= -1;
      } else if (unit < 1) {
        newImage.xDensityDpi = newImage.yDensityDpi = 0;
      }
    }
  }

  if ((newImage.xDensityDpi * newImage.yDensityDpi == 0) &&
      rawData.indexOf(app0) >= 0) {
    //    qDebug() << "why?!";
    double multX = 1.0, multY = 1.0;
    iter = rawData.indexOf("JFIF");
    if (iter >= 0) {
      iter += 7;
      c = rawData.at(iter);
      if (c == 0x00) {
        multX = -1.0; // aspect ratio
      } else if (c == 0x02) {
        multX = multY = -1.0; // unit - cm
      }
      stream.unsetDevice();
      buf.close();
      buf.open(QIODevice::ReadOnly);
      buf.seek(0);
      stream.setDevice(&buf);
      stream.setByteOrder(QDataStream::BigEndian);
      stream.skipRawData(iter + 1);
      quint16 x, y;
      stream >> x >> y;
      newImage.xDensityDpi = (int)(x * multX);
      newImage.yDensityDpi = (int)(y * multY);
    }
  }

  QString var = getDbVar("upload_size");
  QStringList l = var.split(" ", QString::SkipEmptyParts);
  destPix = QPixmap(fromDir + QDir::separator() + fileName);
  QPixmap preview = destPix;
  newImage.origWidth = destPix.width();
  newImage.origHeight = destPix.height();
  if (l.at(0) == "1") {
    int w = l.at(1).toInt();
    int h = l.at(2).toInt();
    if (destPix.width() > w) {
      destPix = destPix.scaledToWidth(w, Qt::SmoothTransformation);
    }
    if (destPix.height() > h) {
      destPix = destPix.scaledToHeight(h, Qt::SmoothTransformation);
    }
  } else if (l.at(0) == "0") {
  } else {
  }

  newImage.width = destPix.width();
  newImage.height = destPix.height();

  if (preview.width() > preview.height()) {
    preview = preview.scaledToWidth(pm::previewSize, Qt::SmoothTransformation);
  } else {
    preview = preview.scaledToHeight(pm::previewSize, Qt::SmoothTransformation);
  }

  newImage.name = fileName;
  newImage.description = description;
  newImage.header = header;
  newImage.preview = preview;
  newImage.location = fromDir;
  newImage.cd = cd;
  newImage.inventary = inv;
  newImage.date = date;

  //  qDebug() << newImage.name << " " << newImage.xDensityDpi << " " <<
  //  newImage.yDensityDpi;
}

void MainWindow::organizeInvWidget() {
  invModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
  invModel->setHeaderData(6, Qt::Horizontal, tr("Type"));
  invModel->setHeaderData(3, Qt::Horizontal, tr("Header"));

  invWidget->treeView()->setColumnHidden(0, true);
  invWidget->treeView()->setColumnHidden(2, true);
  invWidget->treeView()->setColumnHidden(4, true);
  invWidget->treeView()->setColumnHidden(8, true);
  invWidget->treeView()->setColumnHidden(5, true);
  invWidget->treeView()->setColumnHidden(7, true);
  invWidget->treeView()->setColumnWidth(3, 150);
  invWidget->treeView()->setColumnWidth(6, 50);
}
