/*
   This file is part of the KDE libraries
   Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <QFile>
#include <QDataStream>

#include <kdebug.h>
#include <ktzfiletimezone.h>


/******************************************************************************/

KTzfileTimeZone::KTzfileTimeZone(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude,
        const QString &comment)
  : KTimeZone(source, name, countryCode, latitude, longitude, comment)
//    , d(0)
{
}

KTzfileTimeZone::~KTzfileTimeZone()
{
//    delete d;
}

int KTzfileTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
{
    return offset(toTime_t(utcDateTime));
}

int KTzfileTimeZone::offset(time_t t) const
{
    if (t != (time_t)-1)
    {
        const KTzfileTimeZoneData *tdata = static_cast<const KTzfileTimeZoneData*>(data(true));
        if (tdata)
        {
            const KTzfileTimeZoneData::LocalTimeType *ltt = tdata->getLocalTime(t);
            if (ltt)
                return ltt->gmtoff;
        }
    }
    return 0;
}

int KTzfileTimeZone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
    int offset = 0;
    if (zoneDateTime.isValid()  &&  zoneDateTime.timeSpec() == Qt::LocalTime)
    {
        const KTzfileTimeZoneData *tdata = static_cast<const KTzfileTimeZoneData*>(data(true));
        if (tdata)
        {
            // Get the specified date/time as an offset from epoch (in zone time)
            QDateTime dt = zoneDateTime;
            dt.setTimeSpec(Qt::UTC);
            uint ut = dt.toTime_t();
            time_t tztime = static_cast<time_t>(ut);
            if (ut != (uint)-1  &&  tztime >= 0)
            {
                quint32 count = tdata->nTransitionTimes();
                for (quint32 i = 0;  i < count;  ++i)
                {
                    const KTzfileTimeZoneData::TransitionTime *tt = tdata->transitionTime(i);
                    if (!tt)
                        break;    // how the hell did we reach the end??
                    qint32 t = tt->time;
                    const KTzfileTimeZoneData::LocalTimeType *ltt = tdata->localTimeType(tt->localTimeIndex);
                    if (!ltt)
                        continue;   // something funny is going on here ...
                    int newOffset = ltt->gmtoff;
                    if (tztime - offset < t)
                    {
                        // The date/time falls before this transition
                        if (secondOffset)
                        {
                            if (tztime - newOffset >= t)
                                *secondOffset = newOffset;   // and it falls again after this transition
                            else
                                *secondOffset = offset;
                        }
                        return offset;
                    }
                    offset = newOffset;
                }
            }
        }
    }
    if (secondOffset)
        *secondOffset = offset;
    return offset;
}

bool KTzfileTimeZone::isDstAtUtc(const QDateTime &utcDateTime) const
{
    return isDst(toTime_t(utcDateTime));
}

bool KTzfileTimeZone::isDst(time_t t) const
{
    if (t != (time_t)-1)
    {
        const KTzfileTimeZoneData *tdata = static_cast<const KTzfileTimeZoneData*>(data(true));
        if (tdata)
        {
            const KTzfileTimeZoneData::LocalTimeType *ltt = tdata->getLocalTime(t);
            if (ltt)
                    return ltt->isdst;
        }
    }
    return false;
}

bool KTzfileTimeZone::transitionTime(const QDateTime &utcDateTime, KTzfileTimeZone::Adjustment &adjustment)
{
    if (utcDateTime.timeSpec() != Qt::UTC)
        return false;
    uint ut = utcDateTime.toTime_t();
    time_t t = static_cast<time_t>(ut);
    if (ut != (uint)-1  &&  t >= 0)
    {
        const KTzfileTimeZoneData *tdata = static_cast<const KTzfileTimeZoneData*>(data(true));
        if (tdata)
        {
            const KTzfileTimeZoneData::LocalTimeType *ltt = tdata->getLocalTime(t);
            if (ltt)
            {
                adjustment.utcOffset = ltt->gmtoff;
                adjustment.isDst     = ltt->isdst;
                adjustment.abbrev    = tdata->abbreviation(ltt->abbrIndex);
                return true;
            }
        }
    }
    return false;
}


/******************************************************************************/

class KTzfileTimeZoneDataPrivate
{
public:
    QList<int> utcOffsets;
};


