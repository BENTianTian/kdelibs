/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef _KCARDIMPLEMENTATION_H
#define _KCARDIMPLEMENTATION_H

#include <qstring.h>
#include <kpcsc.h>


#define KCARD_TYPE_UNKNOWN   "Unknown"
#define KCARD_TYPE_PROCESSOR "Processor"
#define KCARD_TYPE_GSM       "GSM"
#define KCARD_TYPE_MEMORY    "Memory"
#define KCARD_TYPE_JAVA      "Java"


class KCardReader;

class KCardImplementation {


 public:

  

  int init(const QString &);
  virtual int selectFile(const QString);
  virtual int selectDirectory(const QString);
  virtual KCardCommand getCardSerialNumber();

  virtual const QString& getType() const;
  virtual const QString& getSubType() const;
  virtual const QString& getSubSubType() const;

  /* Attempt to match the ATR.  This uses a heuristic to determine
   * if the given ATR is for a card with this type/subtype/subsubtype.
   * It should return 0 or -1 for a failure, and a number up to 100 for
   * the percentage of probability of successful match.
   */
  virtual int matchATR(KCardCommand atr);

  virtual int lastError() const;
  virtual void clearError();

  virtual QString lastErrorMessage () const {return QString::null;};
 private:

  class KCardImplementationPrivate;
  KCardImplementationPrivate *d;

  KPCSC * pcscInt;

 protected:

  KCardImplementation (const QString & type=KCARD_TYPE_UNKNOWN, 
		       const QString & subType=KCARD_TYPE_UNKNOWN,
		       const QString & subSubType=KCARD_TYPE_UNKNOWN);

  virtual ~KCardImplementation();

  KCardReader * _kcardreader;
  QString _type, _subType, _subSubType;
  int _errno;
};

#endif
