// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

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

#ifndef __kio_jobclasses_h__
#define __kio_jobclasses_h__

#include <qobject.h>
#include <qlinkedlist.h>
#include <qstringlist.h>
#include <qpointer.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kurl.h>
#include <kio/global.h>

#include <kjob.h>

class Observer;
class QTimer;

#define KIO_COPYJOB_HAS_SETINTERACTIVE // new in 3.4. Used by kio_trash.

namespace KIO {

    class Slave;
    class SlaveInterface;


    /**
     * The base class for all jobs.
     * For all jobs created in an application, the code looks like
     *
     * \code
     *   KIO::Job * job = KIO::someoperation( some parameters );
     *   connect( job, SIGNAL( result( KJob * ) ),
     *            this, SLOT( slotResult( KJob * ) ) );
     * \endcode
     *   (other connects, specific to the job)
     *
     * And slotResult is usually at least:
     *
     * \code
     *  if ( job->error() )
     *      job->showErrorDialog( this or 0L  );
     * \endcode
     * @see KIO::Scheduler
     * @see KIO::Slave
     */
    class KIO_EXPORT Job : public KJob {
        Q_OBJECT

    protected:
        Job( bool showProgressInfo );

    public:
        virtual ~Job();
        void start() {} // Since KIO autostarts its jobs

        /**
         * Abort this job.
         * This kills all subjobs and deletes the job.
	 *
         * @param quietly if false, Job will emit signal result
         * and ask kio_uiserver to close the progress window.
         * @p quietly is set to true for subjobs. Whether applications
         * should call with true or false depends on whether they rely
         * on result being emitted or not.
         */
        virtual bool doKill();

        /**
         * Converts an error code and a non-i18n error message into an
         * error message in the current language. The low level (non-i18n)
         * error message (usually a url) is put into the translated error
         * message using %1.
         *
         * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
	 * \code
         *   i18n( "Could not read\n%1" ).arg( errortext );
	 * \endcode
         * Use this to display the error yourself, but for a dialog box
         * use Job::showErrorDialog. Do not call it if error()
	 * is not 0.
	 * @return the error message and if there is no error, a message
	 *         telling the user that the app is broken, so check with
	 *         error() whether there is an error
         */
        QString errorString() const;

        /**
         * Converts an error code and a non-i18n error message into i18n
         * strings suitable for presentation in a detailed error message box.
         *
         * @param reqUrl the request URL that generated this error message
         * @param method the method that generated this error message
         * (unimplemented)
         * @return the following strings: caption, error + description,
	 *         causes+solutions
         */
        QStringList detailedErrorStrings(const KUrl *reqUrl = 0L,
                                         int method = -1) const;

        /**
         * Display a dialog box to inform the user of the error given by
         * this job.
         * Only call if error is not 0, and only in the slot connected
         * to result.
         * @param parent the parent widget for the dialog box, can be 0 for
	 *        top-level
         */
        void showErrorDialog( QWidget * parent = 0L );

        /**
         * Enable or disable the automatic error handling. When automatic
         * error handling is enabled and an error occurs, then showErrorDialog()
         * is called with the specified @p parentWidget (if supplied) , right before
         * the emission of the result signal.
         *
         * The default is false.
         *
         * See also isAutoErrorHandlingEnabled , showErrorDialog
         *
         * @param enable enable or disable automatic error handling
         * @param parentWidget the parent widget, passed to showErrorDialog.
	 *        Can be 0 for top-level
	 * @see isAutoErrorHandlingEnabled()
         */
        void setAutoErrorHandlingEnabled( bool enable, QWidget *parentWidget = 0 );

        /**
         * Returns whether automatic error handling is enabled or disabled.
         * See also setAutoErrorHandlingEnabled .
	 * @return true if automatic error handling is enabled
	 * @see setAutoErrorHandlingEnabled()
         */
        bool isAutoErrorHandlingEnabled() const;

        /**
         * Enable or disable the automatic warning handling. When automatic
         * warning handling is enabled and an error occurs, then a message box
         * is displayed with the warning message
         *
         * The default is true.
         *
         * See also isAutoWarningHandlingEnabled , showErrorDialog
         *
         * @param enable enable or disable automatic warning handling
         * @see isAutoWarningHandlingEnabled()
         */
        void setAutoWarningHandlingEnabled( bool enable );

        /**
         * Returns whether automatic warning handling is enabled or disabled.
         * See also setAutoWarningHandlingEnabled .
         * @return true if automatic warning handling is enabled
         * @see setAutoWarningHandlingEnabled()
         */
        bool isAutoWarningHandlingEnabled() const;

        /**
         * Enable or disable the message display from the job.
         *
         * The default is true.
         * @param enable enable or disable message display
         */
        void setInteractive(bool enable);

        /**
         * Returns whether message display is enabled or disabled.
         * See also setInteractive .
         * @return true if message display is enabled
         * @see setInteractive()
         */
        bool isInteractive() const;
        /**
         * Associate this job with a window given by @p window.
	 * @param window the window to associate to
	 * @see window()
         */
        void setWindow(QWidget *window);