KTzfileTimeZoneData::KTzfileTimeZoneData()
  : m_nTransitionTimes(0),
    m_nLocalTimeTypes(0),
    m_nLeapSecondAdjusts(0),
    m_nIsStandard(0),
    m_nIsUtc(0),
    m_transitionTimes(0),
    m_localTimeTypes(0),
    m_leapSecondAdjusts(0),
    m_isStandard(0),
    m_isUtc(0),
    d(new KTzfileTimeZoneDataPrivate)
{ }

KTzfileTimeZoneData::KTzfileTimeZoneData(const KTzfileTimeZoneData &rhs)
  : KTimeZoneData(),
    m_transitionTimes(0),
    m_localTimeTypes(0),
    m_leapSecondAdjusts(0),
    m_isStandard(0),
    m_isUtc(0),
    d(new KTzfileTimeZoneDataPrivate)
{
    operator=(rhs);
    d->utcOffsets = rhs.d->utcOffsets;
}

KTzfileTimeZoneData::~KTzfileTimeZoneData()
{
    delete[] m_transitionTimes;
    delete[] m_localTimeTypes;
    delete[] m_leapSecondAdjusts;
    delete[] m_isStandard;
    delete[] m_isUtc;
    delete d;
}

KTzfileTimeZoneData &KTzfileTimeZoneData::operator=(const KTzfileTimeZoneData &rhs)
{
    delete[] m_transitionTimes;
    delete[] m_localTimeTypes;
    delete[] m_leapSecondAdjusts;
    delete[] m_isStandard;
    delete[] m_isUtc;
    KTimeZoneData::operator=(rhs);
    m_nTransitionTimes   = rhs.m_nTransitionTimes;
    m_nLocalTimeTypes    = rhs.m_nLocalTimeTypes;
    m_nLeapSecondAdjusts = rhs.m_nLeapSecondAdjusts;
    m_nIsStandard        = rhs.m_nIsStandard;
    m_nIsUtc             = rhs.m_nIsUtc;
    m_abbreviations      = rhs.m_abbreviations;
    d->utcOffsets        = rhs.d->utcOffsets;
    if (m_nTransitionTimes)
    {
        m_transitionTimes = new TransitionTime[m_nTransitionTimes];
        memcpy(m_transitionTimes, rhs.m_transitionTimes, sizeof(TransitionTime) * m_nTransitionTimes);
    }
    else
        m_transitionTimes = 0;
    if (m_nLocalTimeTypes)
    {
        m_localTimeTypes = new LocalTimeType[m_nLocalTimeTypes];
        memcpy(m_localTimeTypes, rhs.m_localTimeTypes, sizeof(LocalTimeType) * m_nLocalTimeTypes);
    }
    else
        m_localTimeTypes = 0;
    if (m_nLeapSecondAdjusts)
    {
        m_leapSecondAdjusts = new LeapSecondAdjust[m_nLeapSecondAdjusts];
        memcpy(m_leapSecondAdjusts, rhs.m_leapSecondAdjusts, sizeof(LeapSecondAdjust) * m_nLeapSecondAdjusts);
    }
    else
        m_leapSecondAdjusts = 0;
    if (m_nIsStandard)
    {
        m_isStandard = new bool[m_nIsStandard];
        memcpy(m_isStandard, rhs.m_isStandard, sizeof(bool) * m_nIsStandard);
    }
    else
        m_isStandard = 0;
    if (m_nIsUtc)
    {
        m_isUtc = new bool[m_nIsUtc];
        memcpy(m_isUtc, rhs.m_isUtc, sizeof(bool) * m_nIsUtc);
    }
    else
        m_isUtc = 0;
    return *this;
}

KTimeZoneData *KTzfileTimeZoneData::clone()
{
    return new KTzfileTimeZoneData(*this);
}

const KTzfileTimeZoneData::TransitionTime *KTzfileTimeZoneData::transitionTime(int index) const
{
    if (index < 0  ||  static_cast<unsigned>(index) > m_nTransitionTimes)
        return 0;
    return m_transitionTimes + index;
}

const KTzfileTimeZoneData::LocalTimeType *KTzfileTimeZoneData::localTimeType(int index) const
{
    if (index < 0  ||  static_cast<unsigned>(index) > m_nLocalTimeTypes)
        return 0;
    return m_localTimeTypes + index;
}

const KTzfileTimeZoneData::LeapSecondAdjust *KTzfileTimeZoneData::leapSecondAdjustment(int index) const
{
    if (index < 0  ||  static_cast<unsigned>(index) > m_nLeapSecondAdjusts)
        return 0;
    return m_leapSecondAdjusts + index;
}

