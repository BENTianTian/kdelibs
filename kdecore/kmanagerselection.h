/****************************************************************************

 $Id$

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef __KMANAGERSELECTION_H
#define __KMANAGERSELECTION_H

#include <qobject.h>
#ifdef Q_WS_X11 // FIXME(E)

#include <X11/Xlib.h>

class KSelectionOwnerPrivate;

// since 3.2
class KSelectionOwner
    : public QObject
    {
    Q_OBJECT
    public:
        KSelectionOwner( Atom selection, int screen_P = -1, QObject* parent = NULL );
        KSelectionOwner( const char* selection, int screen_P = -1, QObject* parent = NULL );
	virtual ~KSelectionOwner();
        bool claim( bool force, bool force_kill = true );
        void release();
	bool filterEvent( XEvent* ev_P ); // internal
    signals:
        void lostOwnership(); // it's NOT safe to delete the instance in a slot
    protected:
        virtual bool handleMessage( XEvent* ev );
        virtual bool genericReply( Atom target, Atom property, Window requestor );
        virtual void replyTargets( Atom property, Window requestor );
        virtual void getAtoms();
        void setData( long extra1, long extra2 );
    private:
        void filter_selection_request( XSelectionRequestEvent& ev_P );
        bool handle_selection( Atom target_P, Atom property_P, Window requestor_P );
        const Atom selection;
        const int screen;
        Window window;
        Time timestamp;
        long extra1, extra2;
        static Atom manager_atom;
        static Atom xa_multiple;
        static Atom xa_targets;
        static Atom xa_timestamp;
    protected:
        virtual void virtual_hook( int id, void* data );
    private:
        KSelectionOwnerPrivate* d;
    };

class KSelectionWatcherPrivate;

// since 3.2
class KSelectionWatcher
    : public QObject
    {
    Q_OBJECT
    public:
        KSelectionWatcher( Atom selection_P, int screen_P = -1, QObject* parent = NULL );
        KSelectionWatcher( const char* selection_P, int screen_P = -1, QObject* parent = NULL );
	virtual ~KSelectionWatcher();
        Window owner();
        void filterEvent( XEvent* ev_P ); // internal
    signals:
        void newOwner( Window owner );
        void lostOwner(); // it's safe to delete the instance in a slot
    private:
        void init();
        const Atom selection;
        const int screen;
        Window selection_owner;
        static Atom manager_atom;
    protected:
        virtual void virtual_hook( int id, void* data );
    private:
        KSelectionWatcherPrivate* d;
    };

#endif
#endif