        /**
         * Returns the window this job is associated with.
	 * @return the associated window
	 * @see setWindow()
         */
        QWidget *window() const;

        /**
         * Set the parent Job.
         * One example use of this is when FileCopyJob calls open_RenameDlg,
         * it must pass the correct progress ID of the parent CopyJob
         * (to hide the progress dialog).
	 * You can set the parent job only once. By default a job does not
	 * have a parent job.
	 * @param parentJob the new parent job
         */
        void setParentJob( Job* parentJob );

        /**
	 * Returns the parent job, if there is one.
	 * @return the parent job, or 0 if there is none
         * @see setParentJob
         */
        Job* parentJob() const;

        /**
         * Set meta data to be sent to the slave, replacing existing
	 * meta data.
	 * @param metaData the meta data to set
	 * @see addMetaData()
	 * @see mergeMetaData()
         */
        void setMetaData( const KIO::MetaData &metaData);

        /**
         * Add key/value pair to the meta data that is sent to the slave.
	 * @param key the key of the meta data
	 * @param value the value of the meta data
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const QString &key, const QString &value);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will be overridden.
	 * @param values the meta data to add
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const QMap<QString,QString> &values);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will remain unchanged.
	 * @param values the meta data to merge
	 * @see setMetaData()
	 * @see addMetaData()
         */
        void mergeMetaData(const QMap<QString,QString> &values);

        /**
         * @internal. For the scheduler. Do not use.
         */
        MetaData outgoingMetaData() const;

        /**
         * Get meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @return the job's meta data
         */
        MetaData metaData() const;

        /**
         * Query meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @param key the key of the meta data to retrieve
	 * @return the value of the meta data, or QString() if the
	 *         @p key does not exist
         */
        QString queryMetaData(const QString &key);

    Q_SIGNALS:
        /**
         * @deprecated. Don't use !
         * Emitted when the job is canceled.
         * Signal result() is emitted as well, and error() is,
         * in this case, ERR_USER_CANCELED.
	 * @param job the job that emitted this signal
         */
        void canceled( KJob *job );

        /**
         * Emitted when the slave successfully connected to the host.
         * There is no guarantee the slave will send this, and this is
         * currently unused (in the applications).
	 * @param job the job that emitted this signal
         */
        void connected( KIO::Job *job );

        /**
         * Emitted to display information about the speed of this job.
	 * @param job the job that emitted this signal
	 * @param speed the speed in bytes/s
         */
        void speed( KIO::Job *job, unsigned long speed );

    protected Q_SLOTS:
        /**
         * Called whenever a subjob finishes.
         * Default implementation checks for errors and propagates
         * to parent job, then calls removeSubjob.
         * Override if you don't want subjobs errors to be propagated,
         * or if you want this job to keep running after the last subjob finished.
	 * @param job the subjob
	 * @see result()
         */
        virtual void slotResult( KJob *job );

        void slotFinished( KJob *job, int id );

        /**
         * Forward signal from subjob.
	 * @param job the subjob
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( KIO::Job *job, unsigned long speed );
        /**
         * Forward signal from subjob.
	 * @param job the subjob
	 * @param msg the info message
	 * @see infoMessage()
         */
        void slotInfoMessage( KJob *job, const QString &msg );

        /**
         * Remove speed information.
         */
        void slotSpeedTimeout();

    protected:
        /**
         * Add a job that has to be finished before a result
         * is emitted. This has obviously to be called before
         * the finish signal is emitted by the slave.
         *
	 * @param job the subjob to add
         * @param inheritMetaData if true, the subjob will
         * inherit the meta data from this job.
         */
        virtual void addSubjob( Job *job, bool inheritMetaData=true );

        /**
         * Mark a sub job as being done.
         *
         * KDE4 change: this doesn't terminate the parent job anymore, call emitResult to do that.
	 *
	 * @param job the subjob to remove
         * @param mergeMetaData if set, the metadata received by the subjob is
         *                      merged into this job.
         */
        void removeSubjob( KJob *job, bool mergeMetaData = false );

        /**
         * @return true if we still have subjobs running
         */
        bool hasSubjobs() const { return !m_subjobs.isEmpty(); }

        /**
         * @return the full list of sub jobs
         */
        QList<Job *> subjobs() const { return m_subjobs; }

        /**
         * Utility function for inherited jobs.
         * Emits the speed signal and starts the timer for removing that info
	 *
	 * @param speed the speed in bytes/s
         */
        void emitSpeed( unsigned long speed );

        /**
         * @internal
         * Some extra storage space for jobs that don't have their own
         * private d pointer.
         */
        enum { EF_TransferJobAsync    = (1 << 0),
               EF_TransferJobNeedData = (1 << 1),
               EF_TransferJobDataSent = (1 << 2),
               EF_ListJobUnrestricted = (1 << 3) };
        int &extraFlags();

        MetaData m_incomingMetaData;
        MetaData m_outgoingMetaData;

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        // Could be a QSet, but well, it's very typical to have only one item in this list.
        QList<Job *> m_subjobs;
        QTimer *m_speedTimer;
        QPointer<QWidget> m_window;