bool KTzfileTimeZoneData::isStandard(int index) const
{
    if (index < 0  ||  static_cast<unsigned>(index) > m_nIsStandard)
        return true;
    return m_isStandard[index];
}

bool KTzfileTimeZoneData::isUtc(int index) const
{
    if (index < 0  ||  static_cast<unsigned>(index) > m_nIsUtc)
        return true;
    return m_isUtc[index];
}

QList<QByteArray> KTzfileTimeZoneData::abbreviations() const
{
    return m_abbreviations;
}

QByteArray KTzfileTimeZoneData::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC)
        return QByteArray();
    uint ut = utcDateTime.toTime_t();
    time_t t = static_cast<time_t>(ut);
    if (ut != (uint)-1  &&  t >= 0)
    {
        const LocalTimeType *ltt = getLocalTime(t);
        if (ltt)
            return abbreviation(ltt->abbrIndex);
    }
    return QByteArray();
}

QList<int> KTzfileTimeZoneData::utcOffsets() const
{
    return d->utcOffsets;
}

QByteArray KTzfileTimeZoneData::abbreviation(int index) const
{
    quint8 i = static_cast<quint8>(index);
    if (index >= 0  &&  static_cast<int>(i) == index  &&  i < m_abbreviations.count())
        return m_abbreviations[i];
    return QByteArray();
}

const KTzfileTimeZoneData::TransitionTime *KTzfileTimeZoneData::getTransitionTime(time_t t) const
{
    quint32 i;
    for (i = 0;  i < m_nTransitionTimes;  ++i)
    {
        if (m_transitionTimes[i].time > t)
            break;
    }
    return (i > 0) ? m_transitionTimes + i - 1 : 0;
}

const KTzfileTimeZoneData::LocalTimeType *KTzfileTimeZoneData::getLocalTime(time_t t) const
{
    const TransitionTime *tt = getTransitionTime(t);
    if (tt)
    {
        unsigned i = tt->localTimeIndex;
        if (i < m_nLocalTimeTypes)
            return m_localTimeTypes + i;
    }
    return 0;
}

quint32 KTzfileTimeZoneData::getLeapSeconds(time_t t) const
{
    quint32 i;
    for (i = 0;  i < m_nLeapSecondAdjusts;  ++i)
    {
        if (m_leapSecondAdjusts[i].time > t)
            break;
    }
    return (i > 0) ? m_leapSecondAdjusts[i - 1].leapSeconds : 0;
}


/******************************************************************************/

class KTzfileTimeZoneSourcePrivate
{
public:
    KTzfileTimeZoneSourcePrivate(const QString &loc)
      : location(loc) {}
    ~KTzfileTimeZoneSourcePrivate() {}

    QString location;
};


KTzfileTimeZoneSource::KTzfileTimeZoneSource(const QString &location)
  : d(new KTzfileTimeZoneSourcePrivate(location))
{
}

KTzfileTimeZoneSource::~KTzfileTimeZoneSource()
{
    delete d;
}

