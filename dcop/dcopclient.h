/*
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DCOPCLIENT_H
#define _DCOPCLIENT_H

#include <qobject.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qstring.h>

class DCOPObjectProxy;
class DCOPClientPrivate;
class DCOPClientTransaction;

typedef QValueList<QCString> QCStringList;

/**
 * Provide inter-process communication and remote procedure calls
 * for KDE applications.
 * This class provides IPC and RPC for KDE applications.  Usually you
 * will not have to instantiate one yourself because @ref KApplication
 * contains a method to return a pointer to a @ref DCOPClient object which
 * can be used for your whole application.
 *
 * Before being able to send or receive any DCOP messages, you will have
 * to attach your client object to the DCOP server, and then register
 * your application with a specific name. See @ref attach()
 * and @ref registerAs() for
 * more information.
 *
 * Data to be sent should be serialized into a @ref QDataStream which was
 * initialized with the @ref QByteArray that you actually intend to send
 * the data in.  An example of how you might do this:
 *
 * <pre>
 *   QByteArray data;
 *   QDataStream dataStream(data, IO_WriteOnly);
 *   dataStream << QString("This is text I am serializing");
 *   client->send("someApp", "someObject", "someFunction", data);
 * </pre>
 *
 * @see KApplication::dcopClient()
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOPClient : public QObject
{
  Q_OBJECT

 public:
  /**
   * Create a new DCOP client, but do not attach to any server.  */
  DCOPClient();

  /**
   * Clean up any open connections and dynamic data.
   */
  virtual ~DCOPClient();

  /**
   * Specify the address of a server to use upon attaching.
   *
   * If no server address is ever specified, attach will try its best to
   * find the server anyway.
   */
  static void setServerAddress(const QCString &addr);

  /**
   * Attach to the DCOP server.
   *
   * If the connection was already attached,
   * the connection will be re-established with the current server address.
   *
   * Naturally, only attached application can use DCOP services.
   *
   * If a @ref QApplication object exists then client registers itself as
   * @ref QApplication->name() + "-" + <pid>.
   * If no @ref QApplication object exists the client registers itself as
   * "anonymous".
   *
   * If you want to register differently, you should use @ref registerAs()
   * instead.
   *
   * @return @p true if attaching was successful.
   */
  bool attach();

  /**
   * @internal
   * Internal function for @ref KUniqueApplication to register the @ref DCOPClient
   * with the application in case the application didn't exist at the
   * time the @ref DCOPClient was created.
   */
  void bindToApp();

  /**
   * Detach from the DCOP server.
   */
  bool detach();

  /**
   * Query whether or not the client is attached to the server.
   */
  bool isAttached() const;

  /**
   * Register at the DCOP server.
   *
   * If the application was already registered,
   * the registration will be re-done with the new @ref appId.
   *
   * @p appId is a @em unique application/program id that the server
   * will use to associate requests with. If there is already an application
   * registered with the same name, the server will add a number to the
   * id to unify it. If @p addPID is true, the PID of the current process
   * will be added to id.
   *
   * Registration is necessary if you want to allow other clients to talk
   * to you.  They can do so using your @p appId as first parameter
   * for @ref send() or @ref call(). If you just want to talk to
   *  other clients, you
   * do not need to register at the server. In that case @ref attach() is
   * enough.
   * It will implicitly register you as "anonymous".
   *
   * @return The actual @p appId used for the registration or a null string
   * if the registration wasn't successful.
   */
  QCString registerAs( QCString appId, bool addPID = true );

  /**
   * Query whether or not the client is registered at the server.
   */
  bool isRegistered() const;

 /**
  * Returns the current app id or a null string if the application
  * hasn't yet been registered.
  */
  QCString appId() const;


  /**
   * @return The socket over which DCOP is communicating with the server.
   */
  int socket() const;

  /**
   * Send a data block to the server.
   *
   * @param remApp The remote application id.
   * @param remObj The name of the remote object.
   * @param remFun The remote function in the specified object to call.
   * @param data The data to provide to the remote function.
   * @param fast Tf set to @p true, a "fast" form of IPC will be used.
   *        Fast connections are not guaranteed to be implemented, but
   *        if they are they work only on the local machine, not across
   *        the network.  "fast" is only a hint not an order.
   *
   * @return Whether or not the server was able to accept the send.
   */
  bool send(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data,
	    bool fast=false);

  /**
   * This function acts exactly the same as the above, but the data
   * parameter can be specified as a @ref QString for convenience.
   */
  bool send(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QString &data,
	    bool fast=false);

  /**
   * Perform a synchronous send and receive.
   *
   *  The parameters are
   * the same as for send, with the exception of another @ref QByteArray
   * being provided for results to be (optionally) returned in.
   *
   * @see send()
   */
  bool call(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data,
	    QCString& replyType, QByteArray &replyData, bool fast=false);


  /**
   * Reimplement to handle app-wide function calls unassociated w/an object.
   *
   * Note that @p fun is normalized. See @ref normalizeFunctionSignature().
   *
   * If you do not want to reimplement this function for whatever reason,
   * you can also use a @ref DCOPObjectProxy.
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);

  /**
   * Delay the reply of the current function call
   * until @ref endTransaction() is called.
   *
   * This allows a server to queue requests.
   *
   * NOTE: Should be called from inside process(...) only!
   */
  DCOPClientTransaction *beginTransaction( );

  /**
   * Send the delayed reply of a function call.
   */
  void endTransaction( DCOPClientTransaction *, QCString& replyType, QByteArray &replyData);

  /**
   * Test whether the current function call is delayed.
   *
   * NOTE: Should be called from inside process(...) only!
   * @return The ID of the current transaction
   *         0 if no transaction is going on.
   */
  Q_INT32 transactionId();

  /**
   * Check whether @p remApp is registered with the @ref DCOPServer.
   * @return @p true if the remote application is registered, otherwise @p false.
   */
  bool isApplicationRegistered( const QCString& remApp);

  /**
   * Retrieve the list of all currently registered applications.
   */
  QCStringList registeredApplications();

  /**
   * Receive a piece of data from the server.
   *
   * @param app The application the data was intended for.  Should be
   *        equal to our appId that we passed when the @ref DCOPClient was
   *        created.
   * @param obj The name of the object to pass the data on to.
   * @param fun The name of the function in the object to call.
   * @param data The arguments for the function.
   * @internal
   */
  bool receive(const QCString &app, const QCString &obj,
	       const QCString &fun, const QByteArray& data,
	       QCString& replyType, QByteArray &replyData);



  /**
   * Normalizes the function signature @p fun.
   *
   * A normalized signature doesn't contain any unnecessary whitespace
   * anymore. The remaining whitespace consists of single blanks only (0x20).
   *
   * Example for a normalized signature:
   * <pre>
   *   "void someFunction(QString,int)"
   * </pre>
   *
   * When using @ref send() or @ref call(), normlization is done
   * automatically for you.
   *
   */
  static QCString normalizeFunctionSignature( const QCString& fun );


  /*
   * Retrieve the @p appId of the last application that talked to us.
   */
  QCString senderId() const;


    /*
     * Install object @p objId as application-wide default object.
     *
     * All app-wide messages that have not been processed by the dcopclient
     * will be send further to @p objId.
     */
    void setDefaultObject( const QCString& objId );

    /*
     * Retrieve the current default object or an empty string if no object is
     * installed as default object.
     *
     * A default object receives application-wide messages that have not
     * been processed by the @ref DCOPClient itself.
     */
    QCString defaultObject() const;

    /**
     * Enable / disable the @ref applicationRegistered() / @ref applicationRemoved()
     * signals.
     *
     * They are disabled by default.
     */
    void setNotifications( bool enabled );

signals:
  /**
   * Indicates that the application @p appId has been registered with
   * the server we are attached to.
   *
   * You need to call @ref setNotifications() first, to tell the @ref DCOPServer
   * that you want to get these events
   */
  void applicationRegistered( const QCString& appId );
  /**
   * Indicates that the formerly registered application @p appId has
   * been removed.
   *
   * You need to call @ref setNotifications() first, to tell the @ref DCOPServer
   * that you want to get these events
   */
  void applicationRemoved( const QCString& appId );

  /**
   * Indicates that the process of establishing DCOP communications failed
   * in some manner.
   *  Usually attached to a dialog box or some other visual
   * aid.
   */
    void attachFailed(const QString &msg);

public slots:

    protected slots:
    void processSocketData(int socknum);

 protected:

 private:
    DCOPClientPrivate *d;

    friend class DCOPObjectProxy;
    void installObjectProxy( DCOPObjectProxy*);
    void removeObjectProxy( DCOPObjectProxy*);
    bool attachInternal( bool registerAsAnonymous = TRUE );


};

#endif