        class JobPrivate;
        JobPrivate *d;
    };

    /**
     * A simple job (one url and one command).
     * This is the base class for all jobs that are scheduled.
     * Other jobs are high-level jobs (CopyJob, DeleteJob, FileCopyJob...)
     * that manage subjobs but aren't scheduled directly.
     */
    class KIO_EXPORT SimpleJob : public KIO::Job {
    Q_OBJECT

    public:
        /**
	 * Creates a new simple job. You don't need to use this constructor,
	 * unless you create a new job that inherits from SimpleJob.
	 * @param url the url of the job
	 * @param command the command of the job
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        SimpleJob(const KUrl& url, int command, const QByteArray &packedArgs,
                  bool showProgressInfo);

        ~SimpleJob();

        void start();

        /**
	 * Returns the SimpleJob's URL
	 * @return the url
	 */
        const KUrl& url() const { return m_url; }

        /**
         * Abort job.
         * This kills all subjobs and deletes the job.
         */
        virtual bool doKill();

        /**
         * Abort job.
         * Suspends slave to be reused by another job for the same request.
         */
        virtual void putOnHold();

        /**
         * Discard suspended slave.
         */
        static void removeOnHold();

        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         **/
        virtual void start( Slave *slave );

        /**
         * @internal
         * Called to detach a slave from a job.
         **/
        void slaveDone();

        /**
         * @internal
         * Slave in use by this job.
         */
        Slave *slave() const { return m_slave; }

        /**
         * @internal
         */
        int command() const { return m_command; }

    public Q_SLOTS:
        /**
         * Forward signal from the slave
         * Can also be called by the parent job, when it knows the size.
	 * @param data_size the total size
         */
        void slotTotalSize( KIO::filesize_t data_size );

    protected Q_SLOTS:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

        /**
         * @internal
         * Called on a slave's warning.
         */
        virtual void slotWarning( const QString & );

        /**
         * Called on a slave's info message.
	 * @param s the info message
	 * @see infoMessage()
         */
        virtual void slotInfoMessage( const QString &s );

        /**
         * Called on a slave's connected signal.
	 * @see connected()
         */
        void slotConnected();

        /**
         * Forward signal from the slave.
	 * @param data_size the processed size in bytes
	 * @see processedSize()
         */
        void slotProcessedSize( KIO::filesize_t data_size );
        /**
         * Forward signal from the slave.
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( unsigned long speed );

        /**
         * MetaData from the slave is received.
	 * @param _metaData the meta data
	 * @see metaData()
         */
        virtual void slotMetaData( const KIO::MetaData &_metaData);

    public Q_SLOTS:
        /**
         * @internal
         * Called on a slave's error.
         * Made public for the scheduler.
         */
        virtual void slotError( int , const QString & );

    protected Q_SLOTS:
        /**
         * @internal
         */
        void slotNeedProgressId();

    protected:
        Slave * m_slave;
        QByteArray m_packedArgs;
        KUrl m_url;
        KUrl m_subUrl;
        int m_command;
        KIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
	/*
	 * Allow jobs that inherit SimpleJob and are aware
	 * of redirections to store the SSL session used.
	 * Retrieval is handled by SimpleJob::start
	 * @param m_redirectionURL Reference to redirection URL,
	 * used instead of m_url if not empty
	 */
	void storeSSLSessionFromJob(const KUrl &m_redirectionURL);
    private:
	class SimpleJobPrivate* d;
    };

    /**
     * A KIO job that retrieves information about a file or directory.
     * @see KIO::stat()
     */
    class KIO_EXPORT StatJob : public SimpleJob {

    Q_OBJECT

    public:
        /**
	 * Do not use this constructor to create a StatJob, use KIO::stat() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        StatJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
	 * A stat() can have two meanings. Either we want to read from this URL,
         * or to check if we can write to it. First case is "source", second is "dest".
         * It is necessary to know what the StatJob is for, to tune the kioslave's behavior
         * (e.g. with FTP).
	 * @param source true for "source" mode, false for "dest" mode
         */
        void setSide( bool source ) { m_bSource = source; }

        /**
         * Selects the level of @p details we want.
         * By default this is 2 (all details wanted, including modification time, size, etc.),
         * setDetails(1) is used when deleting: we don't need all the information if it takes
         * too much time, no need to follow symlinks etc.
         * setDetails(0) is used for very simple probing: we'll only get the answer
         * "it's a file or a directory, or it doesn't exist". This is used by KRun.
	 * @param details 2 for all details, 1 for simple, 0 for very simple
         */
        void setDetails( short int details ) { m_details = details; }

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the result of the stat
         */
        const UDSEntry & statResult() const { return m_statResult; }

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    Q_SIGNALS:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        void slotStatEntry( const KIO::UDSEntry & entry );
        void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotMetaData( const KIO::MetaData &_metaData);

    protected:
        UDSEntry m_statResult;
        KUrl m_redirectionURL;
        bool m_bSource;
        short int m_details;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class StatJobPrivate;
        StatJobPrivate *d;
    };

