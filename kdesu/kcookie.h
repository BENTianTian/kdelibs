/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __KCookie_h_Included__
#define __KCookie_h_Included__

#include <qbytearray.h>

/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 and DCOP cookies).
 */

class KDESU_EXPORT KCookie
{
public:
    KCookie();

    /**
     * Returns the X11 display.
     */
    QByteArray display() const { return m_Display; }

#ifdef Q_WS_X11
    /**
     * Returns the X11 magic cookie, if available.
     */
    QByteArray displayAuth() const { return m_DisplayAuth; }
#endif

    /**
     * Select the DCOP transport to look for. Default: "local"
     */
    void setDcopTransport(const QByteArray &dcopTransport);

    /**
     * Returns the netid where the dcopserver is running
     */
    QByteArray dcopServer();

    /**
     * Returns a list of magic cookies for DCOP protocol authentication.
     * The order is the same as in dcopServer().
     */
    QByteArray dcopAuth();

    /**
     * Returns a list of magic cookies for the ICE protocol.
     */
    QByteArray iceAuth();

private:
    void getXCookie();
    void getICECookie();

    void blockSigChild();
    void unblockSigChild();

    bool m_bHaveDCOPCookies;
    bool m_bHaveICECookies;

    QByteArray m_Display;
#ifdef Q_WS_X11
    QByteArray m_DisplayAuth;
#endif
    QByteArray m_DCOPSrv;
    QByteArray m_DCOPAuth;
    QByteArray m_ICEAuth;
    QByteArray m_dcopTransport;

    class KCookiePrivate;
    KCookiePrivate *d;
};


#endif // __KCookie_h_Included__
