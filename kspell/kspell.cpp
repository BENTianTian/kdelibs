/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

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

// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h> // atoi

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <qtextcodec.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include "kspell.h"
#include <kwm.h>

#define MAXLINELENGTH 150

enum {
	GOOD=     0,
	IGNORE=   1,
	REPLACE=  2,
	MISTAKE=  3
};

//TODO
//Parse stderr output
//e.g. -- invalid dictionary name

/*
  Things to put in KSpellConfigDlg:
    make root/affix combinations that aren't in the dictionary (-m)
    don't generate any affix/root combinations (-P)
    Report  run-together  words   with   missing blanks as spelling errors.  (-B)
    default dictionary (-d [dictionary])
    personal dictionary (-p [dictionary])
    path to ispell -- NO: ispell should be in $PATH
    */


//  Connects a slot to KProcIO's output signal
#define OUTPUT(x) (connect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))

// Disconnect a slot from...
#define NOOUTPUT(x) (disconnect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))
		


KSpell::KSpell (QWidget *_parent, QString _caption,
		QObject *obj, const char *slot, KSpellConfig *_ksc,
		bool _progressbar, bool _modal)
{
  autoDelete = false;
  modaldlg = _modal;
  progressbar = _progressbar;

  proc=0;
  ksconfig=0;
  ksdlg=0;

  //won't be using the dialog in ksconfig, just the option values
  if (_ksc!=0)
    ksconfig = new KSpellConfig (*_ksc);
  else
    ksconfig = new KSpellConfig;

  codec = 0;
  switch (ksconfig->encoding())
  {
  case KS_E_LATIN1:
     codec = QTextCodec::codecForName("ISO 8859-1");
     break;
  case KS_E_LATIN2:
     codec = QTextCodec::codecForName("ISO 8859-2");
     break;
  default:
     break;
  }

  kdDebug(750) << __FILE__ << ":" << __LINE__ << " Codec = " << (codec ? codec->name() : "<default>") << endl;

  texmode=dlgon=FALSE;
  m_status = Starting;
  dialogsetup = FALSE;
  progres=10;
  curprog=0;

  dialogwillprocess=FALSE;
  dialog3slot="";

  personaldict=FALSE;
  dlgresult=-1;

  caption=_caption;

  parent=_parent;

  trystart=0;
  maxtrystart=2;

  if ( obj && slot )
      // caller wants to know when kspell is ready
      connect (this, SIGNAL (ready(KSpell *)), obj, slot);
  else
      // Hack for modal spell checking
      connect (this, SIGNAL (ready(KSpell *)), this, SLOT( slotModalReady() ) );
  proc=new KProcIO(codec);

  startIspell();
}


void
KSpell::startIspell()
  //trystart = {0,1,2}
{

  kdDebug(750) << "Try #" << trystart << endl;

  if (trystart>0)
    proc->resetAll();

  switch (ksconfig->client())
    {
    case KS_CLIENT_ISPELL:
      proc->setExecutable("ispell");
      kdDebug(750) << "Using ispell" << endl;
      break;
    case KS_CLIENT_ASPELL:
      proc->setExecutable("aspell");
      kdDebug(750) << "Using aspell" << endl;
      break;
    }

  *proc << "-a" << "-S";
  if (ksconfig->noRootAffix())
    {
      *proc<<"-m";
    }
  if (ksconfig->runTogether())
    {
      *proc << "-B";
    }

  if (trystart<2)
    {
      if (! ksconfig->dictionary().isEmpty())
	{
	  kdDebug(750) << "using dictionary [" << ksconfig->dictionary() << "]" << endl;
	  *proc << "-d";
	  *proc << ksconfig->dictionary();
	}
    }

  //Note to potential debuggers:  -Tlatin2 _is_ being added on the
  //  _first_ try.  But, some versions of ispell will fail with this
  // option, so kspell tries again without it.  That's why as 'ps -ax'
  // shows "ispell -a -S ..." withou the "-Tlatin2" option.

  if (trystart<1)
    switch (ksconfig->encoding())
      {
      case KS_E_LATIN1:
	*proc << "-Tlatin1";
	break;
      case KS_E_LATIN2:
	*proc << "-Tlatin2";
	break;
      }


  /*
  if (ksconfig->personalDict()[0]!='\0')
    {
      kdDebug(750) << "personal dictionary [" << ksconfig->personalDict() << "]" << endl;
      *proc << "-p";
      *proc << ksconfig->personalDict();
    }
    */


  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  if (trystart==0) //don't connect these multiple times
    {
      connect (proc, SIGNAL (  receivedStderr (KProcess *, char *, int)),
	       this, SLOT (ispellErrors (KProcess *, char *, int)));


      connect(proc, SIGNAL(processExited(KProcess *)),
	      this, SLOT (ispellExit (KProcess *)));

      OUTPUT(KSpell2);
    }

  if (proc->start ()==FALSE)
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
  }
}

