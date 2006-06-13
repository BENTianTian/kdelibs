//
//  DUMMY -- A dummy class with a slot to demonstrate KProcess signals
//
//  version 0.2, Aug 2nd 1997
//
//  Copyright 1997 Christian Czezatke <e9025461@student.tuwien.ac.at>
//


#ifndef DUMMY_H
#define DUMMY_H

#include <stdio.h>
#include <qobject.h>
#include "kprocio.h"

class Dummy : public QObject
{
 Q_OBJECT

 public Q_SLOTS:
   void printMessage(KProcess *proc)
   {
     printf("Process %d exited!\n", (int)proc->pid()); 
   } 
 
   void gotOutput(KProcIO*proc)
   {
    QString line;
    while(true) {
       int result = proc->readln(line);
       if (result == -1) return;
       printf("OUTPUT>> [%d] '%s'\n", result, qPrintable(line));
    }
   }

};

#endif


