/* This is a comment */

#ifndef TEST_H
#define TEST_H

// some comment
#include <dcopobject.h>
#include <dcopref.h>

/* We never use it, but it's a test after all :)
 */
class Test2;

// this has to work too
#include <qstring.h>

class Test3;

class Test : public MyNamespace::MyParentClass, virtual public DCOPObject
{
    K_DCOP

public:
    Test();
    ~Test();
    int getPageOfObj( int obj );
    void setCell( KSpreadTable *table,
		  const QPoint& point );

k_dcop:
    virtual QString url();
    virtual DCOPRef firstView();
    virtual DCOPRef nextView();
    virtual int getNumObjects();
    virtual DCOPRef getObject( int num );
    virtual int getNumPages();
    virtual DCOPRef getPage( int num );
    int getPageOfObj( int obj );
    void setCell( const int& point = 3 );

private:
    Test3 *doc();

};

#endif // end

/* Test for line numbers */