void
KSpell::ispellErrors (KProcess *, char *buffer, int buflen)
{
  buffer [buflen-1] = '\0';
  //  kdebug (KDEBUG_INFO, 750, "ispellErrors [%s]\n",	  buffer);
}

void KSpell::KSpell2 (KProcIO *)

{
  kdDebug(750) << "KSpell::KSpell2" << endl;
  trystart=maxtrystart;  //We've officially started ispell and don't want
       //to try again if it dies.
  QString line;

  if (proc->fgets (line, TRUE)==-1)
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }


  if (line[0]!='@') //@ indicates that ispell is working fine
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize KDE in any text!
  if (ignore ("kde")==FALSE)
  {
     kdDebug(750) << "@KDE was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize linux in any text!
  if (ignore ("linux")==FALSE)
  {
     kdDebug(750) << "@Linux was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  NOOUTPUT (KSpell2);

  m_status = Running;
  emit ready(this);
}

void
KSpell::setUpDialog (bool reallyuseprogressbar)
{
  if (dialogsetup)
    return;

  //Set up the dialog box
  ksdlg=new KSpellDlg (parent, "dialog",
		       progressbar && reallyuseprogressbar, modaldlg );
  ksdlg->setCaption (caption);
  connect (ksdlg, SIGNAL (command (int)), this,
		SLOT (slotStopCancel (int)) );
  connect (this, SIGNAL ( progress (unsigned int) ),
	   ksdlg, SLOT ( slotProgress (unsigned int) ));
  KWM kwm;
  kwm.setMiniIcon (ksdlg->winId(), kapp->miniIcon());

  dialogsetup = TRUE;
}

bool KSpell::addPersonal (QString word)
{
  QString qs (word);

  //we'll let ispell do the work here b/c we can
  qs=qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("&");
  personaldict=TRUE;

  return proc->fputs(qs);
}

bool KSpell::writePersonalDictionary ()
{
  return proc->fputs ("#");
}

bool KSpell::ignore (QString word)
{
  QString qs (word);

  //we'll let ispell do the work here b/c we can
  qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("@");

  return proc->fputs(qs);
}

bool
KSpell::cleanFputsWord (QString s, bool appendCR)
{
  QString qs(s);

  for (unsigned int i=0;i<qs.length();i++)
  {
    //we need some puctuation for ornaments
    if (qs.at(i)!='\'' && qs.at(i)!='\"')
      if (
	  ispunct ((char)(QChar)qs.at(i)) // #### Should use qs[i].isPunct()
	    || qs[i].isSpace())
	  qs.remove(i,1);
  }

  return proc->fputs(qs, appendCR);
}

bool
KSpell::cleanFputs (QString s, bool appendCR)
{
  QString qs(s);
  unsigned int j=0,l=qs.length();

  if (l<MAXLINELENGTH)
    {
      for (unsigned int i=0;i<l;i++,j++)
	{
	  if (//qs.at(i-1)=='\n' &&
	      ispunct ((char)(QChar)qs.at(i)) // #### Should use qs[i].isPunct()
	    && qs.at(i)!='\'' && qs.at(i)!='\"')
	    qs.replace (i,1," ");
	
	}

      if (qs.isEmpty())
	qs="";

      return proc->fputs (qs.ascii(), appendCR);
    }
  else
    return proc->fputs ("\n",appendCR);

}

bool KSpell::checkWord (QString buffer, bool _usedialog)
{
  QString qs (buffer);

  qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  ///set the dialog signal handler
  dialog3slot = SLOT (checkWord3());

  usedialog=_usedialog;
  setUpDialog(FALSE);
  if (_usedialog)
    ksdlg->show();
  else
    ksdlg->hide();

  OUTPUT (checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->fputs ("%"); // turn off terse mode
  proc->fputs (buffer.ascii()); // send the word to ispell

  return TRUE;
}

void KSpell::checkWord2 (KProcIO *)
{
  QString word;

  QString line;

  proc->fgets (line, TRUE); //get ispell's response

  NOOUTPUT(checkWord2);

  int e;
  if ((e=parseOneResponse (line, word, &sugg))==MISTAKE &&
      usedialog)
    {
      cwword=word;
      dialog (word, &sugg, SLOT (checkWord3()));
      return;
    }
      //emits a "corrected" signal _even_ if no change was made
      //so that the calling program knows when the check is complete

  emit corrected (word, word, 0L);
}

void KSpell::checkWord3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  emit corrected (cwword, replacement(), 0L);
}

QString KSpell::funnyWord (QString word)
{
  QString qs;
  unsigned int i=0;

  for (i=0; word [i]!='\0';i++)
    {
      if (word [i]=='+')
	continue;
      if (word [i]=='-')
	{
	  QString shorty;
	  unsigned int j;
	  int k;
	
	  for (j=i+1;word [j]!='\0' && word [j]!='+' &&
		 word [j]!='-';j++)
	    shorty+=word [j];
	  i=j-1;

	  if ((k=qs.findRev (shorty))==0
	//	 || k==(signed)(qs.length()-shorty.length())
		|| k!=-1)
	    qs.remove (k,shorty.length());
	  else
	    {
              qs+='-';
              qs+=shorty;  //it was a hyphen, not a '-' from ispell
            }
	}
      else
	qs+=word [i];
    }
  return qs;
}
	

int KSpell::parseOneResponse (const QString &buffer, QString &word, QStringList *sugg)
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  word = "";
  posinline=0;

  sugg->clear();

  if (buffer [0]=='*')
    {
      return GOOD;
    }

  if (buffer [0]=='&' || buffer [0]=='?' || buffer [0]=='#')
    {
      int i,j;


      QString qs (buffer);
      word = qs.mid (2,qs.find (' ',3)-2);
      //check() needs this
      orig=word;

      /////// Ignore-list stuff //////////
      //We don't take advantage of ispell's ignore function because
      //we can't interrupt ispell's output (when checking a large
      //buffer) to add a word to _it's_ ignore-list.
      QString qword (word);
      if (ignorelist.findIndex(qword.lower())!=-1)
	return IGNORE;

      //// Position in line ///
      QString qs2;

      qs=buffer;
      if (qs.find(':')!=-1)
	qs2=qs.left (qs.find (':'));
      else
	qs2=qs;

      posinline = qs2.right( qs2.length()-qs2.findRev(' ') ).toInt();

      ///// Replace-list stuff ////
      QStringList::Iterator it = replacelist.begin();
      for(;it != replacelist.end(); it++, it++) // Skip two entries at a time.
      {
         if (word == *it) // Word matches
         {
            it++;
            word = *it;   // Replace it with the next entry
            return REPLACE;
	 }
      }

      /////// Suggestions //////
      if (buffer [0]!='#')
	{
	  qs = buffer.mid(buffer.find(':')+2, buffer.length());
	  qs+=',';
	  sugg->clear();
	  i=j=0;
	  while ((unsigned int)i<qs.length())
	    {
	      QString temp = qs.mid (i,(j=qs.find (',',i))-i);
	      sugg->append (funnyWord (temp));
	
	      i=j+2;
	    }
	}

      if ((sugg->count()==1) && ((*sugg)[0] == word))
	return GOOD;

      return MISTAKE;
    }


  kdError(750) << "HERE?: [" << buffer << "]" << endl;
  kdError(750) << "Please report this to dsweet@kde.org" << endl;
  kdError(750) << "Thank you!" << endl;
  emit done((bool)FALSE);
  emit done (KSpell::origbuffer);
  return MISTAKE;
}


