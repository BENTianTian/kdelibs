/* This file is part of the KDE libraries

   Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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

#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kapp.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kcmenumngr.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "keditcl.h"
#include "keditcl.moc"

class KEdit::KEditPrivate
{
public:
    bool overwriteEnabled;
};


KEdit::KEdit(QWidget *_parent, const char *name)
   : QMultiLineEdit(_parent, name)
{
    d = new KEditPrivate;
    d->overwriteEnabled = false;

    parent = _parent;

    // fancy optimized refreshing (Matthias and Paul)
    repaintTimer = new QTimer(this);
    connect(repaintTimer, SIGNAL(timeout()), this, SLOT(repaintAll()));

    // set some defaults

    line_pos = col_pos = 0;

    installEventFilter( this );

    srchdialog = NULL;
    replace_dialog= NULL;
    gotodialog = NULL;

    setAcceptDrops(true);
    KCursor::setAutoHideCursor( this, true );
}


KEdit::~KEdit()
{
  delete d;
}

void
KEdit::insertText(QTextStream *stream)
{
   setAutoUpdate(FALSE);
   int line, col;
   getCursorPosition(&line, &col);
   int saveline = line;
   int savecol = col;
   QString textLine;
   // WABA/Bernd: Reading/inserting it in line by line kills performance
   // with large files because this will take O(n^2) time.
   textLine = stream->read(); // Read all!
   // We might want to read it in slightly smaller chunks since
   // now worst case memory consumption seems to be 4x file size.
   // 2x for ASCII --> UNICODE, 1 copy in "textLine", 1 copy in the
   // widget itself.
   // If we can read it in, let's say, blocks of 64Kb, we only need
   // 64Kb + 2x file size. 2x for ASCII --> UNICODE, 64 Kb in textline,
   // 1 copy in widget itself.
   insertAt( textLine, line, col);
   setCursorPosition(saveline, savecol);
   setAutoUpdate(true);

   if (!repaintTimer->isActive())
      repaintTimer->start(0,TRUE);
   repaint();

   setModified(true);
   setFocus();

   // Bernd: Please don't leave debug message like that lying around
   // they cause ENORMOUSE performance hits. Once upon a day
   // kedit used to be really really fast using memmap etc .....
   // oh well ....

   //   QString str = text();
   //   for (int i = 0; i < (int) str.length(); i++)
   //     printf("KEdit: U+%04X\n", str[i].unicode());

}

void
KEdit::cleanWhiteSpace()
{
   if (!hasMarkedText()) return;
   QString oldText = markedText();
   QString newText;
   QStringList lines = QStringList::split('\n', oldText);
   bool addSpace = false;
   bool firstLine = true;
   QChar lastChar = oldText[oldText.length()-1];
   QChar firstChar = oldText[0];
   for(QStringList::Iterator it = lines.begin();
       it != lines.end();)
   {
      QString line = (*it).simplifyWhiteSpace();
      if (line.isEmpty())
      {
         if (addSpace)
            newText += QString::fromLatin1("\n\n");
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += '\n';
            firstLine = false;
         }
         addSpace = false;
      }
      else
      {
         if (addSpace)
            newText += ' ';
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += ' ';
            firstLine = false;
         }
         newText += line;
         addSpace = true;
      }
      it = lines.remove(it);
   }
   if (addSpace)
   {
      if (lastChar == '\n')
         newText += '\n';
      else if (lastChar.isSpace())
         newText += ' ';
   }

   if (oldText == newText)
   {
      deselect();
      return;
   }

   setAutoUpdate(FALSE);
   del();
   insert(newText);
   setAutoUpdate(TRUE);
   if (!repaintTimer->isActive())
      repaintTimer->start(0,TRUE);
   repaint();

   setModified(true);
   setFocus();
}

void
KEdit::saveText(QTextStream *stream)
{
   int line_count = numLines();
   for(int i = 0; i < line_count; i++)
   {
      (*stream) << textLine(i) << '\n';
   }
}

void KEdit::repaintAll(){
    repaint(FALSE);
}

int KEdit::currentLine(){

  computePosition();
  return line_pos;

};

int KEdit::currentColumn(){

  computePosition();
  return col_pos;

}

void KEdit::computePosition(){

  int line, col, coltemp;

  getCursorPosition(&line,&col);
  QString linetext = textLine(line);

  // O.K here is the deal: The function getCursorPositoin returns the character
  // position of the cursor, not the screenposition. I.e,. assume the line
  // consists of ab\tc then the character c will be on the screen on position 8
  // whereas getCursorPosition will return 3 if the cursors is on the character c.
  // Therefore we need to compute the screen position from the character position.
  // That's what all the following trouble is all about:

  coltemp  = 	col;
  int pos  = 	0;
  int find = 	0;
  int mem  = 	0;
  bool found_one = false;

  // if you understand the following algorithm you are worthy to look at the
  // kedit+ sources -- if not, go away ;-)

  while(find >=0 && find <= coltemp- 1 ){
    find = linetext.find('\t', find, TRUE );
    if( find >=0 && find <= coltemp - 1 ){
      found_one = true;
      pos = pos + find - mem;
      pos = pos + 8  - pos % 8;
      mem = find;
      find ++;
    }
  }

  pos = pos + coltemp - mem ;  // add the number of characters behind the
                               // last tab on the line.

  if (found_one){
    pos = pos - 1;
  }

  line_pos = line;
  col_pos = pos;

}


void KEdit::keyPressEvent ( QKeyEvent *e){


  if ((e->state() & ControlButton ) && (e->key() == Key_K) ){

    int line = 0;
    int col  = 0;
    QString killstring;

    if(!killing){
      killbufferstring = "";
      killtrue = false;
      lastwasanewline = false;
    }

    getCursorPosition(&line,&col);
    killstring = textLine(line);
    killstring = killstring.mid(col,killstring.length());


    if(!killbufferstring.isEmpty() && !killtrue && !lastwasanewline){
      killbufferstring += '\n';
    }

    if( (killstring.length() == 0) && !killtrue){
      killbufferstring += '\n';
      lastwasanewline = true;
    }

    if(killstring.length() > 0){

      killbufferstring += killstring;
      lastwasanewline = false;
      killtrue = true;

    }else{

      lastwasanewline = false;
      killtrue = !killtrue;

    }

    killing = true;

    QMultiLineEdit::keyPressEvent(e);
    setModified();
    emit CursorPositionChanged();
    return;
  }

  if ((e->state() & ControlButton ) && (e->key() == Key_Y) ){

    int line = 0;
    int col  = 0;

    getCursorPosition(&line,&col);

    if(!killtrue)
      killbufferstring += '\n';

    insertAt(killbufferstring,line,col);

    killing = false;
    setModified();
    emit CursorPositionChanged();
    return;
  }

  killing = false;

  if ((e->state() & ControlButton ) && (e->key() == Key_Insert) ){
    copy();
    return;
  }

  if ((e->state() & ShiftButton ) && (e->key() == Key_Insert) ){
    paste();
	setModified();
	emit CursorPositionChanged();
	return;
    }

  if ((e->state() & ShiftButton ) && (e->key() == Key_Delete) ){
    cut();
	setModified();
	emit CursorPositionChanged();
	return;
    }

  if (d->overwriteEnabled)
  {
    if (e->key() == Key_Insert){
       this->setOverwriteMode(!this->isOverwriteMode());
       emit toggle_overwrite_signal();
       return;
    }
  }

  QMultiLineEdit::keyPressEvent(e);
  emit CursorPositionChanged();

}


void KEdit::mousePressEvent (QMouseEvent* e){


  QMultiLineEdit::mousePressEvent(e);
  emit CursorPositionChanged();

}

void KEdit::mouseMoveEvent (QMouseEvent* e){

  QMultiLineEdit::mouseMoveEvent(e);
  emit CursorPositionChanged();


}


void KEdit::installRBPopup(QPopupMenu *p) {
  KContextMenuManager::insert( this, p );
}

void KEdit::mouseReleaseEvent (QMouseEvent* e){


  QMultiLineEdit::mouseReleaseEvent(e);
  emit CursorPositionChanged();

}

void KEdit::selectFont(){

  QFont font = this->font();
  KFontDialog::getFont(font);
  this->setFont(font);

}

void KEdit::setModified(bool _mod){
  setEdited(_mod);
}

bool KEdit::isModified(){
    return edited();
}

bool KEdit::eventFilter(QObject *, QEvent *ev){

  if (ev->type() == QEvent::Paint)
	{
	if (srchdialog)
		if (srchdialog->isVisible())
			srchdialog->raise();

	if (replace_dialog)
		if (replace_dialog->isVisible())
			replace_dialog->raise();
	}

  return FALSE;

}


QString KEdit::markedText(){
  return QMultiLineEdit::markedText();
}

void KEdit::doGotoLine() {

	if( !gotodialog )
		gotodialog = new KEdGotoLine( parent, "gotodialog" );

	this->clearFocus();

	gotodialog->show();
	// this seems to be not necessary
	// gotodialog->setFocus();
	if( gotodialog->result() ) {
		setCursorPosition( gotodialog->getLineNumber()-1 , 0, FALSE );
		emit CursorPositionChanged();
		setFocus();
	}
}


void  KEdit::dragMoveEvent(QDragMoveEvent* e) {

  if(QUriDrag::canDecode(e))
    e->accept();
  else if(QTextDrag::canDecode(e))
    QMultiLineEdit::dragMoveEvent(e);
}


void  KEdit::dragEnterEvent(QDragEnterEvent* e) {

  kdDebug() << "KEdit::dragEnterEvent()" << endl;
  e->accept(QUriDrag::canDecode(e) || QTextDrag::canDecode(e));
}


void  KEdit::dropEvent(QDropEvent* e) {

  kdDebug() << "KEdit::dropEvent()" << endl;

  if(QUriDrag::canDecode(e)) {
   emit gotUrlDrop(e);
  }
  else if(QTextDrag::canDecode(e))
    QMultiLineEdit::dropEvent(e);
}

void KEdit::setOverwriteEnabled(bool b)
{
  d->overwriteEnabled = b;
}

