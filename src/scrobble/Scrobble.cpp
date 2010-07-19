/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Scrobble.h"
#include "ScrobblePoint.h"
#include <QStringList>
using lastfm::Scrobble;
 

QByteArray
Scrobble::sourceString() const
{
    switch ( source() )
    {
        case LastFmRadio: return "L" + extra( "trackauth" ).toAscii();
        case Player: return "P" + extra( "playerId" ).toUtf8();
        case MediaDevice: return "P" + extra( "mediaDeviceId" ).toUtf8();
        case NonPersonalisedBroadcast: return "R";
        case PersonalisedRecommendation: return "E";
        default: return "U";
    }
}


bool
Scrobble::isValid( Invalidity* v ) const
{
    #define TEST( test, x ) \
        if (test) { \
            if (v) *v = x; \
            return false; \
        }
    
    TEST( duration() < ScrobblePoint::kScrobbleMinLength, TooShort );
    
    // Radio tracks above preview length always scrobble
    if (source() == LastFmRadio)
        return true;

    TEST( !timestamp().isValid(), NoTimestamp );
    
    // actual spam prevention is something like 12 hours, but we are only
    // trying to weed out obviously bad data, server side criteria for
    // "the future" may change, so we should let the server decide, not us
    TEST( timestamp() > QDateTime::currentDateTime().addMonths( 1 ), FromTheFuture );
    
    TEST( timestamp() < QDateTime::fromString( "2003-01-01", Qt::ISODate ), FromTheDistantPast );
    
    // Check if any required fields are empty
    TEST( artist().isNull(), ArtistNameMissing );
    TEST( title().isEmpty(), TrackNameMissing );
    
    TEST( (QStringList() << "unknown artist"
                         << "unknown"
                         << "[unknown]"
                         << "[unknown artist]").contains( artist().name().toLower() ), 
           ArtistInvalid );

    return true;
}