bool KSpell::checkList (QStringList *_wordlist)
{
  wordlist=_wordlist;
  if ((totalpos=wordlist->count())==0)
    return FALSE;
  wlIt = wordlist->begin();

  setUpDialog();

  //  ksdlg->show(); //only show if we need it

  //set the dialog signal handler
  dialog3slot = SLOT (checkList4 ());

  proc->fputs ("%"); // turn off terse mode & check one word at a time
  lastpos=0; //now counts which *word number* we are at in checkList3()
  connect (this, SIGNAL (eza()), this, SLOT (checkList2()));
  emit eza();
  OUTPUT(checkList3a);

  return TRUE;
}

void KSpell::checkList2 ()
  //output some words from the list
{
  //  disconnect (this, SIGNAL (eza()), this, SLOT (checkList2()));
  if (wlIt == wordlist->end())
  {
     NOOUTPUT(checkList3a);
     ksdlg->hide();
     emit done(TRUE);
  }
  else
  {
     cleanFputsWord (*wlIt);
     wlIt++;
  }
}

void KSpell::checkList3a (KProcIO *)
{
  connect (this, SIGNAL (ez()), this, SLOT (checkList3()));
  emit ez();
}

void KSpell::checkList3 ()
{
  int e, tempe;

  disconnect (this, SIGNAL (ez()), this, SLOT (checkList3()));


  QString word;
  QString line;

    do
      {
	tempe=proc->fgets (line, TRUE); //get ispell's response
	if (tempe>0)
	  {
	    lastpos++;
	    //kdDebug(750) << "lastpos advance on [" << temp << "]" << endl;
	    if ((e=parseOneResponse (line, word, &sugg))==MISTAKE ||
		e==REPLACE)
	      {
		dlgresult=-1;

		//orig is set by parseOneResponse()
		//		lastpos=newbuffer.find (orig,lastpos,TRUE);

		if (e==REPLACE)
		  {
		    emit corrected (orig, replacement(), lastpos);
		    //  newbuffer.replace (lastpos,orig.length(),word);
		  }
		else
		  {
		    cwword=word;
		    dlgon=TRUE;
		    dialog (word, &sugg, SLOT (checkList4()));
		    return;
		  }
	      }

	  }
      	emitProgress (); //maybe
      } while (tempe>=0);

    if (!dlgon) //is this condition needed?
      emit eza();
}

