#ifndef KARCHIVETEST_H
#define KARCHIVETEST_H

#include <QObject>

class KArchiveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateTar();
    void testReadTar();
    void testUncompress();
    void testTarFileData();
    void testTarMaxLength();

    void testCreateZip();
    void testReadZip();
    void testZipFileData();
    void testZipMaxLength();
};


#endif
