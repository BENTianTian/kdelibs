/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __SSH_h_Included__
#define __SSH_h_Included__

#include "stub.h"

#include <kdelibs_export.h>

/**
 * Executes a remote command, using ssh.
 */

class KDESU_EXPORT SshProcess: public StubProcess
{
public:
    SshProcess(const QByteArray &host = QByteArray(), const QByteArray &user = QByteArray(),
               const QByteArray &command = QByteArray());
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /**
     * Sets the target host.
     */
    void setHost(const QByteArray &host) { m_Host = host; }

    /**
     * Sets the localtion of the remote stub.
     */
    void setStub(const QByteArray &stub);

    /** 
     * Checks if the current user\@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     *
     * @todo The return doc is so obviously wrong that the C code needs to be checked.
     */
    int checkNeedPassword();

    /**
     * Checks if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Executes the command.
     */
    int exec(const char *password, int check=0);

    QByteArray prompt() const  { return m_Prompt; }
    QByteArray error() const { return m_Error; }

protected:
    virtual QByteArray display();
    virtual QByteArray displayAuth();
    virtual QByteArray dcopServer();

private:
    QByteArray dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort;
    int  m_dcopSrv;
    QByteArray m_Prompt;
    QByteArray m_Host;
    QByteArray m_Error;
    QByteArray m_Stub;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SshProcessPrivate;
    SshProcessPrivate *d;
};

#endif
