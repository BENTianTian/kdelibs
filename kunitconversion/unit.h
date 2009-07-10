/*
 *   Copyright (C) 2007-2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CONVERSION_UNIT_H
#define CONVERSION_UNIT_H

#include <QString>
#include <KLocalizedString>
#include "plasmaconversion_export.h"

namespace Conversion
{

class UnitCategory;

class PLASMACONVERSION_EXPORT Complex
{
public:
    Complex() {};
    virtual ~Complex() {};
    virtual double toDefault(double) const = 0;
    virtual double fromDefault(double) const = 0;
};

class PLASMACONVERSION_EXPORT Unit : public QObject
{
    Q_OBJECT
public:
    Unit(QObject* parent = 0);
    explicit Unit(QObject* parent, int id, double multiplier, const QString& symbol,
                  const QString& description, const QString& match,
                  const KLocalizedString& real, const KLocalizedString& integer);
    explicit Unit(QObject* parent, int id, const Complex* complex, const QString& symbol,
                  const QString& description, const QString& match,
                  const KLocalizedString& real, const KLocalizedString& integer);
    virtual ~Unit();
    /**
     * @return translated name for unit.
     **/
    QString description() const;

    /**
     * @return symbol for the unit.
     **/
    QString symbol() const;

    /**
     * @return singular or plural based on value.
     **/
    QString toString(double value) const;

    /**
     * @return unit multiplier.
     **/
    double multiplier() const;

    /**
     * Set unit multiplier.
     **/
    void setMultiplier(double multiplier);

    /**
     * @return unit category.
     **/
    UnitCategory* category() const;

    /**
     * @return if unit is valid.
     **/
    bool isValid() const;

protected:
    double toDefault(double value) const;
    double fromDefault(double value) const;

private:
    friend class UnitCategory;
    class Private;
    Private* const d;
};

#define U(id, m, s, d, sy, r, i) (new Conversion::Unit(this, id, m, s, d, sy, r, i))

} // Conversion namespace

#endif