KTimeZoneData* KTzfileTimeZoneSource::parse(const KTimeZone *zone) const
{
    quint32 abbrCharCount;     // the number of characters of time zone abbreviation strings
    quint32 ttisgmtcnt;
    quint8  is;
    quint8  T_, Z_, i_, f_;    // tzfile identifier prefix

    QFile f(d->location + '/' + zone->name());
    if (!f.open(QIODevice::ReadOnly))
    {
        kError() << "Cannot open " << f.fileName() << endl;
        return 0;
    }
    QDataStream str(&f);

    // Read the file type identifier
    str >> T_ >> Z_ >> i_ >> f_;
    if (T_ != 'T' || Z_ != 'Z' || i_ != 'i' || f_ != 'f')
    {
        kError() << "Not a TZFILE: " << f.fileName() << endl;
        return 0;
    }
    // Discard 16 bytes reserved for future use
    unsigned i;
    for (i = 0; i < 4; i++)
        str >> ttisgmtcnt;

    KTzfileTimeZoneData* data = new KTzfileTimeZoneData;

    // Read the sizes of arrays held in the file
    str >> data->m_nIsUtc
        >> data->m_nIsStandard
        >> data->m_nLeapSecondAdjusts
        >> data->m_nTransitionTimes
        >> data->m_nLocalTimeTypes
        >> abbrCharCount;
    // kDebug() << "header: " << data->n_isUtc << ", " << data->n_isStandard << ", " << data->m_leapSecondAdjusts << ", " <<
    //    data->n_transitionTimes << ", " << data->n_localTimeTypes << ", " << abbrCharCount << endl;

    // Read the transition times, at which the rules for computing local time change
    data->m_transitionTimes = new KTzfileTimeZoneData::TransitionTime[data->m_nTransitionTimes];
    KTzfileTimeZoneData::TransitionTime *tt = data->m_transitionTimes;
    for (i = 0;  i < data->m_nTransitionTimes;  ++tt, ++i)
    {
        str >> tt->time;
    }
    tt = data->m_transitionTimes;
    for (i = 0; i < data->m_nTransitionTimes; ++tt, ++i)
    {
        // NB: these appear to be 1-based, not zero-based!
        str >> tt->localTimeIndex;
    }

    // Read the local time types
    data->m_localTimeTypes = new KTzfileTimeZoneData::LocalTimeType[data->m_nLocalTimeTypes];
    KTzfileTimeZoneData::LocalTimeType *ltt = data->m_localTimeTypes;
    for (i = 0;  i < data->m_nLocalTimeTypes;  ++ltt, ++i)
    {
        str >> ltt->gmtoff;
        str >> is;
        ltt->isdst = (is != 0);
        str >> ltt->abbrIndex;
        // Add the UTC offset to the complete list of UTC offsets
        if (data->d->utcOffsets.indexOf(ltt->gmtoff) < 0)
            data->d->utcOffsets.append(ltt->gmtoff);
        // kDebug() << "local type: " << ltt->gmtoff << ", " << is << ", " << ltt->abbrIndex << endl;
    }
    qSort(data->d->utcOffsets);

    // Read the timezone abbreviations. They are stored as null terminated strings in
    // a character array.
    // Make sure we don't fall foul of maliciously coded time zone abbreviations.
    if (abbrCharCount > 64)
    {
        kError() << "excessive length for timezone abbreviations: " << abbrCharCount << endl;
        delete data;
        return 0;
    }
    QByteArray array(abbrCharCount, 0);
    str.readRawData(array.data(), array.size());
    char *abbrs = array.data();
    if (abbrs[abbrCharCount - 1] != 0)
    {
        // These abbrevations are corrupt!
        kError() << "timezone abbreviations not null terminated: " << abbrs[abbrCharCount - 1] << endl;
        delete data;
        return 0;
    }
    quint8 n = 0;
    for (i = 0; i < abbrCharCount; ++n, i += strlen(abbrs + i) + 1)
    {
        data->m_abbreviations.append(QByteArray(abbrs + i));
        // Convert the LocalTimeTypes pointer to a sequential index
        ltt = data->m_localTimeTypes;
        for (unsigned j = 0;  j < data->m_nLocalTimeTypes;  ++ltt, ++j)
        {
            if (ltt->abbrIndex == i)
                ltt->abbrIndex = n;
        }
    }

    // Read the leap second adjustments
    data->m_leapSecondAdjusts = new KTzfileTimeZoneData::LeapSecondAdjust[data->m_nLeapSecondAdjusts];
    KTzfileTimeZoneData::LeapSecondAdjust* lsa = data->m_leapSecondAdjusts;
    for (i = 0;  i < data->m_nLeapSecondAdjusts;  ++lsa, ++i)
    {
        str >> lsa->time >> lsa->leapSeconds;
        // kDebug() << "leap entry: " << lsa->time << ", " << lsa->leapSeconds << endl;
    }

    // Read the standard/wall time indicators.
    // These are true if the transition times associated with local time types
    // are specified as standard time, false if wall clock time.
    data->m_isStandard = new bool[data->m_nIsStandard];
    bool *iss = data->m_isStandard;
    for (i = 0;  i < data->m_nIsStandard;  ++iss, ++i)
    {
        str >> is;
        *iss = (is != 0);
        // kDebug() << "standard: " << is << endl;
    }

    // Read the UTC/local time indicators.
    // These are true if the transition times associated with local time types
    // are specified as UTC, false if local time.
    data->m_isUtc = new bool[data->m_nIsUtc];
    bool *isu = data->m_isUtc;
    for (i = 0;  i < data->m_nIsUtc;  ++isu, ++i)
    {
        str >> is;
        *isu = (is != 0);
        // kDebug() << "UTC: " << is << endl;
    }
    return data;
}
