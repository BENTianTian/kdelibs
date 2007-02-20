/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QSet>
#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <kdirwatch.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "knfsshare.h"

class KNFSSharePrivate
{
public:
  KNFSSharePrivate();

  bool readExportsFile();
  bool findExportsFile();

  QSet<QString> sharedPaths;
  QString exportsFile;
};

KNFSSharePrivate::KNFSSharePrivate()
{
  if (findExportsFile())
      readExportsFile();
}

/**
 * Try to find the nfs config file path
 * First tries the kconfig, then checks
 * several well-known paths
 * @return wether an 'exports' file was found.
 **/
bool KNFSSharePrivate::findExportsFile() {
  KConfig knfsshare("knfsshare");
  KConfigGroup config(&knfsshare, "General");
  exportsFile = config.readPathEntry("exportsFile");

  if ( QFile::exists(exportsFile) )
    return true;

  if ( QFile::exists("/etc/exports") )
    exportsFile = "/etc/exports";
  else {
    kDebug(7000) << "KNFSShare: Could not found exports file!" << endl;
    return false;
  }

  config.writeEntry("exportsFile",exportsFile);
  return true;
}

/**
 * Reads all paths from the exports file
 * and fills the sharedPaths dict with the values
 */
bool KNFSSharePrivate::readExportsFile() {
  QFile f(exportsFile);

  kDebug(7000) << "KNFSShare::readExportsFile " << exportsFile << endl;

  if (!f.open(QIODevice::ReadOnly)) {
    kError() << "KNFSShare: Could not open " << exportsFile << endl;
    return false;
  }

  sharedPaths.clear();

  QTextStream s( &f );

  bool continuedLine = false; // is true if the line before ended with a backslash
  QString completeLine;

  while ( !s.atEnd() )
  {
    QString currentLine = s.readLine().trimmed();

    if (continuedLine) {
      completeLine += currentLine;
      continuedLine = false;
    }
    else
      completeLine = currentLine;

    // is the line continued in the next line ?
    if ( completeLine.endsWith(QLatin1String("\\")) )
    {
      continuedLine = true;
      // remove the ending backslash
      completeLine.chop(1);
      continue;
    }

    // comments or empty lines
    if (completeLine.startsWith(QLatin1String("#")))
    {
      continue;
    }

    QString path;

    // Handle quotation marks
    if ( completeLine.startsWith(QLatin1String("\"")) ) {
      int i = completeLine.indexOf(QLatin1Char('"'), 1);
      if (i == -1) {
        kError() << "KNFSShare: Parse error: Missing quotation mark: " << completeLine << endl;
        continue;
      }
      path = completeLine.mid(1,i-1);

    } else { // no quotation marks
      int i = completeLine.indexOf(QLatin1Char(' '));
      if (i == -1)
          i = completeLine.indexOf(QLatin1Char('\t'));

      if (i == -1)
        path = completeLine;
      else
        path = completeLine.left(i);

    }

    kDebug(7000) << "KNFSShare: Found path: " << path << endl;

    // normalize path
    if ( !path.endsWith(QLatin1String("/")) )
             path += QLatin1Char('/');

    sharedPaths.insert(path);
  }

  f.close();

  return true;

}

KNFSShare::KNFSShare() {
  d = new KNFSSharePrivate();
  if (QFile::exists(d->exportsFile)) {
    KDirWatch::self()->addFile(d->exportsFile);
    connect(KDirWatch::self(), SIGNAL(dirty (const QString&)),this,
   	        SLOT(slotFileChange(const QString&)));
  }
}

KNFSShare::~KNFSShare() {
  if (QFile::exists(d->exportsFile)) {
    KDirWatch::self()->removeFile(d->exportsFile);
  }
  delete d;
}


bool KNFSShare::isDirectoryShared( const QString & path ) const {
  QString fixedPath = path;
  if ( path[path.length()-1] != '/' )
       fixedPath += '/';

  return d->sharedPaths.contains(fixedPath);
}

QStringList KNFSShare::sharedDirectories() const {
  return d->sharedPaths.values();
}

QString KNFSShare::exportsPath() const {
  return d->exportsFile;
}



void KNFSShare::slotFileChange( const QString & path ) {
  if (path == d->exportsFile)
     d->readExportsFile();

  emit changed();
}

KNFSShare* KNFSShare::_instance = 0L;
static KStaticDeleter<KNFSShare> ksdNFSShare;

KNFSShare* KNFSShare::instance() {
  if (! _instance )
      _instance = ksdNFSShare.setObject(_instance, new KNFSShare());

  return _instance;
}

#include "knfsshare.moc"

