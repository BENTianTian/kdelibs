#ifndef __KSCONFIG_H__
#define __KSCONFIG_H__

#include <qwidget.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qstrlist.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <kconfig.h>
#include <klined.h>
#include <qpushbutton.h>

class QLabel;

enum Encoding {
	KS_E_ASCII=0,
	KS_E_LATIN1=1,
	KS_E_LATIN2=2
};

enum KSpellClients {
  KS_CLIENT_ISPELL=0,
  KS_CLIENT_ASPELL=1
};

/**
 *  KSpellConfig v 0.2
 *
 * KSpellConfig is config class for KSpell.  It contains all of the options
 *  settings.  The options are set to default values by the constructor
 *  and can be reset either by using the public interface or by using
 *  KSpellConfig as a widget in a dialog (or, preferably a tabbed dialog
 *  using QTabDialog or KTabCtl) and letting the user change the settings.
 *  This way an application that uses KSpell can either rely on the default
 *  settings (in the simplest case), or offer a dialog to configure
 *  KSpell, or offer a dialog to configure KSpell _for_this_app_only_ 
 *  (in which case, the application should save the settings for use
 *  next time it is run).
 *
 *
 *  @author David Sweet dsweet@wheatcomputers.com
 *  @short Configuration dialog for KSpell
 *  @version $Id$
 *  @see KSpell
 */

class KSpellConfig : public QWidget
{
 Q_OBJECT

public:
  
  /**
   *  Create a KSpellConfig with the default options set if _ksc==0, or
   *   copy the options in _ksc.
   *  This constructor also sets up the dialog.
   */
  KSpellConfig (QWidget *parent=0, const char *name=0,
		KSpellConfig *_ksc=0);

  KSpellConfig (const KSpellConfig &);

  virtual ~KSpellConfig ();

  void operator= (const KSpellConfig &ksc);

  /**
   *Options setting routines.
   * The ignorelist is a QStrList that contains words you'd like KSpell
   *  to ignore when it it spellchecking.  When you get a KSpellConfig
   *  object back from KSpell (using KSpell::kcConfig()), the ignorlist
   *  contains whatever was put in by you plus any words the user has
   *  chosen to ignore via the dialog box.  It might be useful to save
   *  this list with the document being edited to facilitate quicker
   *  future spellchecking.
   */
  void setNoRootAffix (bool);
  void setRunTogether(bool);
  void setDictionary (const QString qs);
  void setDictFromList (bool dfl);
  //  void setPersonalDict (const char *s);
  void setEncoding (int enctype);
  void setIgnoreList (QStrList _ignorelist);
  void setClient (int client);

  /**
   * Options reading routines.
   */
  bool noRootAffix () const;
  bool runTogether() const;
  const QString dictionary () const;
  bool dictFromList () const;
  //QString personalDict () const;
  int encoding () const;
  QStrList ignoreList () const;
  int client () const; //see enums at top of file

  /**
   * Call this method before this class is deleted  if you want the settings
   *  you have (or the user has) chosen to become the global, default settings.
   */
  bool writeGlobalSettings ();


protected:

  // The options
  int enc;			//1 ==> -Tlatin1			  
  bool bnorootaffix;		// -m
  bool bruntogether;		// -B
  bool dictfromlist;				  
  bool nodialog;
  QString qsdict;		// -d [dict]
  QString qspdict;		// -p [dict]
  QStrList ignorelist;
  enum {rdictlist=3, rencoding=4, rhelp=6};
  KConfig *kc;			 
  int iclient;            // defaults to ispell, may be aspell, too

  QCheckBox *cb1, *cb2;
  KLineEdit *kle1; //, *kle2;
  QGridLayout *layout;			  
  QButtonGroup *dictgroup;
  QRadioButton *dictlistbutton, *dicteditbutton;
  QLabel *dictlist;
  QComboBox *dictcombo, *encodingcombo, *clientcombo;
  QPushButton *browsebutton1;

  QStrList *langnames, *langfnames;
  void fillInDialog ();
  bool readGlobalSettings ();

  /**
   * This takes a dictionary file name (fname) and returns
   * a language abbreviation (lname; like de for German), appropriate for the
   * $LANG variable, and a human-readble name (hname; like "Deutsch").
   *
   * It also truncates ".aff" at the end of fname.
   * 
   * TRUE is returned if lname.data()==$LANG
   **/

  bool interpret (QString &fname, QString &lname,
		  QString &hname);
protected slots:
  void sHelp();
//void sBrowseDict();
  //void sBrowsePDict();
  void sNoAff(bool);
  void sRunTogether(bool);
  void sDictionary(bool);
  void sPathDictionary(bool);
  void sSetDictionary (int);
  void sChangeEncoding (int);
  void sChangeClient (int);
  //void textChanged1 (const char *);
  //void textChanged2 (const char *);

};

#endif
