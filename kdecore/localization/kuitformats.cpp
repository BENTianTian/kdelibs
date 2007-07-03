/*  This file is part of the KDE libraries
    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kuitformats_p.h>

#include <config.h>

#include <QStringList>
#include <QRegExp>

#include <kdebug.h>
#include <klocale.h>

static QString insertIntegerSeparators (const QString &istr,
                                        const QChar &sep, int ngrp)
{
    int len = istr.length();
    int flen = (len / ngrp + 1) * (ngrp + 1);
    QString fistr(flen, ' ');
    for (int i = 0, fi = 0; i < len; ++i, ++fi) {
        if (i > 0 && i % 3 == 0) {
            fistr[flen - fi - 1] = sep;
            ++fi;
        }
        fistr[flen - fi - 1] = istr[len - i - 1];
    }
    fistr = fistr.trimmed();
    return fistr;
}

QString KuitFormats::toNumberUS (const QString &numstr)
{
    int len = numstr.length();
    int p1 = 0;
    while (p1 < len && !numstr[p1].isDigit()) {
        ++p1;
    }
    if (p1 == len) {
        return numstr;
    }
    int p2 = p1 + 1;
    while (p2 < len && numstr[p2].isDigit()) {
        ++p2;
    }

    QString pre = numstr.left(p1);
    QString mid = insertIntegerSeparators(numstr.mid(p1, p2 - p1), ',', 3);
    QString post = numstr.mid(p2);

    return pre + mid + post;
}

QString KuitFormats::toNumberEuro (const QString &numstr)
{
    int len = numstr.length();
    int p1 = 0;
    while (p1 < len && !numstr[p1].isDigit()) {
        ++p1;
    }
    if (p1 == len) {
        return numstr;
    }
    int p2 = p1 + 1;
    while (p2 < len && numstr[p2].isDigit()) {
        ++p2;
    }

    QString pre = numstr.left(p1);
    QString mid = insertIntegerSeparators(numstr.mid(p1, p2 - p1), '.', 3);

    QString post = numstr.mid(p2);
    if (post.startsWith('.')) {
        post[0] = ',';
    }

    return pre + mid + post;
}

QString KuitFormats::toKeyCombo (const QString &shstr, const QString &delim,
                                 const QHash<QString, QString> &keydict)
{
    static QRegExp delRx("[+-]");

    int p = delRx.indexIn(shstr); // find delimiter

    QStringList keys;
    if (p < 0) { // single-key shortcut, no delimiter found
        keys.append(shstr);
    }
    else { // multi-key shortcut
        QChar oldDelim = shstr[p];
        keys = shstr.split(oldDelim, QString::SkipEmptyParts);
    }

    for (int i = 0; i < keys.size(); ++i) {
        // Normalize key, trim and all lower-case.
        QString nkey = keys[i].trimmed().toLower();
        keys[i] = keydict.contains(nkey) ? keydict[nkey] : nkey;
    }
    return keys.join(delim);
}

QString KuitFormats::toInterfacePath (const QString &inpstr,
                                      const QString &delim)
{
    static QRegExp delRx("/|\\||->");

    int p = delRx.indexIn(inpstr); // find delimiter
    if (p < 0) { // single-element path, no delimiter found
        return inpstr;
    }
    else { // multi-element path
        QString oldDelim = delRx.capturedTexts().at(0);
        QStringList guiels = inpstr.split(oldDelim, QString::SkipEmptyParts);
        return guiels.join(delim);
    }
}