    /**
     * A KIO job that creates a directory
     * @see KIO::mkdir()
     */
    class KIO_EXPORT MkdirJob : public SimpleJob {

    Q_OBJECT

    public:
        /**
	 * Do not use this constructor to create a MkdirJob, use KIO::mkdir() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        MkdirJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    Q_SIGNALS:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        void slotRedirection( const KUrl &url);
        virtual void slotFinished();

    protected:
        KUrl m_redirectionURL;

    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class MkdirJobPrivate;
        MkdirJobPrivate *d;
    };

    /**
     * @internal
     * Used for direct copy from or to the local filesystem (i.e. SlaveBase::copy())
     */
    class KIO_EXPORT DirectCopyJob : public SimpleJob {
    Q_OBJECT

    public:
        /**
         * Do not create a DirectCopyJob. Use KIO::copy() or KIO::file_copy() instead.
         */
        DirectCopyJob(const KUrl& url, int command, const QByteArray &packedArgs,
                      bool showProgressInfo);
        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

    Q_SIGNALS:
        /**
         * @internal
         * Emitted if the job found an existing partial file
         * and supports resuming. Used by FileCopyJob.
         */
        void canResume( KIO::Job *job, KIO::filesize_t offset );

    private Q_SLOTS:
        void slotCanResume( KIO::filesize_t offset );
    };


    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave ( dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     */
    class KIO_EXPORT TransferJob : public SimpleJob {
    Q_OBJECT

    public:
       /**
	* Do not create a TransferJob. Use KIO::get() or KIO::put()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        TransferJob(const KUrl& url, int command,
                    const QByteArray &packedArgs,
                    const QByteArray &_staticData,
                    bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

        /**
         * Called when m_subJob finishes.
	 * @param job the job that finished
         */
        virtual void slotResult( KJob *job );

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void suspend();

        /**
         * Flow control. Resume data processing from the slave.
         */
        void resume();

        /**
         * Flow control.
	 * @return true if the job is suspended
         */
	bool isSuspended() const { return m_suspended; }


        /**
	 * Checks whether we got an error page. This currently only happens
	 * with HTTP urls. Call this from your slot connected to result().
	 *
         * @return true if we got an (HTML) error page from the server
         * instead of what we asked for.
         */
        bool isErrorPage() const { return m_errorPage; }

        /**
         * Enable the async data mode.
         * When async data is enabled, data should be provided to the job by
         * calling sendAsyncData() instead of returning data in the
         * dataReq() signal.
         */
        void setAsyncDataEnabled(bool enabled);

        /**
         * Provide data to the job when async data is enabled.
         * Should be called exactly once after receiving a dataReq signal
         * Sending an empty block indicates end of data.
         */
        void sendAsyncData(const QByteArray &data);

        /**
         * When enabled, the job reports the amount of data that has been sent,
         * instead of the amount of data that that has been received.
         * @see slotProcessedSize
         * @see slotSpeed
         */
        void setReportDataSent(bool enabled);

        /**
         *  Returns whether the job reports the amount of data that has been
         *  sent (true), or whether the job reports the amount of data that
         * has been received (false)
         */
        bool reportDataSent();

    Q_SIGNALS:
        /**
         * Data from the slave has arrived.
         * @param job the job that emitted this signal
         * @param data data received from the slave.
         *
         * End of data (EOD) has been reached if data.size() == 0, however, you
         * should not be certain of data.size() == 0 ever happening (e.g. in case
         * of an error), so you should rely on result() instead.
         */
        void data( KIO::Job *job, const QByteArray &data );

        /**
         * Request for data.
         * Please note, that you shouldn't put too large chunks
         * of data in it as this requires copies within the frame
         * work, so you should rather split the data you want
         * to pass here in reasonable chunks (about 1MB maximum)
         *
	 * @param job the job that emitted this signal
         * @param data buffer to fill with data to send to the
         * slave. An empty buffer indicates end of data. (EOD)
         */
        void dataReq( KIO::Job *job, QByteArray &data );

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param url the new URL
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

        /**
         * Mimetype determined.
	 * @param job the job that emitted this signal
	 * @param type the mime type
         */
        void mimetype( KIO::Job *job, const QString &type );

        /**
         * @internal
         * Emitted if the "put" job found an existing partial file
         * (in which case offset is the size of that file)
         * and emitted by the "get" job if it supports resuming to
         * the given offset - in this case @p offset is unused)
         */
        void canResume( KIO::Job *job, KIO::filesize_t offset );


    protected Q_SLOTS:
        virtual void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();
        virtual void slotMimetype( const QString &mimetype );
        virtual void slotNeedSubURLData();
        virtual void slotSubURLData(KIO::Job*, const QByteArray &);
        virtual void slotMetaData( const KIO::MetaData &_metaData);
        void slotErrorPage();
        void slotCanResume( KIO::filesize_t offset );
        void slotPostRedirection();

    protected:
        bool m_suspended;
        bool m_errorPage;
        QByteArray staticData;
        KUrl m_redirectionURL;
        KUrl::List m_redirectionList;
        QString m_mimetype;
        TransferJob *m_subJob;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class TransferJobPrivate *d;
    };

    /**
     * StoredTransferJob is a TransferJob (for downloading or uploading data) that
     * also stores a QByteArray with the data, making it simpler to use than the
     * standard TransferJob.
     *
     * For KIO::storedGet it puts the data into the member QByteArray, so the user
     * of this class can get hold of the whole data at once by calling data()
     * when the result signal is emitted.
     * You should only use StoredTransferJob to download data if you cannot
     * process the data by chunks while it's being downloaded, since storing
     * everything in a QByteArray can potentially require a lot of memory.
     *
     * For KIO::storedPut the user of this class simply provides the bytearray from
     * the start, and the job takes care of uploading it.
     * You should only use StoredTransferJob to upload data if you cannot
     * provide the in chunks while it's being uploaded, since storing
     * everything in a QByteArray can potentially require a lot of memory.
     */
    class KIO_EXPORT StoredTransferJob : public KIO::TransferJob {
        Q_OBJECT

    public:
       /**
	* Do not create a StoredTransferJob. Use storedGet() or storedPut()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        StoredTransferJob(const KUrl& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData,
                          bool showProgressInfo);

        /**
         * Set data to be uploaded. This is for put jobs.
         * Automatically called by KIO::storedPut(const QByteArray &, ...),
         * do not call this yourself.
         */
        void setData( const QByteArray& arr );

        /**
         * Get hold of the downloaded data. This is for get jobs.
         * You're supposed to call this only from the slot connected to the result() signal.
         */
        QByteArray data() const { return m_data; }

    private Q_SLOTS:
        void slotStoredData( KIO::Job *job, const QByteArray &data );
        void slotStoredDataReq( KIO::Job *job, QByteArray &data );
    private:
        QByteArray m_data;
        int m_uploadOffset;
    };

    /**
     * The MultiGetJob is a TransferJob that allows you to get
     * several files from a single server. Don't create directly,
     * but use KIO::multi_get() instead.
     * @see KIO::multi_get()
     */
    class KIO_EXPORT MultiGetJob : public TransferJob {
    Q_OBJECT

    public:
        /**
	 * Do not create a MultiGetJob directly, use KIO::multi_get()
	 * instead.
	 *
	 * @param url the first url to get
	 * @param showProgressInfo true to show progress information to the user
	 */
        MultiGetJob(const KUrl& url, bool showProgressInfo);

        virtual ~MultiGetJob();

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
         virtual void start(Slave *slave);

	/**
	 * Get an additional file.
	 *
	 * @param id the id of the file
	 * @param url the url of the file to get
	 * @param metaData the meta data for this request
	 */
        void get(long id, const KUrl &url, const MetaData &metaData);

    Q_SIGNALS:
        /**
         * Data from the slave has arrived.
	 * @param id the id of the request
         * @param data data received from the slave.
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( long id, const QByteArray &data);

        /**
         * Mimetype determined
	 * @param id the id of the request
	 * @param type the mime type
         */
        void mimetype( long id, const QString &type );

        /**
         * File transfer completed.
         *
         * When all files have been processed, result(KJob *) gets
         * emitted.
	 * @param id the id of the request
         */
        void result( long id);

    protected Q_SLOTS:
        virtual void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotMimetype( const QString &mimetype );
    private:
        struct GetRequest {
        public:
           GetRequest(long _id, const KUrl &_url, const MetaData &_metaData)
             : id(_id), url(_url), metaData(_metaData) { }
           long id;
           KUrl url;
           MetaData metaData;
           bool operator==( const GetRequest& req ) const { return req.id == id; }
        };
        bool findCurrentEntry();
        void flushQueue(QLinkedList<GetRequest> &queue);

        typedef QLinkedList<GetRequest> RequestQueue;
        RequestQueue m_waitQueue;
        RequestQueue m_activeQueue;
        bool b_multiGetActive;
        GetRequest m_currentEntry;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MultiGetJobPrivate* d;
    };

    /**
     * A MimetypeJob is a TransferJob that  allows you to get
     * the mime type of an URL. Don't create directly,
     * but use KIO::mimetype() instead.
     * @see KIO::mimetype()
     */
    class KIO_EXPORT MimetypeJob : public TransferJob {
    Q_OBJECT

    public:
       /**
	* Do not create a MimetypeJob directly. Use KIO::mimetype()
	* instead.
	* @param url the url to get
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param showProgressInfo true to show progress information to the user
	*/
        MimetypeJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the mimetype of the URL
         */
         QString mimetype() const { return m_mimetype; }

        /**
	 * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
	 * @param slave the slave that works on the job
         */
        virtual void start( Slave *slave );

    protected Q_SLOTS:
        virtual void slotFinished( );
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MimetypeJobPrivate* d;
    };

    /**
     * The FileCopyJob copies data from one place to another.
     * @see KIO::file_copy()
     * @see KIO::file_move()
     */
    class KIO_EXPORT FileCopyJob : public Job {
    Q_OBJECT

    public:
	/**
	* Do not create a FileCopyJob directly. Use KIO::file_move()
	* or KIO::file_copy() instead.
	* @param src the source URL
	* @param dest the destination URL
	* @param permissions the permissions of the resulting resource
	* @param move true to move, false to copy
	* @param overwrite true to allow overwriting, false otherwise
	* @param resume true to resume an operation, false otherwise
	* @param showProgressInfo true to show progress information to the user
	 */
        FileCopyJob( const KUrl& src, const KUrl& dest, int permissions,
                     bool move, bool overwrite, bool resume, bool showProgressInfo);

        ~FileCopyJob();
        /**
         * If you know the size of the source file, call this method
         * to inform this job. It will be displayed in the "resume" dialog.
	 * @param size the size of the source file
         */
        void setSourceSize(KIO::filesize_t size);

        /**
         * Sets the modification time of the file
         *
         * Note that this is ignored if a direct copy (SlaveBase::copy) can be done,
         * in which case the mtime of the source is applied to the destination (if the protocol
         * supports the concept).
         */
        void setModificationTime( time_t mtime );

	/**
	 * Returns the source URL.
	 * @return the source URL
	 */
        KUrl srcURL() const { return m_src; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KUrl destURL() const { return m_dest; }

    public Q_SLOTS:
        void slotStart();
        void slotData( KIO::Job *, const QByteArray &data);
        void slotDataReq( KIO::Job *, QByteArray &data);

    protected Q_SLOTS:
        /**
         * Called whenever a subjob finishes.
	 * @param job the job that emitted this signal
         */
        virtual void slotResult( KJob *job );

        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the processed size in bytes
         */
        void slotProcessedSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the total size
         */
        void slotTotalSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param pct the percentage
         */
        void slotPercent( KJob *job, unsigned long pct );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param offset the offset to resume from
         */
        void slotCanResume( KIO::Job *job, KIO::filesize_t offset );

    protected:
        void startCopyJob();
        void startCopyJob(const KUrl &slave_url);
        void startRenameJob(const KUrl &slave_url);
        void startDataPump();
        void connectSubjob( SimpleJob * job );

    private:
        void startBestCopyMethod();

    protected:
        KUrl m_src;
        KUrl m_dest;
        int m_permissions;
        bool m_move:1;
        bool m_overwrite:1;
        bool m_resume:1;
        bool m_canResume:1;
        bool m_resumeAnswerSent:1;
        QByteArray m_buffer;
        SimpleJob *m_moveJob;
        SimpleJob *m_copyJob;
        TransferJob *m_getJob;
        TransferJob *m_putJob;
        KIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class FileCopyJobPrivate;
	FileCopyJobPrivate* d;
    };

    /**
     * A ListJob is allows you to get the get the content of a directory.
     * Don't create the job directly, but use KIO::listRecursive() or
     * KIO::listDir() instead.
     * @see KIO::listRecursive()
     * @see KIO::listDir()
     */
    class KIO_EXPORT ListJob : public SimpleJob {
    Q_OBJECT

    public:
       /**
	* Do not create a ListJob directly. Use KIO::listDir() or
	* KIO::listRecursive() instead.
	* @param url the url of the directory
	* @param showProgressInfo true to show progress information to the user
	* @param recursive true to get the data recursively from child directories,
	*        false to get only the content of the specified dir
	* @param prefix the prefix of the files, or QString() for no prefix
	* @param includeHidden true to include hidden files (those starting with '.')
	*/
        ListJob(const KUrl& url, bool showProgressInfo,
                bool recursive = false, const QString &prefix = QString(),
                bool includeHidden = true);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

        /**
         * Returns the ListJob's redirection URL. This will be invalid if there
         * was no redirection.
         * @return the redirection url
         */
        const KUrl& redirectionURL() const { return m_redirectionURL; }

        /**
         * Do not apply any KIOSK restrictions to this job.
         */
        void setUnrestricted(bool unrestricted);

    Q_SIGNALS:
        /**
         * This signal emits the entry found by the job while listing.
         * The progress signals aren't specific to ListJob. It simply
         * uses SimpleJob's processedSize (number of entries listed) and
         * totalSize (total number of entries, if known),
         * as well as percent.
	 * @param job the job that emitted this signal
	 * @param list the list of UDSEntries
         */
        void entries( KIO::Job *job, const KIO::UDSEntryList& list);

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        virtual void slotFinished( );
        virtual void slotMetaData( const KIO::MetaData &_metaData);
        virtual void slotResult( KJob *job );
        void slotListEntries( const KIO::UDSEntryList& list );
        void slotRedirection( const KUrl &url );
        void gotEntries( KIO::Job * subjob, const KIO::UDSEntryList& list );

    private:
        bool recursive;
        bool includeHidden;
        QString prefix;
        unsigned long m_processedEntries;
        KUrl m_redirectionURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class ListJobPrivate* d;
    };

    /// @internal
    struct KIO_EXPORT CopyInfo
    {
        KUrl uSource;
        KUrl uDest;
        QString linkDest; // for symlinks only
        int permissions;
        //mode_t type;
        time_t ctime;
        time_t mtime;
        KIO::filesize_t size; // 0 for dirs
    };

    /**
     * CopyJob is used to move, copy or symlink files and directories.
     * Don't create the job directly, but use KIO::copy(),
     * KIO::move(), KIO::link() and friends.
     *
     * @see KIO::copy()
     * @see KIO::copyAs()
     * @see KIO::move()
     * @see KIO::moveAs()
     * @see KIO::link()
     * @see KIO::linkAs()
     */
    class KIO_EXPORT CopyJob : public Job {
    Q_OBJECT

    public:
	/**
	 * Defines the mode of the operation
	 */
        enum CopyMode{ Copy, Move, Link };

	/**
	 * Do not create a CopyJob directly. Use KIO::copy(),
	 * KIO::move(), KIO::link() and friends instead.
	 *
	 * @param src the list of source URLs
	 * @param dest the destination URL
	 * @param mode specifies whether the job should copy, move or link
	 * @param asMethod if true, behaves like KIO::copyAs(),
	 * KIO::moveAs() or KIO::linkAs()
	 * @param showProgressInfo true to show progress information to the user
	 * @see KIO::copy()
	 * @see KIO::copyAs()
	 * @see KIO::move()
	 * @see KIO::moveAs()
	 * @see KIO::link()
	 * @see KIO::linkAs()
	 */
        CopyJob( const KUrl::List& src, const KUrl& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

        virtual ~CopyJob();

	/**
	 * Returns the list of source URLs.
	 * @return the list of source URLs.
	 */
        KUrl::List srcURLs() const { return m_srcList; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KUrl destURL() const { return m_dest; }

        /**
         * By default the permissions of the copied files will be those of the source files.
         *
         * But when copying "template" files to "new" files, people prefer the umask
         * to apply, rather than the template's permissions.
         * For that case, call setDefaultPermissions(true)
         *
         * TODO KDE4: consider adding this as bool to copy/copyAs?
         */
        void setDefaultPermissions( bool b );

        /**
         * When an error happens while copying/moving a file, the user will be presented with
         * a dialog for skipping the file that can't be copied/moved.
         * Or if the error is that the destination file already exists, the standard
         * rename dialog is shown.
         * If the program doesn't want CopyJob to show dialogs, but to simply fail on error,
         * call setInteractive( false ).
         *
         * KDE4: remove, already in Job
         */
        void setInteractive( bool b );

    Q_SIGNALS:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( KIO::Job *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( KIO::Job *job, unsigned long dirs );

        /**
	 * Emitted when it is known which files / directories are going
	 * to be created. Note that this may still change e.g. when
	 * existing files with the same name are discovered.
	 * @param job the job that emitted this signal
	 * @param files a list of items that are about to be created.
	 */
        void aboutToCreate( KIO::Job *job, const QList<KIO::CopyInfo> &files);

        /**
	 * Sends the number of processed files.
	 * @param job the job that emitted this signal
	 * @param files the number of processed files
	 */
        void processedFiles( KIO::Job *job, unsigned long files );
        /**
	 * Sends the number of processed directories.
	 * @param job the job that emitted this signal
	 * @param dirs the number of processed dirs
	 */
        void processedDirs( KIO::Job *job, unsigned long dirs );

        /**
         * The job is copying a file or directory.
	 * @param job the job that emitted this signal
	 * @param src the URL of the file or directory that is currently
	 *             being copied
	 * @param dest the destination of the current operation
         */
        void copying( KIO::Job *job, const KUrl& src, const KUrl& dest );
        /**
         * The job is creating a symbolic link.
	 * @param job the job that emitted this signal
	 * @param target the URL of the file or directory that is currently
	 *             being linked
	 * @param to the destination of the current operation
         */
        void linking( KIO::Job *job, const QString& target, const KUrl& to );
        /**
         * The job is moving a file or directory.
	 * @param job the job that emitted this signal
	 * @param from the URL of the file or directory that is currently
	 *             being moved
	 * @param to the destination of the current operation
         */
        void moving( KIO::Job *job, const KUrl& from, const KUrl& to );
        /**
         * The job is creating the directory @p dir.
	 * @param job the job that emitted this signal
	 * @param dir the directory that is currently being created
         */
        void creatingDir( KIO::Job *job, const KUrl& dir );
        /**
         * The user chose to rename @p from to @p to.
	 * @param job the job that emitted this signal
	 * @param from the original name
	 * @param to the new name
         */
        void renamed( KIO::Job *job, const KUrl& from, const KUrl& to );

        /**
         * The job emits this signal when copying or moving a file or directory successfully finished.
         * This signal is mainly for the Undo feature.
	 *
	 * @param job the job that emitted this signal
         * @param from the source URL
         * @param to the destination URL
         * @param directory indicates whether a file or directory was successfully copied/moved.
	 *                  true for a directory, false for file
         * @param renamed indicates that the destination URL was created using a
         * rename operation (i.e. fast directory moving). true if is has been renamed
         */
        void copyingDone( KIO::Job *job, const KUrl &from, const KUrl &to, bool directory, bool renamed );
        /**
         * The job is copying or moving a symbolic link, that points to target.
         * The new link is created in @p to. The existing one is/was in @p from.
         * This signal is mainly for the Undo feature.
	 * @param job the job that emitted this signal
         * @param from the source URL
	 * @param target the target
         * @param to the destination URL
         */
        void copyingLinkDone( KIO::Job *job, const KUrl &from, const QString& target, const KUrl& to );

    protected:
        void statCurrentSrc();
        void statNextSrc();

        // Those aren't slots but submethods for slotResult.
        void slotResultStating( KJob * job );
        void startListing( const KUrl & src );
        void slotResultCreatingDirs( KJob * job );
        void slotResultConflictCreatingDirs( KJob * job );
        void createNextDir();
        void slotResultCopyingFiles( KJob * job );
        void slotResultConflictCopyingFiles( KJob * job );
        void copyNextFile();
        void slotResultDeletingDirs( KJob * job );
        void deleteNextDir();
        void skip( const KUrl & sourceURL );
        void slotResultRenaming( KJob * job );
        //void slotResultSettingDirAttributes( KJob * job );
        void setNextDirAttribute();
    private:
        void startRenameJob(const KUrl &slave_url);
        bool shouldOverwrite( const QString& path ) const;
        bool shouldSkip( const QString& path ) const;
        void skipSrc();

    protected Q_SLOTS:
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KJob *job );
        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KJob*, qulonglong data_size );
        /**
         * Forward signal from subjob
	 * @param size the total size
         */
        void slotTotalSize( KJob*, qulonglong size );

        void slotReport();
    private:
        CopyMode m_mode;
        bool m_asMethod;
        enum DestinationState { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST };
        DestinationState destinationState;
        enum { STATE_STATING, STATE_RENAMING, STATE_LISTING, STATE_CREATING_DIRS,
               STATE_CONFLICT_CREATING_DIRS, STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES,
               STATE_DELETING_DIRS, STATE_SETTING_DIR_ATTRIBUTES } state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        QList<CopyInfo> files;
        QList<CopyInfo> dirs;
        KUrl::List dirsToRemove;
        KUrl::List m_srcList;
        KUrl::List::Iterator m_currentStatSrc;
        bool m_bCurrentSrcIsDir;
        bool m_bCurrentOperationIsLink;
        bool m_bSingleFileCopy;
        bool m_bOnlyRenames;
        KUrl m_dest;
        KUrl m_currentDest;
        //
        QStringList m_skipList;
        QStringList m_overwriteList;
        bool m_bAutoSkip;
        bool m_bOverwriteAll;
        int m_conflictError;

        QTimer *m_reportTimer;
        //these both are used for progress dialog reporting
        KUrl m_currentSrcURL;
        KUrl m_currentDestURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class CopyJobPrivate;
        CopyJobPrivate* d;
        friend class CopyJobPrivate; // for DestinationState
    };

    /**
     * A more complex Job to delete files and directories.
     * Don't create the job directly, but use KIO::del() instead.
     *
     * @see KIO::del()
     */
    class KIO_EXPORT DeleteJob : public Job {
    Q_OBJECT

    public:
	/**
	 * Do not create a DeleteJob directly. Use KIO::del()
	 * instead.
	 *
	 * @param src the list of URLs to delete
	 * @param shred true to shred (make sure that data is not recoverable)a
	 * @param showProgressInfo true to show progress information to the user
	 * @see KIO::del()
	 */
        DeleteJob( const KUrl::List& src, bool shred, bool showProgressInfo );

	/**
	 * Returns the list of URLs.
	 * @return the list of URLs.
	 */
        KUrl::List urls() const { return m_srcList; }

    Q_SIGNALS:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( KIO::Job *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( KIO::Job *job, unsigned long dirs );

        /**
	 * Sends the number of processed files.
	 * @param job the job that emitted this signal
	 * @param files the number of processed files
	 */
        void processedFiles( KIO::Job *job, unsigned long files );
        /**
	 * Sends the number of processed directories.
	 * @param job the job that emitted this signal
	 * @param dirs the number of processed dirs
	 */
        void processedDirs( KIO::Job *job, unsigned long dirs );

        /**
	 * Sends the URL of the file that is currently being deleted.
	 * @param job the job that emitted this signal
	 * @param file the URL of the file or directory that is being
	 *        deleted
	 */
        void deleting( KIO::Job *job, const KUrl& file );

    protected Q_SLOTS:
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KJob *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KJob*, qulonglong data_size );
        void slotReport();

    private:
        void statNextSrc();
        void deleteNextFile();
        void deleteNextDir();

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        int m_totalFilesDirs;
        KUrl m_currentURL;
        KUrl::List files;
        KUrl::List symlinks;
        KUrl::List dirs;
        KUrl::List m_srcList;
        KUrl::List::Iterator m_currentStat;
	QStringList m_parentDirs;
        QTimer *m_reportTimer;
    protected:
        /** \internal */
	virtual void virtual_hook( int id, void* data );
    private:
	class DeleteJobPrivate* d;
    };

}

#endif
