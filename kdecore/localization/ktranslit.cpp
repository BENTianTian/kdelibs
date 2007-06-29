/*  This file is part of the KDE libraries    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <config.h>

#include <ktranslit_p.h>

#include <QHash>

// -----------------------------------------------------------------------------
// Base class.

class KTranslitPrivate
{
};

KTranslit::KTranslit ()
: d(NULL)
{
}

KTranslit::~KTranslit ()
{
    delete d;
}

KTranslit *KTranslit::create (const QString &lang)
{
    if (lang == QString::fromAscii("sr")) {
        return new KTranslitSerbian();
    }
    else {
        return NULL;
    }
}

QStringList KTranslit::fallbackList (const QString &lang)
{
    QStringList fallbacks;

    if (   lang == QString::fromAscii("sr@latin")
        || lang == QString::fromAscii("sr@Latn")) {
        fallbacks += QString::fromAscii("sr");
    }

    return fallbacks;
}

void splitLangScript (const QString &lang, QString &ln, QString &scr)
{
    ln = lang;
    scr.clear();
    int pos = lang.indexOf('@');
    if (pos >= 0) {
        ln = lang.left(pos);
        scr = lang.mid(pos + 1);
    }
}

QString KTranslit::higherPriorityScript (const QString &lang,
                                         const KLocale *locale)
{
    if (locale == NULL) {
        return QString();
    }

    // Split into pure language and script part.
    QString ln, scr;
    splitLangScript(lang, ln, scr);

    // Search through higher priority languages.
    QString finalScrHi;
    if (lang != KLocale::defaultLanguage()) {
        foreach (const QString &langHi, locale->languageList()) {
            // Don't search lower priority languages.
            if (langHi == lang)
                break;

            // Split current spec into pure language and script parts.
            QString lnHi, scrHi;
            splitLangScript(langHi, lnHi, scrHi);

            // Return current script if languages match.
            if (lnHi == ln) {
                finalScrHi = scrHi;
                break;
            }
        }
    }
    return finalScrHi;
}

QString KTranslit::transliterate (const QString &str,
                                  const QString &script) const
{
    Q_UNUSED(script);
    return str;
}

// -----------------------------------------------------------------------------
// Serbian.

class KTranslitSerbianPrivate
{
    public:
    QHash<QString, bool> latinNames;
    QHash<QChar, QString> dictC2L;
};

KTranslitSerbian::KTranslitSerbian ()
: d(new KTranslitSerbianPrivate())
{
    d->latinNames[QString::fromAscii("latin")] = true;
    d->latinNames[QString::fromAscii("Latn")] = true;

    #define SR_DICTC2L_ENTRY(a, b) do { \
        d->dictC2L[QString::fromUtf8(a)[0]] = QString::fromUtf8(b); \
    } while (0)
    SR_DICTC2L_ENTRY("а", "a");
    SR_DICTC2L_ENTRY("б", "b");
    SR_DICTC2L_ENTRY("в", "v");
    SR_DICTC2L_ENTRY("г", "g");
    SR_DICTC2L_ENTRY("д", "d");
    SR_DICTC2L_ENTRY("ђ", "đ");
    SR_DICTC2L_ENTRY("е", "e");
    SR_DICTC2L_ENTRY("ж", "ž");
    SR_DICTC2L_ENTRY("з", "z");
    SR_DICTC2L_ENTRY("и", "i");
    SR_DICTC2L_ENTRY("ј", "j");
    SR_DICTC2L_ENTRY("к", "k");
    SR_DICTC2L_ENTRY("л", "l");
    SR_DICTC2L_ENTRY("љ", "lj");
    SR_DICTC2L_ENTRY("м", "m");
    SR_DICTC2L_ENTRY("н", "n");
    SR_DICTC2L_ENTRY("њ", "nj");
    SR_DICTC2L_ENTRY("о", "o");
    SR_DICTC2L_ENTRY("п", "p");
    SR_DICTC2L_ENTRY("р", "r");
    SR_DICTC2L_ENTRY("с", "s");
    SR_DICTC2L_ENTRY("т", "t");
    SR_DICTC2L_ENTRY("ћ", "ć");
    SR_DICTC2L_ENTRY("у", "u");
    SR_DICTC2L_ENTRY("ф", "f");
    SR_DICTC2L_ENTRY("х", "h");
    SR_DICTC2L_ENTRY("ц", "c");
    SR_DICTC2L_ENTRY("ч", "č");
    SR_DICTC2L_ENTRY("џ", "dž");
    SR_DICTC2L_ENTRY("ш", "š");
    SR_DICTC2L_ENTRY("А", "A");
    SR_DICTC2L_ENTRY("Б", "B");
    SR_DICTC2L_ENTRY("В", "V");
    SR_DICTC2L_ENTRY("Г", "G");
    SR_DICTC2L_ENTRY("Д", "D");
    SR_DICTC2L_ENTRY("Ђ", "Đ");
    SR_DICTC2L_ENTRY("Е", "E");
    SR_DICTC2L_ENTRY("Ж", "Ž");
    SR_DICTC2L_ENTRY("З", "Z");
    SR_DICTC2L_ENTRY("И", "I");
    SR_DICTC2L_ENTRY("Ј", "J");
    SR_DICTC2L_ENTRY("К", "K");
    SR_DICTC2L_ENTRY("Л", "L");
    SR_DICTC2L_ENTRY("Љ", "Lj");
    SR_DICTC2L_ENTRY("М", "M");
    SR_DICTC2L_ENTRY("Н", "N");
    SR_DICTC2L_ENTRY("Њ", "Nj");
    SR_DICTC2L_ENTRY("О", "O");
    SR_DICTC2L_ENTRY("П", "P");
    SR_DICTC2L_ENTRY("Р", "R");
    SR_DICTC2L_ENTRY("С", "S");
    SR_DICTC2L_ENTRY("Т", "T");
    SR_DICTC2L_ENTRY("Ћ", "Ć");
    SR_DICTC2L_ENTRY("У", "U");
    SR_DICTC2L_ENTRY("Ф", "F");
    SR_DICTC2L_ENTRY("Х", "H");
    SR_DICTC2L_ENTRY("Ц", "C");
    SR_DICTC2L_ENTRY("Ч", "Č");
    SR_DICTC2L_ENTRY("Џ", "Dž");
    SR_DICTC2L_ENTRY("Ш", "Š");
}

KTranslitSerbian::~KTranslitSerbian ()
{
    delete d;
}

QString KTranslitSerbian::transliterate (const QString &str,
                                         const QString &script) const
{
    if (d->latinNames.contains(script)) {
        // NOTE: This loop has been somewhat optimized for speed.
        int slen = str.length();
        QString nstr;
        nstr.reserve(slen + 5);
        for (int i = 0; i < slen; ++i) {
            QChar c = str[i];
            QString r = d->dictC2L[c];
            if (!r.isEmpty()) {
                if (   r.length() > 1
                    && (   (i + 1 < slen && str[i + 1].isUpper())
                        || (i > 0 && str[i - 1].isUpper()))) {
                    nstr.append(r.toUpper());
                }
                else {
                    nstr.append(r);
                }
            }
            else {
                nstr.append(c);
            }
        }
        return nstr;
    }
    else {
        return str;
    }
}
