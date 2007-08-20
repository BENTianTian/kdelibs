/* This file is part of the KDE libraries
   Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>

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

#include "kremoteencoding.h"

#include <config.h>

#include <kdebug.h>
#include <kstringhandler.h>

KRemoteEncoding::KRemoteEncoding(const char *name)
  : codec(0), d(0)
{
  setEncoding(name);
}

KRemoteEncoding::~KRemoteEncoding()
{
  // delete d;		// not necessary yet
}

QString KRemoteEncoding::decode(const QByteArray& name) const
{
#ifdef CHECK_UTF8
  if (codec->mibEnum() == 106 && !KStringHandler::isUtf8(name))
    return QLatin1String(name);
#endif

  QString result = codec->toUnicode(name);
  if (codec->fromUnicode(result) != name)
    // fallback in case of decoding failure
    return QLatin1String(name);

  return result;
}

QByteArray KRemoteEncoding::encode(const QString& name) const
{
  QByteArray result = codec->fromUnicode(name);
  if (codec->toUnicode(result) != name)
    return name.toLatin1();

  return result;
}

QByteArray KRemoteEncoding::encode(const KUrl& url) const
{
  return encode(url.path());
}

QByteArray KRemoteEncoding::directory(const KUrl& url, bool ignore_trailing_slash) const
{
  QString dir = url.directory(ignore_trailing_slash ? KUrl::DirectoryOptions(0) : KUrl::ObeyTrailingSlash);

  return encode(dir);
}

QByteArray KRemoteEncoding::fileName(const KUrl& url) const
{
  return encode(url.fileName());
}

const char *KRemoteEncoding::encoding() const
{
    return codec->name();
}

int KRemoteEncoding::encodingMib() const
{
    return codec->mibEnum();
}

void KRemoteEncoding::setEncoding(const char *name)
{
  // don't delete codecs

  if (name)
    codec = QTextCodec::codecForName(name);
  else
    codec = QTextCodec::codecForMib( 106 ); // fallback to UTF-8

  if (codec == 0)
    codec = QTextCodec::codecForMib(4 /* latin-1 */);

  Q_ASSERT(codec);

  kDebug() << "setting encoding " << codec->name()
	    << " for name=" << name << endl;
}

void KRemoteEncoding::virtual_hook(int, void*)
{
}
