/* -*- C++ -*-

   This file declares the InConstructionState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: InConstructionState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef InConstructionState_H
#define InConstructionState_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "StateImplementation.h"

namespace ThreadWeaver {

    class WeaverImpl;

    /** InConstructionState handles the calls to the WeaverImpl
        object until the constructor has finished.
    */
    class THREADWEAVER_EXPORT InConstructionState : public StateImplementation
    {
    public:
	explicit InConstructionState( WeaverInterface *weaver)
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

        /** reimpl */
        StateId stateId() const;
    };

}

#endif // InConstructionState_H