void KSpell::checkList4 ()
{
  dlgon=FALSE;

  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkList4()));

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      kdDebug(750) << "cklist4: lastpos==(" << lastpos << ")" << endl;
      wordlist->remove (wlIt);
      wordlist->insert (wlIt, replacement());
      wlIt++;
      break;
    case KS_CANCEL:
      ksdlg->hide();
      emit done ((bool)FALSE);
      return;
    case KS_STOP:
      ksdlg->hide();
      emit done (TRUE);
      break;
    };

  emit eza();
}

bool KSpell::check( const QString &_buffer )
{
  QString qs;

  setUpDialog ();
  //set the dialog signal handler
  dialog3slot = SLOT (check3 ());

  kdDebug(750) << "KS: check" << endl;
  origbuffer = _buffer;
  if ( ( totalpos = origbuffer.length() ) == 0 )
    {
      emit done(origbuffer);
      return FALSE;
    }


  // Torben: I corrected the \n\n problem directly in the
  //         origbuffer since I got errors otherwise
  if ( origbuffer.right(2) != "\n\n" )
    {
      if (origbuffer.at(origbuffer.length()-1)!='\n')
	{
	  origbuffer+='\n';
	  origbuffer+='\n'; //shouldn't these be removed at some point?
	}
      else
	origbuffer+='\n';
    }

  newbuffer=origbuffer;

  OUTPUT(check2);
  proc->fputs ("!");

  //lastpos is a position in newbuffer (it has offset in it)
  offset=lastlastline=lastpos=lastline=0;

  emitProgress ();

  int i = origbuffer.find('\n', lastline)+1;
  qs=origbuffer.mid (lastline, i-lastline);
  cleanFputs (qs,FALSE);

  lastline=i; //the character position, not a line number

  ksdlg->show();

  return TRUE;
}

