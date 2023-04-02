/*
   Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012 Her Majesty the Queen in Right of Canada (Communications
   Research Center Canada)

   Copyright (C) 2020 Matthias P. Braendli, matthias.braendli@mpb.li
   Copyright (C) 2023 Andy Mace, andy.mace@mediauk.net

   This file contains a set of utility functions that are used to show
   useful information to the user, and handles time and date for the
   the signalling.
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
#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <memory>

/* Must be called once per ETI frame to update the time */
void update_dab_time(void);
void get_dab_time(time_t *time, uint32_t *millis);

/* Convert a date and time into the modified Julian date
 * used in FIG 0/10
 *
 * Year is four digit format.
 * Months are Jan=1, Feb=2, etc.
 * First day of the month is 1, as usual.
 *
 * Returns corresponding MJD
 */
uint32_t gregorian2mjd(int year, int month, int day);

/* Shows the introductory header on program start */
void header_message();

/* The usage information refers to the command-line
 * ensemble definition, and explains how to create
 * an ensemble without using a configuration file
 */
void printUsage(char *name, FILE* out = stderr);

/* This usage information explains how to run the program
 * with a configuration file
 */
void printUsageConfigfile(char *name, FILE* out = stderr);


unsigned long hexparse(const std::string& input);

bool stringEndsWith(std::string const &fullString, std::string const &ending);


class InitException : public std::exception
{
    public:
        InitException(const std::string m = "ODR-MuxMPE initialisation error")
            throw()
            : msg(m) {}
        ~InitException(void) throw() {}
        const char* what() const throw() { return msg.c_str(); }
    private:
        std::string msg;
};

struct ts_destination_t {
    
    unsigned int payload_pid;
    unsigned int pmt_pid;
    unsigned int ts_id;
    unsigned int service_type;
    unsigned int service_id;
    std::string service_name;
    std::string service_provider_name;
    std::string output;
    std::string output_host;
    unsigned int output_port;
    unsigned int output_ttl;
    std::string output_srt_passphrase;
    std::string output_source_address;
};

struct udp_source_t {
    std::string name;
    std::string protocol;
    std::string source;
    unsigned int sourceport;
    std::string sourceip; 
};

