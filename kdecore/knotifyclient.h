/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KNOTIFY_CLIENT
#define _KNOTIFY_CLIENT
#include <qobject.h>
#include <kapp.h>
#include <dcopclient.h>

#undef None // X11 headers...

/**
 * This class provides a method for issuing events to a KNotifyServer
 * call KNotifyClient::event("eventname"); to issue it.
 * On installation, there should be a file called 
 * $KDEDIR/share/apps/appname/eventsrc which contains the events.
 *
 * The file looks like this:
 * <pre>
 * [!Global!]
 * appname=executable
 * friendly=FriendlyNameOfApp
 *
 * [eventname]
 * friendly=FriendlyNameOfEvent
 * description=DescriptionOfEvent
 * default_sound=/path/to/sound/file
 * default_presentation=1
 *  ...
 * </pre>
 * This class works, but is still in testing.
 *
 * @author Charles Samuels <charles@altair.dhs.org>
 */
class KNotifyClient : public QObject
{
Q_OBJECT

public:
	enum Presentation
	{
		Default=-1,
		None=0,
		Sound=1,
		Messagebox=2,
		Logwindow=4,
		Logfile=8,
		Stderr=16
	};
	
public:
	/**
	 * The Default constructor.  You should have little use
	 * for this thanks to the @ref event method.
	 * @param message The event type to send, such as "Desktop1" for a virtual
	 *                desktop change
	 * @param text If you need to send off a message with your alert.  This
	 *             will happen for an error level of 2 or more.
	 * @param present How to present it.  If "Default" is chosen, the server
	 *                will decide according to the config
	 * @param client The DCOPClient to use.  Usually it pulls the one from
	 *               your KApplication.
	*/
	KNotifyClient(const QString &message, const QString &text=0,
	             Presentation present=Default,
	             const QString &file=0,
	             DCOPClient* client=0);

	virtual ~KNotifyClient();
	
public slots:
	/**
	 * If you need to send a message a whole lot sequentially, for reasons I 
	 * don't want to know, you might instanciate a KNotifyClient, and call
	 * this function when needed.
	 */
	bool send();

public: //static methods
	/**
	 * This should be the most used method in here.
	 * Call it by KNotifyClient::event("EventName");
	 * It will use KApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only.
	 */
	static bool event(const QString &message, const QString &text=0);
	/**
	 * Will fire an event that's not registered.
	 * @param text The error message text, if applicable
	 * @param present The error message level, one again, defaulting to "Default"
	 * @param file The sound file to play if selected with present
	 */
	static bool userEvent(const QString &text=0, Presentation present=Default,
	                      const QString &file=0);
	
	/**
	 * Gets the presentation associated with a certain event name
	 * Remeber that they may be ORed:
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }	
	 */
	static Presentation getPresentation(const QString &eventname);
	
	/**
	 * Gets the default file associated with a certain event name
	 * The control panel module will list all the event names
	 * This has the potential for being slow.
	 * [Not Yet Implemented!]
	 *
	 * This gets it for this app only!
	 */
	static QString getFile(const QString &eventname, Presentation present);
	
	/**
	 * Gets the default presentation for the event of this program.
	 * Remember that the Presentation may be ORed.  Try this:
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }
	 */
	static Presentation getDefaultPresentation(const QString &eventname);
	
	/**
	 * Gets the default File for the event of this program.
	 * It gets it in relation to present.
	 * Some events don't apply to this function ("Message Box")
	 * Some do (Sound)
	 */
	static QString getDefaultFile(const QString &eventname, Presentation present);

private:
	/**
	 * Why does kdoc include this? This is an internal structure that's actually
	 * declared under "private"
	 * @internal
	 */
	struct Event
	{
		QString message;
		QString text;
		Presentation present;
		QString file;
		DCOPClient *client;
	} *levent;
	
    class KNotifyClientPrivate;
    KNotifyClientPrivate *d;
};

#endif
