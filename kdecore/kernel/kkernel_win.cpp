/*
   This file is part of the KDE libraries
   Copyright (C) 2007 Christian Ehrlicher <ch.ehrlicher@gmx.de>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kkernel_win.h"

#include <config.h>
#include <QtCore/QBool>

#ifdef Q_OS_WIN
#include <kdebug.h>

#include <QtCore/QDir>
#include <QApplication>

#include <windows.h>
#include <shellapi.h>
#include <process.h>

#if defined(__MINGW32__)
# define WIN32_CAST_CHAR (const WCHAR*)
#else
# define WIN32_CAST_CHAR (LPCWSTR)
#endif

static void kMessageOutput(QtMsgType type, const char *msg);

static class kGlobalClass {
    public: 
        kGlobalClass() {
            qInstallMsgHandler(kMessageOutput);
            kde4prefix[0] = 0;
        }

        wchar_t kde4prefix[MAX_PATH + 1];
} myGlobals; 

static HINSTANCE kdecoreDllInstance = NULL;

/**
 * The dll entry point - get the instance handle for GetModuleFleNameW
 * Maybe also some special initialization / cleanup can be done here
 **/
BOOL WINAPI DllMain ( HINSTANCE hinstDLL,DWORD fdwReason,LPVOID )
{
    switch ( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        kdecoreDllInstance = hinstDLL;
        break;
    default:
        break;
    }
    return true;
}

// can't use QCoreApplication::applicationDirPath() because sometimes we
// don't have an instantiated QCoreApplication
QString getKde4Prefix()
{
    if (myGlobals.kde4prefix[0] == 0) {
        //the path is C:\some\path\kde4\bin\kdecore.dll
        GetModuleFileNameW(kdecoreDllInstance, myGlobals.kde4prefix, MAX_PATH + 1);
        int bs1 = 0, bs2 = 0;

        //we convert \ to / and remove \bin\kdecore.dll from the string
        int pos;
        for (pos = 0; pos < MAX_PATH + 1 && myGlobals.kde4prefix[pos] != 0; ++pos) {
            if (myGlobals.kde4prefix[pos] == '\\') {
                bs1 = bs2;
                bs2 = pos;
                myGlobals.kde4prefix[pos] = '/';
            }
        }
        Q_ASSERT(bs1);
        Q_ASSERT(pos < MAX_PATH + 1);
        myGlobals.kde4prefix[bs1] = 0;
    }
    return QString::fromUtf16((ushort *) myGlobals.kde4prefix);
}

/**
 \return a value from MS Windows native registry.
 @param key is usually one of HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE
        constants defined in WinReg.h.
 @param subKey is a registry subkey defined as a path to a registry folder, eg.
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
        ('\' delimiter must be used)
 @param item is an item inside subKey or "" if default folder's value should be returned
 @param ok if not null, will be set to true on success and false on failure
*/
QString getWin32RegistryValue ( HKEY key, const QString& subKey, const QString& item, bool *ok = 0 )
{
#define FAILURE \
 { if (ok) \
  *ok = false; \
 return QString(); }

    if ( subKey.isEmpty() )
        FAILURE;
    HKEY hKey;
    TCHAR *lszValue;
    DWORD dwType=REG_SZ;
    DWORD dwSize;

    if ( ERROR_SUCCESS!=RegOpenKeyExW ( key, WIN32_CAST_CHAR subKey.utf16(), 0, KEY_READ, &hKey ) )
        FAILURE;

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey, WIN32_CAST_CHAR item.utf16(), NULL, NULL, NULL, &dwSize ) )
        FAILURE;

    lszValue = new TCHAR[dwSize];

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey, WIN32_CAST_CHAR item.utf16(), NULL, &dwType, ( LPBYTE ) lszValue, &dwSize ) ) {
        delete [] lszValue;
        FAILURE;
    }
    RegCloseKey ( hKey );

    QString res = QString::fromUtf16 ( ( const ushort* ) lszValue );
    delete [] lszValue;
    return res;
}


bool showWin32FilePropertyDialog ( const QString& fileName )
{
    QString path_ = QDir::convertSeparators ( QFileInfo ( fileName ).absoluteFilePath() );

    SHELLEXECUTEINFOW execInfo;
    memset ( &execInfo,0,sizeof ( execInfo ) );
    execInfo.cbSize = sizeof ( execInfo );
    execInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    const QString verb ( QLatin1String ( "properties" ) );
    execInfo.lpVerb = WIN32_CAST_CHAR verb.utf16();
    execInfo.lpFile = WIN32_CAST_CHAR path_.utf16();
    return ShellExecuteExW ( &execInfo );
}

QByteArray getWin32LocaleName()
{
    bool ok;
    QString localeNumber = getWin32RegistryValue ( HKEY_CURRENT_USER,
                           QLatin1String("Control Panel\\International"),
                           "Locale", &ok );
    if ( !ok )
        return QByteArray();
    QString localeName = getWin32RegistryValue ( HKEY_LOCAL_MACHINE,
                         QLatin1String("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes"),
                         localeNumber, &ok );
    if ( !ok )
        return QByteArray();
    return localeName.toLatin1();
}

/**
 \return a value from MS Windows native registry for shell folder \a folder.
*/
QString getWin32ShellFoldersPath ( const QString& folder )
{
    return getWin32RegistryValue ( HKEY_CURRENT_USER,
                                   QLatin1String("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                                   folder );
}

void kMessageOutput(QtMsgType type, const char *msg)
{
#if 1
    int BUFSIZE=4096;
    char *buf = new char[BUFSIZE];
    switch (type) {
        case QtDebugMsg:
            strlcpy(buf,"Debug:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtWarningMsg:
            strlcpy(buf,"Warning:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtCriticalMsg:
            strlcpy(buf,"Critial:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtFatalMsg:
            strlcpy(buf,"Fatal:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            //abort();
            break;
    }
    strlcat(buf,"\n",BUFSIZE);
    OutputDebugStringA(buf);
    delete[] buf;
#else
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        //abort();
    }
#endif
}

#endif  // Q_OS_WIN
