#! /usr/bin/env bash
# Translations
perl cupsdcomment.pl < cupsd.conf.template > cupsd.conf.template.cpp
$XGETTEXT *.cpp -o $podir/cupsdconf4.pot
rm -f cupsd.conf.template.cpp
