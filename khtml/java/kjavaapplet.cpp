#include "kjavaappletwidget.h"
#include "kjavaappletcontext.h"

#include <kdebug.h>

class KJavaAppletPrivate
{
friend class KJavaApplet;
private:
   bool    reallyExists;
   QString className;
   QString appName;
   QString baseURL;
   QString codeBase;
   QString archives;
   QSize   size;
   QString windowName;

   KJavaAppletWidget* UIwidget;
};


KJavaApplet::KJavaApplet( KJavaAppletWidget* _parent,
                          KJavaAppletContext* _context )
    : params(), liveconnect( new KJavaLiveConnect( _context, this ) )
{
    d = new KJavaAppletPrivate;

    d->UIwidget = _parent;

    if( _context )
        context = _context;
    else
        context = new KJavaAppletContext();

    d->reallyExists = false;
    id = -1;
}

KJavaApplet::~KJavaApplet()
{
    if ( d->reallyExists )
        context->destroy( this );

    delete d;
    delete liveconnect;
}

bool KJavaApplet::isCreated()
{
    return d->reallyExists;
}

void KJavaApplet::setAppletClass( const QString& _className )
{
    d->className = _className;
}

QString& KJavaApplet::appletClass()
{
    return d->className;
}

QString& KJavaApplet::parameter( const QString& name )
{
    return params[ name ];
}

void KJavaApplet::setParameter( const QString& name, const QString& value )
{
    params.insert( name, value );
}

QMap<QString,QString>& KJavaApplet::getParams()
{
    return params;
}

void KJavaApplet::setBaseURL( const QString& baseURL )
{
    d->baseURL = baseURL;
}

QString& KJavaApplet::baseURL()
{
    return d->baseURL;
}

void KJavaApplet::setCodeBase( const QString& codeBase )
{
    d->codeBase = codeBase;
}

QString& KJavaApplet::codeBase()
{
    return d->codeBase;
}

void KJavaApplet::setSize( QSize size )
{
    d->size = size;
}

QSize KJavaApplet::size()
{
    return d->size;
}

void KJavaApplet::setArchives( const QString& _archives )
{
    d->archives = _archives;
}

QString& KJavaApplet::archives()
{
    return d->archives;
}

void KJavaApplet::resizeAppletWidget( int width, int height )
{
    kdDebug(6100) << "KJavaApplet, id = " << id << ", ::resizeAppletWidget to " << width << ", " << height << endl;

    if( d->UIwidget )
        d->UIwidget->resize( width, height );
}

void KJavaApplet::setAppletName( const QString& name )
{
    d->appName = name;
}

void KJavaApplet::setWindowName( const QString& title )
{
    d->windowName = title;
}

QString& KJavaApplet::getWindowName()
{
    return d->windowName;
}

QString& KJavaApplet::appletName()
{
    return d->appName;
}

void KJavaApplet::create( )
{
    context->create( this );
    d->reallyExists = true;
}

void KJavaApplet::init()
{
    context->init( this );
}

void KJavaApplet::start()
{
    context->start( this );
}

void KJavaApplet::stop()
{
    context->stop( this );
}

int KJavaApplet::appletId()
{
    return id;
}

void KJavaApplet::setAppletId( int _id )
{
    id = _id;
}

KJavaLiveConnect::KJavaLiveConnect(KJavaAppletContext* c, KJavaApplet* a) 
    : KParts::LiveConnectExtension(0), context(c), applet(a) {
}

/* TODO: unite JType and KParts::LiveConnectExtension::Type
         make object reference work again
 */

static KParts::LiveConnectExtension::Type convertType(JType type) {
    switch (type) {
        case JBoolean:
            return KParts::LiveConnectExtension::TypeBool;
        case JFunction:
            return KParts::LiveConnectExtension::TypeFunction;
        case JNumber:
            return KParts::LiveConnectExtension::TypeNumber;
        case JObject:
            return KParts::LiveConnectExtension::TypeObject;
        case JString:
            return KParts::LiveConnectExtension::TypeString;
        case JVoid:
        default:
            return KParts::LiveConnectExtension::TypeVoid;
    }
}

bool KJavaLiveConnect::get(const unsigned long /*objid*/, const QString & field, KParts::LiveConnectExtension::Type & type, unsigned long & /*retobjid*/, QString & value )
{
    JType jtype;
    if (!context->getMember(applet, field, jtype, value))
        return false;
    type = convertType(jtype);
    return true;
}

bool KJavaLiveConnect::put(const unsigned long, const QString & name, const QString & value)
{
    return context->putMember(applet, name, value);
}

bool KJavaLiveConnect::call( const unsigned long , const QString & func, const QStringList & args, KParts::LiveConnectExtension::Type & type, unsigned long & /*retobjid*/, QString & value )
{
    JType jtype;
    if (!context->callMember(applet, func, args, jtype, value))
        return false;
    type = convertType(jtype);
    return true;
}

void KJavaLiveConnect::unregister(const unsigned long objid)
{
    context->derefObject(applet, (int) objid);
}

#include "kjavaapplet.moc"
