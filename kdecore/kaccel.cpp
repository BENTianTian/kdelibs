// Make insertAction return KAccelAction*
// remove 'virtual's
// remove m_bAutoUpdate

#include "kaccel.h"

#include <qaccel.h>
#include <qstring.h>
#include <qwidget.h>
#include <kaccelbase.h>
#include <kdebug.h>
#include <kkeysequence.h>
#include <klocale.h>

//----------------------------------------------------

class KAccelPrivate : public KAccelBase
{
public:
	KAccel* m_pAccel;
	int m_nIDAccelNext;

	KAccelPrivate( KAccel* pParent );

	virtual void setEnabled( bool );

	virtual bool removeAction( const QString& sAction );
	// BCI: make virtual when KAccelBase::setActionSlot has been made virtual
	//bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

	virtual bool emitSignal( KAccelBase::Signal );
	virtual bool connectKey( KAccelAction&, KKeySequence );
	virtual bool disconnectKey( KAccelAction&, KKeySequence );

	void setAutoUpdateTemp( bool b ) { m_bAutoUpdate = b; }
};

KAccelPrivate::KAccelPrivate( KAccel* pParent )
{
	m_pAccel = pParent;
	m_nIDAccelNext = 1;
	m_bAutoUpdate = true;
}

void KAccelPrivate::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
	((QAccel*)m_pAccel)->setEnabled( bEnabled );
}

bool KAccelPrivate::removeAction( const QString& sAction )
{
	KAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		int nID = pAction->getID();
		//bool b = actions().removeAction( sAction );
		bool b = KAccelBase::removeAction( sAction );
		((QAccel*)m_pAccel)->removeItem( nID );
		return b;
	} else
		return false;
}

bool KAccelPrivate::emitSignal( KAccelBase::Signal signal )
{
	if( signal == KAccelBase::KEYCODE_CHANGED ) {
		m_pAccel->emitKeycodeChanged();
		return true;
	}
	return false;
}

bool KAccelPrivate::connectKey( KAccelAction& action, KKeySequence key )
{
	//if( action.m_sName == "file_new" ) { char* c = 0; *c = 0; }
	if( !action.getID() )
		action.setID( m_nIDAccelNext++ );

	uint keyCombQt = (QKeySequence) key;
	int nID = ((QAccel*)m_pAccel)->insertItem( key, action.getID() );
	if( nID != action.getID() )
		action.setID( nID );
	if( nID && action.m_pObjSlot )
		((QAccel*)m_pAccel)->connectItem( nID, action.m_pObjSlot, action.m_psMethodSlot );

	kdDebug(125) << "KAccelPrivate::connectKey( \"" << action.m_sName << "\", " << key.toString() << " = 0x" << QString::number(keyCombQt,16) << " ) = " << nID << " m_pObjSlot = " << action.m_pObjSlot << endl;
	return nID != 0;
}

bool KAccelPrivate::disconnectKey( KAccelAction& action, KKeySequence key )
{
	kdDebug(125) << "KAccelPrivate::disconnectKey( &action = " << &action << " )" << endl;
	kdDebug(125) << "KAccelPrivate::disconnectKey( \"" << action.m_sName << "\", " << key.toString() << " )  m_pObjSlot = " << action.m_pObjSlot << endl;
	if( action.getID() && action.m_pObjSlot )
		return ((QAccel*)m_pAccel)->disconnectItem( action.getID(), action.m_pObjSlot, action.m_psMethodSlot );
	return true;
}

//----------------------------------------------------

KAccel::KAccel( QWidget* pParent, const char* psName )
: QAccel( pParent, psName )
{
	kdDebug(125) << "KAccel(): this = " << this << endl;
	d = new KAccelPrivate( this );
}

KAccel::~KAccel()
{
	kdDebug(125) << "~KAccel(): this = " << this << endl;
	delete d;
}

