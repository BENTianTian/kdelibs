/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith  <dsmith@algonet.se>

	This class was inspired by a previous KURLCompletion by
	Henner Zeller <zeller@think.de>
	
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KURLCOMPLETION_H
#define KURLCOMPLETION_H

#include <kcompletion.h>
#include <kio/jobclasses.h>
#include <qstring.h>
#include <qstringlist.h>

class KURL;

class KURLCompletionPrivate;

/**
 * This class does completion of URLs including user directories (~user)
 * and environment variables.  Remote URLs are passed to KIO.
 *
 * @short Completion of a single URL
 * @author David Smith <dsmith@algonet.se>
 * @version $Id$
 */
class KURLCompletion : public KCompletion
{
	Q_OBJECT

public:
	/**
	 * Determines how completion is done
	 * ExeCompletion - executables in $PATH or with full path
	 * FileCompletion - all files with full path or in dir(), URLs are listed
	 *                  using KIO
	 */
    enum Mode { ExeCompletion=1, FileCompletion };

	KURLCompletion();
	/**
	 * This overloaded constructor allows you to set the Mode to ExeCompletion
	 * or FileCompletion without using @ref setMode
	 */
	KURLCompletion(Mode);

	virtual ~KURLCompletion();

	/**
	 * Find completions to the given text
	 *
	 * Remote URLs are listed with KIO. For performance reasons, local files
	 * are listed with KIO only if KURLCOMPLETION_LOCAL_KIO is set.
	 * The completion is done asyncronously if KIO is used.
	 *
	 * Returns the first match for user, environment, and local dir completion
	 * and QString::null for asyncronous completion (KIO)
	 */
	virtual QString makeCompletion(const QString&);

	/**
	 * Set the current directory (used as base for completion)
	 * Default = $HOME
	 */
	virtual void setDir( QString dir) { m_dir = dir; };
	
	/**
	 * Get the current directory
	 */
	virtual QString dir() { return m_dir; };

	/**
	 * Returns true if asyncronous completion is in progress
	 */
	virtual bool isRunning() { return m_running; };

	/**
	 * Stop asyncronous copmpletion
	 */
	virtual void stop() { /* not implemented */ } ;

	/**
	 * Return completion mode: exe or file completion (default FileCompletion)
	 */
	virtual Mode mode() { return m_mode; };

	/**
	 * Change completion mode: exe or file completion
	 */
	virtual void setMode( Mode mode ) { m_mode = mode; };

	/**
	 * Replace environment variables when doing completion (default true)
	 */
	virtual bool replaceEnv() { return m_replace_env; };
	
	virtual void setReplaceEnv( bool replace ) { m_replace_env = replace; };

	/**
	 * Replace user home dirs (~user) when doing completion (default true)
	 */
	virtual bool replaceHome() { return m_replace_home; };
	
	virtual void setReplaceHome( bool replace ) { m_replace_home = replace; };

protected:
	// Called by KCompletion, adds quotes
	void postProcessMatch( QString *match );
	void postProcessMatches( QStringList *matches );

protected slots:
	void slotEntries( KIO::Job *, const KIO::UDSEntryList& );
	void slotIOFinished( KIO::Job * );

private:
	// List a directory using readdir()
	void list(QString dir, QString file, QStringList &list, bool only_exe);

	// Expand environment variables and user home dirs in text
	bool expandEnv( QString &text );
	bool expandTilde( QString &text );

	// List the next dir in m_dirs
	QString listDirectories();
	void init();

	QString m_last_path_listed;
	QString m_last_file_listed;
	int m_last_compl_type;
	int m_last_mode;

	QString m_dir; // "current directory" = base dir for completion
	
	Mode m_mode; // ExeCompletion or FileCompletion
	bool m_replace_env;
	bool m_replace_home;

	KIO::ListJob *m_list_job; // kio job to list directories

	bool m_list_exe; // true = only list executables
	bool m_running; // flag set when all dirs have been listed
	QString m_prepend; // text to prepend to listed items
	QString m_compl_text; // text to pass on to KCompletion

	QStringList m_dirs; // dirs to be listed
	QString m_file_filter; // filter for listed files

	KURL *m_current_url; // the url beeing listed by KIO

	// Insert quotes and neseccary escapes
	bool quoteText(QString *text, bool force, bool skip_last);
	
	// Remove quotes/escapes
	QString unquote(const QString &text);
	QString unescape(const QString &text);

	QChar m_word_break_char;
	QChar m_quote_char1;
	QChar m_quote_char2;
	QChar m_escape_char;

	KURLCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