void KSpell::check2 (KProcIO *)
{
  int e, tempe;
  QString word;
  QString line;

  do
    {
      tempe=proc->fgets (line); //get ispell's response
      kdDebug(750) << "2:(" << tempe << ")" << endl;

      if (tempe>0)
	{
	  //kdDebug(750) << "2:[" << temp << "]" << endl;
	
	  if ((e=parseOneResponse (line, word, &sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;
	      lastpos=posinline+lastlastline+offset;
	
	      //orig is set by parseOneResponse()
	
	      if (e==REPLACE)
		{
		  dlgreplacement=word;
		  emit corrected (orig, replacement(), lastpos);
		  offset+=replacement().length()-orig.length();
		  newbuffer.replace (lastpos, orig.length(), word);
		}
	      else  //MISTAKE
		{
		  cwword=word;
		  //kdDebug(750) << "(Before dialog) word=[" << word << "] cwword =[" << cwword << "]\n" << endl;

		  dialog (word, &sugg, SLOT (check3()));
		  return;
		}
	    }
	
	  }

      emitProgress (); //maybe

    } while (tempe>0);

  proc->ackRead();


  if (tempe==-1) //we were called, but no data seems to be ready...
    return;

  //If there is more to check, then send another line to ISpell.
  if ((unsigned int)lastline<origbuffer.length())
    {
      int i;
      QString qs;

      //kdDebug(750) << "[EOL](" << tempe << ")[" << temp << "]" << endl;

      lastpos=(lastlastline=lastline)+offset; //do we really want this?
      i=origbuffer.find('\n', lastline)+1;
      qs=origbuffer.mid (lastline, i-lastline);
      cleanFputs (qs.ascii(),FALSE);
      lastline=i;
      return;
    }
  else
  //This is the end of it all
    //  if (lastline==-1)
    {
      ksdlg->hide();
      //      kdebug (KDEBUG_INFO, 750, "check2() done");
      newbuffer.truncate (newbuffer.length()-2);
      emitProgress();
      emit done (newbuffer);
    }

}

void KSpell::check3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));

  kdDebug(750) << "check3 [" << cwword << "] [" << replacement() << "] " << dlgresult << endl;

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      offset+=replacement().length()-cwword.length();
      newbuffer.replace (lastpos, cwword.length(),
			 replacement().ascii());
      break;
    case KS_CANCEL:
    //      kdebug (KDEBUG_INFO, 750, "cancelled\n");
      ksdlg->hide();
      emit done (origbuffer.ascii());
      return;
    case KS_STOP:
      ksdlg->hide();
      //buffer=newbuffer);
      emitProgress();
      emit done (newbuffer);
      return;
    };

  proc->ackRead();
}

void
KSpell::slotStopCancel (int result)
{
  if (dialogwillprocess)
    return;

  kdDebug(750) << "KSpell::slotStopCancel [" << result << "]" << endl;

  if (result==KS_STOP || result==KS_CANCEL)
    if (!dialog3slot.isEmpty())
      {
	dlgresult=result;
	connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
	emit dialog3();
      }
}


void KSpell::dialog (QString word, QStringList *sugg, const char *_slot)
{
  dlgorigword=word;

  dialog3slot=_slot;
  dialogwillprocess=TRUE;
  connect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  ksdlg->init (word, sugg);
  emit misspelling (word, sugg, lastpos);
  ksdlg->show();
}

