/**
 * test.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "loader.h"
#include "speller.h"

#include <QCoreApplication>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <QtCore/QDate>

using namespace KSpell2;

int main( int argc, char** argv )
{
    QCoreApplication app(argc,argv);

    Loader::Ptr loader = Loader::openLoader();

    kDebug()<< "Clients are "   << loader->clients()   << endl;
    kDebug()<< "Languages are " << loader->languages() << endl;

    Speller *dict = loader->createSpeller("en_US");

    QStringList words;

    words << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted";

    QTime mtime;
    mtime.start();
    for (QStringList::Iterator itr = words.begin();
         itr != words.end(); ++itr) {
        if (dict && !dict->isCorrect(*itr)) {
            //kDebug()<<"Word " << *itr <<" is misspelled"<<endl;
            QStringList sug = dict->suggest(*itr);
            //kDebug()<<"Suggestions : "<<sug<<endl;
        }
    }
    //mtime.stop();
    kDebug()<<"Elapsed time is "<<mtime.elapsed()<<endl;

    delete dict;

    return 0;
}
