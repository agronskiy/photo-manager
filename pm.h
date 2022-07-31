#ifndef PM_H_INCLUDED
#define PM_H_INCLUDED

class MainWindow;
class QString;

extern MainWindow *mWin;

namespace pm {
enum {
  Id = 0,
  Name,
  Header,
  Description,
  Inventary,
  Location,
  Cd,
  Pid,
  Date,
  Preview
};

extern int ver;
extern int branch;
extern int revision;

extern QString dataDir;
extern QString host;
extern int port;
extern QString dbUser;
extern QString dbPass;
extern int previewSize;
extern int cacheLim;
extern QString cacheDir;
extern QString progName;
extern int itemWidth;
extern int itemHeight;
extern int columnNum;
extern QString version;
extern bool client;

enum UpdateFlags { NoPreview = 0x01, WithPreview = 0x02 };

typedef UpdateFlags LoadFlags;
} // namespace pm

namespace Qt {
enum {
  CatIdRole = 32,
  PreviewRole = 33,
  IdRole,
  HeaderRole,
  NameRole,
  LocationRole,
  PidRole,
  DateRole,
  DescriptionRole,
  InventaryRole,
  CdRole,
  AmountRole,
  WidthRole,
  HeightRole,
  OrigWidthRole,
  OrigHeightRole,
  DpiRole,
  OrigSizeRole
};
}

#endif // PM_H_INCLUDED
