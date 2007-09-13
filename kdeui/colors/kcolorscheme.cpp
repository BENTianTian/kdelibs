/* This file is part of the KDE project
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kcolorscheme.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kglobalsettings.h>
#include <kcolorutils.h>

#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QWidget>

typedef struct {
    int NormalBackground[3];
    int AlternateBackground[3];
    int NormalText[3];
    int InactiveText[3];
} DefaultColors;

DefaultColors defaultViewColors = {
    { 255, 255, 255 },
    { 241, 241, 239 },
    {   0,   0,   0 },
    { 152, 152, 152 }
};

DefaultColors defaultWindowColors = {
    { 241, 241, 241 },
    { 227, 227, 227 },
    {   0,   0,   0 },
    { 152, 152, 152 }
};

DefaultColors defaultButtonColors = {
    { 218, 221, 215 },
    { 218, 221, 215 },
    {   0,   0,   0 },
    { 152, 154, 149 }
};

DefaultColors defaultSelectionColors = {
    {  60, 131, 208 },
    {  60, 131, 208 },
    { 255, 255, 255 },
    { 177, 202, 232 }
};

DefaultColors defaultTooltipColors = {
    { 255, 248, 209 },
    { 255, 248, 209 },
    {   0,   0,   0 },
    { 232, 185, 149 }
};

// Apply state effects to a background brush
QBrush applyStateEffects(QPalette::ColorGroup state, const QBrush &brush,
                         const KSharedConfigPtr &config)
{
    Q_UNUSED(config); // TODO - actually use the config
    // TODO - eventually, it would be great if this actually worked on brushes
    // FIXME - this code is temporary, proof-of-concept
    switch (state) {
        case QPalette::Inactive:
            return QBrush(KColorUtils::darken(brush.color(), 0.3));
        default:
            return brush;
    }
}

// Apply state effects to a foreground brush
QBrush applyStateEffects(QPalette::ColorGroup state, const QBrush &brush,
                         const QBrush &background,
                         const KSharedConfigPtr &config)
{
    Q_UNUSED(config); // TODO - actually use the config
    // TODO - eventually, it would be great if this actually worked on brushes
    // FIXME - this code is temporary, proof-of-concept
    switch (state) {
        case QPalette::Inactive:
            return QBrush(KColorUtils::darken(brush.color(), 0.3));
        case QPalette::Disabled:
            return QBrush(KColorUtils::mix(brush.color(), background.color(), 0.7));
        default:
            return brush;
    }
}

//BEGIN KColorSchemePrivate
class KColorSchemePrivate : public QSharedData
{
public:
    explicit KColorSchemePrivate(const KSharedConfigPtr&, QPalette::ColorGroup, const char*, DefaultColors);

    QBrush background(KColorScheme::BackgroundRole) const;
    QBrush foreground(KColorScheme::ForegroundRole) const;
    QBrush decoration(KColorScheme::DecorationRole) const;
    qreal contrast() const;
private:
    struct {
        QBrush fg[8], bg[8], deco[2];
    } _brushes;
    qreal _contrast;
};

#define DEFAULT(a) QColor( defaults.a[0], defaults.a[1], defaults.a[2] )

KColorSchemePrivate::KColorSchemePrivate(const KSharedConfigPtr &config,
                                         QPalette::ColorGroup state,
                                         const char *group,
                                         DefaultColors defaults)
{
    KConfigGroup cfg( config, group );
    _contrast = KGlobalSettings::contrastF( config );

    // loaded-from-config colors
    _brushes.fg[0] = cfg.readEntry( "ForegroundNormal", DEFAULT(NormalText) );
    _brushes.fg[1] = cfg.readEntry( "ForegroundInactive", DEFAULT(InactiveText) );
    _brushes.fg[2] = cfg.readEntry( "ForegroundActive", QColor(255,0,0) );
    _brushes.fg[3] = cfg.readEntry( "ForegroundLink", QColor(0,0,255) );
    _brushes.fg[4] = cfg.readEntry( "ForegroundVisited", QColor(88,55,150) );
    _brushes.fg[5] = cfg.readEntry( "ForegroundNegative", QColor(107,0,0) );
    _brushes.fg[6] = cfg.readEntry( "ForegroundNeutral", QColor(0,90,95) );
    _brushes.fg[7] = cfg.readEntry( "ForegroundPositive", QColor(0,95,0) );

    _brushes.bg[0] = cfg.readEntry( "BackgroundNormal", DEFAULT(NormalBackground) );
    _brushes.bg[1] = cfg.readEntry( "BackgroundAlternate", DEFAULT(AlternateBackground) );

    _brushes.deco[0] = cfg.readEntry( "DecorationHover", QColor(72,177,60) );
    _brushes.deco[1] = cfg.readEntry( "DecorationFocus", QColor(239,132,65) );

    // apply state adjustments
    if (state != QPalette::Active) {
        for (int i=0; i<7; i++) {
            _brushes.fg[i] = applyStateEffects(state, _brushes.fg[i], _brushes.bg[0], config);
        }
        _brushes.deco[0] = applyStateEffects(state, _brushes.deco[0], _brushes.bg[0], config);
        _brushes.deco[1] = applyStateEffects(state, _brushes.deco[1], _brushes.bg[0], config);
        _brushes.bg[0] = applyStateEffects(state, _brushes.bg[0], config);
        _brushes.bg[1] = applyStateEffects(state, _brushes.bg[0], config);
    }

    // calculated backgrounds
    _brushes.bg[2] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[2].color() );
    _brushes.bg[3] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[3].color() );
    _brushes.bg[4] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[4].color() );
    _brushes.bg[5] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[5].color() );
    _brushes.bg[6] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[6].color() );
    _brushes.bg[7] = KColorUtils::tint( _brushes.bg[0].color(), _brushes.fg[7].color() );
}

QBrush KColorSchemePrivate::background(KColorScheme::BackgroundRole role) const
{
    switch (role) {
        case KColorScheme::AlternateBackground:
            return _brushes.bg[1];
        case KColorScheme::ActiveBackground:
            return _brushes.bg[2];
        case KColorScheme::LinkBackground:
            return _brushes.bg[3];
        case KColorScheme::VisitedBackground:
            return _brushes.bg[4];
        case KColorScheme::NegativeBackground:
            return _brushes.bg[5];
        case KColorScheme::NeutralBackground:
            return _brushes.bg[6];
        case KColorScheme::PositiveBackground:
            return _brushes.bg[7];
        default:
            return _brushes.bg[0];
    }
}

QBrush KColorSchemePrivate::foreground(KColorScheme::ForegroundRole role) const
{
    switch (role) {
        case KColorScheme::InactiveText:
            return _brushes.fg[1];
        case KColorScheme::ActiveText:
            return _brushes.fg[2];
        case KColorScheme::LinkText:
            return _brushes.fg[3];
        case KColorScheme::VisitedText:
            return _brushes.fg[4];
        case KColorScheme::NegativeText:
            return _brushes.fg[5];
        case KColorScheme::NeutralText:
            return _brushes.fg[6];
        case KColorScheme::PositiveText:
            return _brushes.fg[7];
        default:
            return _brushes.fg[0];
    }
}

QBrush KColorSchemePrivate::decoration(KColorScheme::DecorationRole role) const
{
    switch (role) {
        case KColorScheme::FocusColor:
            return _brushes.deco[1];
        default:
            return _brushes.deco[0];
    }
}

qreal KColorSchemePrivate::contrast() const
{
    return _contrast;
}
//END KColorSchemePrivate

//BEGIN KColorScheme
KColorScheme::KColorScheme(const KColorScheme &other) : d(other.d)
{
}

KColorScheme& KColorScheme::operator=(const KColorScheme& other)
{
    d = other.d;
    return *this;
}

KColorScheme::~KColorScheme()
{
}

KColorScheme::KColorScheme(QPalette::ColorGroup state, ColorSet set, KSharedConfigPtr config)
{
    if (!config) {
        config = KGlobal::config();
    }
    switch (set) {
        case Window:
            d = new KColorSchemePrivate(config, state, "Colors:Window", defaultWindowColors);
            break;
        case Button:
            d = new KColorSchemePrivate(config, state, "Colors:Button", defaultButtonColors);
            break;
        case Selection:
            // inactiver/disabled uses Window colors instead, ala gtk
            if (state == QPalette::Active)
                d = new KColorSchemePrivate(config, state, "Colors:Selection", defaultSelectionColors);
            else
                d = new KColorSchemePrivate(config, state, "Colors:Window", defaultWindowColors);
            break;
        case Tooltip:
            d = new KColorSchemePrivate(config, state, "Colors:Tooltip", defaultTooltipColors);
            break;
        default:
            d = new KColorSchemePrivate(config, state, "Colors:View", defaultViewColors);
    }
}

QBrush KColorScheme::background(BackgroundRole role) const
{
    return d->background(role);
}

QBrush KColorScheme::foreground(ForegroundRole role) const
{
    return d->foreground(role);
}

QBrush KColorScheme::decoration(DecorationRole role) const
{
    return d->decoration(role);
}

QColor KColorScheme::shade(ShadeRole role) const
{
    return shade(background().color(), role, d->contrast());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role)
{
    return shade(color, role, KGlobalSettings::contrastF());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role, qreal contrast, qreal chromaAdjust)
{
    // nan -> 1.0
    contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
    qreal y = KColorUtils::luma(color), yi = 1.0 - y;

    // handle very dark colors (base, mid, dark, shadow == midlight, light)
    if (y < 0.006) {
        switch (role) {
            case KColorScheme::LightShade:
                return KColorUtils::shade(color, 0.05 + 0.95 * contrast, chromaAdjust);
            case KColorScheme::MidShade:
                return KColorUtils::shade(color, 0.01 + 0.20 * contrast, chromaAdjust);
            case KColorScheme::DarkShade:
                return KColorUtils::shade(color, 0.02 + 0.40 * contrast, chromaAdjust);
            default:
                return KColorUtils::shade(color, 0.03 + 0.60 * contrast, chromaAdjust);
        }
    }

    // handle very light colors (base, midlight, light == mid, dark, shadow)
    if (y > 0.93) {
        switch (role) {
            case KColorScheme::MidlightShade:
                return KColorUtils::shade(color, -0.02 - 0.20 * contrast, chromaAdjust);
            case KColorScheme::DarkShade:
                return KColorUtils::shade(color, -0.06 - 0.60 * contrast, chromaAdjust);
            case KColorScheme::ShadowShade:
                return KColorUtils::shade(color, -0.10 - 0.90 * contrast, chromaAdjust);
            default:
                return KColorUtils::shade(color, -0.04 - 0.40 * contrast, chromaAdjust);
        }
    }

    // handle everything else
    qreal lightAmount = (0.05 + y * 0.55) * (0.25 + contrast * 0.75);
    qreal darkAmount =  (     - y       ) * (0.55 + contrast * 0.35);
    switch (role) {
        case KColorScheme::LightShade:
            return KColorUtils::shade(color, lightAmount, chromaAdjust);
        case KColorScheme::MidlightShade:
            return KColorUtils::shade(color, (0.15 + 0.35 * yi) * lightAmount, chromaAdjust);
        case KColorScheme::MidShade:
            return KColorUtils::shade(color, (0.35 + 0.15 * y) * darkAmount, chromaAdjust);
        case KColorScheme::DarkShade:
            return KColorUtils::shade(color, darkAmount, chromaAdjust);
        default:
            return KColorUtils::darken(KColorUtils::shade(color, darkAmount, chromaAdjust), 0.5 + 0.3 * y);
    }
}

void KColorScheme::adjustBackground(QPalette &palette, BackgroundRole newRole, QPalette::ColorRole color,
                                    ColorSet set, KSharedConfigPtr config) {
    palette.setBrush(QPalette::Active,   color, KColorScheme(QPalette::Active,   set, config).background(newRole));
    palette.setBrush(QPalette::Inactive, color, KColorScheme(QPalette::Inactive, set, config).background(newRole));
    palette.setBrush(QPalette::Disabled, color, KColorScheme(QPalette::Disabled, set, config).background(newRole));
}

void KColorScheme::adjustForeground(QPalette &palette, ForegroundRole newRole, QPalette::ColorRole color,
                                    ColorSet set, KSharedConfigPtr config) {
    palette.setBrush(QPalette::Active,   color, KColorScheme(QPalette::Active,   set, config).foreground(newRole));
    palette.setBrush(QPalette::Inactive, color, KColorScheme(QPalette::Inactive, set, config).foreground(newRole));
    palette.setBrush(QPalette::Disabled, color, KColorScheme(QPalette::Disabled, set, config).foreground(newRole));
}
//END KColorScheme

//BEGIN KStatefulBrush
class KStatefulBrushPrivate : public QBrush // for now, just be a QBrush
{
    public:
        KStatefulBrushPrivate() : QBrush() {}
        KStatefulBrushPrivate(const QBrush &brush) : QBrush(brush) {} // not explicit
};

KStatefulBrush::KStatefulBrush()
{
    d = new KStatefulBrushPrivate[3];
}

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::ForegroundRole role,
                               KSharedConfigPtr config)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = KColorScheme(QPalette::Active,   set, config).foreground(role);
    d[1] = KColorScheme(QPalette::Disabled, set, config).foreground(role);
    d[2] = KColorScheme(QPalette::Inactive, set, config).foreground(role);
}

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::BackgroundRole role,
                               KSharedConfigPtr config)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = KColorScheme(QPalette::Active,   set, config).background(role);
    d[1] = KColorScheme(QPalette::Disabled, set, config).background(role);
    d[2] = KColorScheme(QPalette::Inactive, set, config).background(role);
}

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::DecorationRole role,
                               KSharedConfigPtr config)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = KColorScheme(QPalette::Active,   set, config).decoration(role);
    d[1] = KColorScheme(QPalette::Disabled, set, config).decoration(role);
    d[2] = KColorScheme(QPalette::Inactive, set, config).decoration(role);
}

KStatefulBrush::KStatefulBrush(const QBrush &brush, KSharedConfigPtr config)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = brush;
    d[1] = applyStateEffects(QPalette::Disabled, brush, config);
    d[2] = applyStateEffects(QPalette::Inactive, brush, config);
}

KStatefulBrush::KStatefulBrush(const QBrush &brush, const QBrush &background,
                               KSharedConfigPtr config)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = brush;
    d[1] = applyStateEffects(QPalette::Disabled, brush, background, config);
    d[2] = applyStateEffects(QPalette::Inactive, brush, background, config);
}

KStatefulBrush::KStatefulBrush(const KStatefulBrush &other)
{
    d = new KStatefulBrushPrivate[3];
    d[0] = other.d[0];
    d[1] = other.d[1];
    d[2] = other.d[2];
}

KStatefulBrush::~KStatefulBrush()
{
    delete[] d;
}

KStatefulBrush& KStatefulBrush::operator=(const KStatefulBrush &other)
{
    d[0] = other.d[0];
    d[1] = other.d[1];
    d[2] = other.d[2];
    return *this;
}

QBrush KStatefulBrush::brush(QPalette::ColorGroup state) const
{
    switch (state) {
        case QPalette::Inactive:
            return d[2];
        case QPalette::Disabled:
            return d[1];
        default:
            return d[0];
    }
}

QBrush KStatefulBrush::brush(const QPalette &pal) const
{
    return brush(pal.currentColorGroup());
}

QBrush KStatefulBrush::brush(const QWidget *widget) const
{
    if (widget)
        return brush(widget->palette());
    else
        return QBrush();
}
//END KStatefulBrush

// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