KAccelBase* KAccel::basePtr()             { return d; }
KAccelActions& KAccel::actions()          { return d->actions(); }
bool KAccel::isEnabled()                  { return d->isEnabled(); }
void KAccel::setEnabled( bool bEnabled )  { d->setEnabled( bEnabled ); }
bool KAccel::setAutoUpdate( bool bAuto )  { return d->setAutoUpdate( bAuto ); }

KAccelAction* KAccel::insertAction( const QString& sAction, const QString& sDesc,
		const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *,
		bool bConfigurable, bool bEnabled )
{
	return d->insertAction( sAction, sDesc, QString::null,
		cutsDef3, cutsDef4,
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable, bEnabled );
}

KAccelAction* KAccel::insertAction( const QString& sAction, const QString& sDesc,
		const QString& cutsDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *,
		bool bConfigurable, bool bEnabled )
{
	KAccelShortcuts cuts( cutsDef );
	return d->insertAction( sAction, sDesc, QString::null,
		cuts, cuts,
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable, bEnabled );
}

KAccelAction* KAccel::insertAction( const QString& sAction, const QString& sDesc,
		KKeySequence cutsDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *,
		bool bConfigurable, bool bEnabled )
{
	KAccelShortcuts cuts( cutsDef );
	return d->insertAction( sAction, sDesc, QString::null,
		cuts, cuts,
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable, bEnabled );
}

KAccelAction* KAccel::insertAction( const char* psAction, const char* psShortcuts,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu*,
		bool bConfigurable, bool bEnabled )
{
	return insertAction( psAction, i18n(psAction),
		KShortcuts(psShortcuts), KShortcuts(psShortcuts),
		pObjSlot, psMethodSlot,
		nIDMenu, 0, bConfigurable, bEnabled );
}

/*bool KAccel::insertAction( const QString& sAction, KShortcuts rgCutDefaults,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *,
		bool bConfigurable, bool bEnabled )
{
	KAccelShortcuts cuts( rgCutDefaults.m_s );
	bool b = d->insertAction( sAction, i18n(sAction.latin1()),
		cuts, cuts,
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable, bEnabled );
	if( m_bAutoUpdate )
		updateConnections();
	return b;
}*/

KAccelAction* KAccel::insertAction( KStdAccel::StdAccel id,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *,
		bool bConfigurable, bool bEnabled )
{
	QString sAction = KStdAccel::action( id );
	if( sAction.isEmpty() )
		return 0;

	KAccelAction* pAction = d->insertAction( sAction, KStdAccel::description( id ), QString::null,
		KStdAccel::defaultShortcuts3( id ), KStdAccel::defaultShortcuts4( id ),
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable, bEnabled );
	if( pAction )
		pAction->setShortcuts( KAccelShortcuts( KStdAccel::key( id ) ) );

	return pAction;
}

bool KAccel::removeAction( const QString& sAction )
	{ return d->removeAction( sAction ); }
bool KAccel::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }

bool KAccel::setActionEnabled( const QString& sAction, bool bEnabled )
{
	return d->setActionEnabled( sAction, bEnabled );
}

bool KAccel::updateConnections()
	{ return d->updateConnections(); }

bool KAccel::setShortcuts( const QString& sAction, const KShortcuts& rgCuts )
{
	kdDebug(125) << "KAccel::setShortcuts()" << endl;
	KAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		if( pAction->m_rgShortcuts != KAccelShortcuts(rgCuts) ) {
			bool bAutoUpdate = d->getAutoUpdate();
			d->setAutoUpdateTemp( true );

			bool b = d->setShortcuts( sAction, rgCuts );

			d->setAutoUpdateTemp( bAutoUpdate );
			return b;
		}
		return true;
	}
	return false;
}

void KAccel::readSettings( KConfig* pConfig )
{
	bool bAutoUpdate = d->getAutoUpdate();
	d->setAutoUpdateTemp( true );

	d->readSettings( pConfig );

	d->setAutoUpdateTemp( bAutoUpdate );
}

void KAccel::writeSettings( KConfig* pConfig ) const
	{ d->writeSettings( pConfig ); }

