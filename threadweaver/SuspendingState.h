/* -*- C++ -*-

   This file declares the SuspendingState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SuspendingState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef SuspendingState_H
#define SuspendingState_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "StateImplementation.h"

namespace ThreadWeaver {

    /** SuspendingState is the state after suspend() has been called, but
        before all threads finished executing the current job and blocked.
    */
    class THREADWEAVER_EXPORT SuspendingState : public StateImplementation
    {
    public:
	explicit SuspendingState( WeaverImpl *weaver)
	    : StateImplementation (weaver)
	    {
	    }
	/** Suspend job processing. */
        virtual void suspend();
        /** Resume job processing. */
        virtual void resume();
        /** Assign a job to an idle thread. */
        virtual Job* applyForWork ( Thread *th,  Job* previous );
        /** Wait (by suspending the calling thread) until a job becomes available. */
        virtual void waitForAvailableJob ( Thread *th );
	/** Overload. */
	void activated();

        /** reimpl */
        StateId stateId() const;
    };

}

#endif // SuspendingState_H
