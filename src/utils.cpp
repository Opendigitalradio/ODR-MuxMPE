/*
   Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012 Her Majesty the Queen in Right of Canada (Communications
   Research Center Canada)

   Copyright (C) 2021 Matthias P. Braendli, matthias.braendli@mpb.li
   Copyright (C) 2023 Andy Mace, andy.mace@mediauk.net

    http://www.opendigitalradio.org
*/
/*
   This file is part of ODR-MuxMPE.

   ODR-MuxMPE is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   ODR-MuxMPE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ODR-MuxMPE.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>
#include <climits>
#include <iostream>
#include <memory>
#include <boost/algorithm/string/join.hpp>
#include "utils.h"

using namespace std;

static time_t dab_time_seconds = 0;
static int dab_time_millis = 0;


void update_dab_time()
{
    if (dab_time_seconds == 0) {
        dab_time_seconds = time(nullptr);
    } else {
        dab_time_millis+= 24;
        if (dab_time_millis >= 1000) {
            dab_time_millis -= 1000;
            ++dab_time_seconds;
        }
    }
}

void get_dab_time(time_t *time, uint32_t *millis)
{
    *time = dab_time_seconds;
    *millis = dab_time_millis;
}


uint32_t gregorian2mjd(int year, int month, int day)
{
    //This is the algorithm for the JD, just substract 2400000.5 for MJD
    year += 8000;
    if(month < 3) {
        year--;
        month += 12;
    }
    uint32_t JD =
        (year * 365) + (year / 4) - (year / 100) + (year / 400) - 1200820
        + ((month * 153 + 3) / 5) - 92 + (day - 1);

    return (uint32_t)(JD - 2400000.5); //truncation, loss of data OK!
}

void header_message()
{
    fprintf(stderr,
            "Welcome to %s %s, compiled at %s, %s",
            PACKAGE_NAME,
#if defined(GITVERSION)
            GITVERSION,
#else
            PACKAGE_VERSION,
#endif
            __DATE__, __TIME__);
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Copyright (C) 2023 Andy Mace (andy.mace@mediauk.net)\n");
    fprintf(stderr, "LICENCE: GPLv3+\n\n");

    fprintf(stderr, "http://opendigitalradio.org\n\n");

    fprintf(stderr, "Input URLs supported: udp\n");

    fprintf(stderr, "Output TS via UDP, SRT (depending on how TSDuck was compiled)\n");

}


void printUsage(char *name, FILE *out)
{
    /* We use fprintf here because this doesn't have
     * to go to the log. */
    fprintf(out, "NAME\n");
    fprintf(out, "  %s - UDP MPE Generator compliant to ETSI EN 300 468 / EN13818-1\n", name);
    fprintf(out, "\nSYNOPSYS\n");
    fprintf(out, "    This software requires a configuration file:\n");
    fprintf(out, "  %s configuration.mux\n", name);
    fprintf(out, "    See doc/example.config for an example format for the configuration file\n");
    fprintf(out, "\nDESCRIPTION\n");
    fprintf(out, "  %s  is a EDI collector that generates an TS stream from several muxes\n", name);
}

bool stringEndsWith(const std::string& fullString, const std::string& ending)
{
    return fullString.length() >= ending.length() and
        fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
}