// for kdegames/ksirtet
void KAccel::setConfigGroup( const QString& s )
	{ d->setConfigGroup( s ); }

void KAccel::emitKeycodeChanged()
{
	kdDebug(125) << "KAccel::emitKeycodeChanged()" << endl;
	emit keycodeChanged();
}

//------------------------------------------------------------
// Obsolete methods -- for backward compatibility
//------------------------------------------------------------

bool KAccel::insertItem( const QString& sDesc, const QString& sAction,
		const char* cutsDef,
		int nIDMenu, QPopupMenu *, bool bConfigurable )
{
	KShortcuts cuts( cutsDef );
	bool b = d->insertAction( sAction, sDesc, QString::null,
		cuts, cuts,
		0, 0,
		nIDMenu, bConfigurable ) != 0;
	return b;
}

bool KAccel::insertItem( const QString& sDesc, const QString& sAction,
		int key,
		int nIDMenu, QPopupMenu*, bool bConfigurable )
{
	KShortcuts cuts( key );
	KAccelAction* pAction = d->insertAction( sAction, sDesc, QString::null,
		cuts, cuts,
		0, 0,
		nIDMenu, bConfigurable );
	return pAction != 0;
}

// Used in kdeutils/kjots
bool KAccel::insertStdItem( KStdAccel::StdAccel id, const QString& sDesc )
{
	insertAction( id, 0, 0 );
	KAccelAction* pAction = d->actionPtr( KStdAccel::action( id ) );
	if( pAction )
		pAction->m_sDesc = sDesc;
	return pAction != 0;
}

bool KAccel::connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate )
{
	kdDebug(125) << "KAccel::connectItem( " << sAction << ", " << pObjSlot << ", " << psMethodSlot << " )" << endl;
	if( bActivate == false )
		d->setActionEnabled( sAction, false );
	bool bAutoUpdate = d->getAutoUpdate();
	d->setAutoUpdateTemp( true );
	bool b = setActionSlot( sAction, pObjSlot, psMethodSlot );
	d->setAutoUpdateTemp( bAutoUpdate );
	if( bActivate == true )
		d->setActionEnabled( sAction, true );
	return b;
}

bool KAccel::removeItem( const QString& sAction )
	{ return d->removeAction( sAction ); }

bool KAccel::setItemEnabled( const QString& sAction, bool bEnable )
	{ return setActionEnabled( sAction, bEnable ); }

#include <qpopupmenu.h>
void KAccel::changeMenuAccel( QPopupMenu *menu, int id, const QString& action )
{
	KAccelAction* pAction = actions().actionPtr( action );
	QString s = menu->text( id );
	if( !pAction || s.isEmpty() )
		return;

	int i = s.find( '\t' );

	QString k = pAction->getShortcut(0).toString();
	if( k.isEmpty() )
		return;

	if ( i >= 0 )
		s.replace( i+1, s.length()-i, k );
	else {
		s += '\t';
		s += k;
	}

	QPixmap *pp = menu->pixmap(id);
	if( pp && !pp->isNull() )
		menu->changeItem( *pp, s, id );
	else
		menu->changeItem( s, id );
}

void KAccel::changeMenuAccel( QPopupMenu *menu, int id, KStdAccel::StdAccel accel )
{
	changeMenuAccel( menu, id, KStdAccel::action( accel ) );
}

int KAccel::stringToKey( const QString& sKey )
{
	return (QKeySequence) KKeySequence( sKey );
}

int KAccel::currentKey( const QString& sAction ) const
{
	KAccelAction* pAction = d->actionPtr( sAction );
	if( pAction ) {
		return pAction->getShortcut(0).getSequence(0).getKey(0).keyQt();
	} else
		return 0;
}

QString KAccel::findKey( int key ) const
{
	KAccelAction* pAction = d->actionPtr( KKeySequence(key) );
	if( pAction )
		return pAction->m_sName;
	else
		return QString::null;
}

QAccel* KAccel::qaccelPtr()
	{ return d->m_pAccel; }

#include <kaccel.moc>
