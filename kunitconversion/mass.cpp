/*
 * Copyright (C) 2008 Petri Damstén <damu@iki.fi>
 * Copyright (C) 2008 Kristof Bal <kristof.bal@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mass.h"
#include <KLocale>

Mass::Mass(QObject* parent)
: Conversion::UnitCategory(parent)
{
    setObjectName("mass");
    setName(i18n("Mass"));
    setDefaultUnit("g");
    addSIUnit(defaultUnit(), i18nc("mass unit", "gram"), i18n("grams"));

    U(i18nc("mass unit", "ton"), i18nc("mass unit", "tons"), "t", 1000000.0,
        << i18nc("mass unit", "tonne"));
    //I guess it's useful...
    U(i18nc("mass unit", "carat"), i18nc("mass unit", "carats"), "CD", 0.2, );
    //http://en.wikipedia.org/wiki/Pound_(mass)#International_pound
    U(i18nc("mass unit", "pound"), i18nc("mass unit", "pounds"),
        i18nc("mass unit: pounds", "lb"), 453.592, );
    //International avoirdupois ounce
    U(i18nc("mass unit", "ounce"), i18nc("mass unit", "ounces"), "oz", 28.349523125, );
    U(i18nc("mass unit", "troy ounce"), i18nc("mass unit", "troy ounces"), "t oz", 31.1034768, );
    U(i18nc("mass unit", "newton"), i18nc("mass unit", "newtons"), "N", 1000.0/9.81, ); //on earth
	//used a lot in industry (aircraft engines for example)
    U(i18nc("mass unit", "kilonewton"), i18nc("mass unit", "kilonewton"), "kN", 1000000.0/9.81, );
}