void KSpell::dialog2 (int result)
{
  QString qs;

  disconnect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  dialogwillprocess=FALSE;
  dlgresult=result;
  ksdlg->standby();

  dlgreplacement=ksdlg->replacement();

  //process result here
  switch (dlgresult)
    {
    case KS_IGNOREALL:
      ignorelist.prepend(dlgorigword.lower());
      break;
    case KS_ADD:
      addPersonal (dlgorigword);
      personaldict=TRUE;
      ignorelist.prepend(dlgorigword.lower());
      break;
    case KS_REPLACEALL:
      replacelist.append (dlgorigword);
      replacelist.append (replacement());
      break;
    }

  emit corrected (dlgorigword, replacement(), lastpos);
  connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
  emit dialog3();
}


KSpell:: ~KSpell ()
{

  if (proc)
    {
      delete proc;
    }
  if (ksconfig)
    delete ksconfig;

  if (ksdlg)
    delete  ksdlg;
}


KSpellConfig KSpell::ksConfig () const
{
  ksconfig->setIgnoreList(ignorelist);
  return *ksconfig;
}

void KSpell::cleanUp ()
{
  if (m_status == Cleaning) return; // Ignore
  if (m_status == Running)
  {
    if (personaldict)
       writePersonalDictionary();
    m_status = Cleaning;
  }
  proc->closeStdin();
}

void KSpell::ispellExit (KProcess *)
{
  kdDebug(750) << "KSpell::ispellExit()" << endl;
  if ((m_status == Starting) && (trystart<maxtrystart))
  {
    trystart++;
    startIspell();
    return;
  }

  if (m_status == Starting)
     m_status = Error;
  else if (m_status == Cleaning)
     m_status = Finished;
  else if (m_status == Running)
     m_status = Crashed;
  else // Error, Finished, Crashed
     return; // Dead already

  kdError(750) << "Death" << endl;
  QTimer::singleShot( 0, this, SLOT(emitDeath()));
}

// This is always called from the event loop to make
// sure that the receiver can safely delete the
// KSpell object.
void KSpell::emitDeath()
{
  emit death();
  if (autoDelete)
     delete this;
}

void KSpell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void KSpell::emitProgress ()
{
  uint nextprog = (uint) (100.*lastpos/totalpos);

  if (nextprog>=curprog)
    {
      curprog=nextprog;
      emit progress (curprog);
    }
}

void KSpell::moveDlg (int x, int y)
{
  QPoint pt (x,y), pt2;
  pt2=parent->mapToGlobal (pt);
  ksdlg->move (pt2.x(),pt2.y());
}



// --------------------------------------------------
// Stuff for modal (blocking) spell checking
//
// Written by Torben Weis <weis@kde.org>. So please
// send bug reports regarding the modal stuff to me.
// --------------------------------------------------

int
KSpell::modalCheck( QString& text )
{
    modalreturn = 0;
    modaltext = text;

    modalWidgetHack = new QWidget(0,0,WType_Modal);
    modalWidgetHack->setGeometry(-10,-10,2,2);

    // qDebug("KSpell1");
    KSpell* spell = new KSpell( 0L, i18n("Spell Checker"), 0 ,0, 0, TRUE, TRUE );
    // qDebug("KSpell2");
    modalWidgetHack->show();
    qApp->enter_loop();
    // qDebug("KSpell3");

    text = modaltext;
    delete spell;
    return modalreturn;
}

void KSpell::slotModalReady()
{
    // qDebug("MODAL READY");
    ASSERT( m_status == Running );
    connect( this, SIGNAL( done( const QString & ) ), this, SLOT( slotModalDone( const QString & ) ) );
    check( modaltext );
}

void KSpell::slotModalDone( const QString &_buffer )
{
    // qDebug("MODAL DONE %s", _buffer );
    modaltext = _buffer;
    cleanUp();

    modalWidgetHack->close(true);
    modalWidgetHack = 0;
    qApp->exit_loop();
}

QString KSpell::modaltext;
int KSpell::modalreturn = 0;
QWidget* KSpell::modalWidgetHack = 0;


#include "kspell.moc"
