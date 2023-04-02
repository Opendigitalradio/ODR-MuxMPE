# Copyright (C) 2008, 2009 Her Majesty the Queen in Right of Canada
# (Communications Research Center Canada)
#
# Copyright (C) 2018 Matthias P. Braendli
#
# Copyright (C) 2023 Andy Mace
#
# http://opendigitalradio.org

# This file is part of ODR-MuxMPE.
#
# ODR-MuxMPE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# ODR-MuxMPE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ODR-MuxMPE.  If not, see <http://www.gnu.org/licenses/>.

if IS_GIT_REPO
GITVERSION_FLAGS = -DGITVERSION="\"`git describe --dirty`\""
else
GITVERSION_FLAGS =
endif

bin_PROGRAMS=odr-muxmpe

INCLUDE=-Ilib/charset -Ilib -Isrc

lib_charset_sources = lib/charset/charset.cpp \
					  lib/charset/charset.h \
					  lib/charset/utf8/checked.h \
					  lib/charset/utf8/core.h \
					  lib/charset/utf8/unchecked.h \
					  lib/charset/utf8.h

odr_muxmpe_CFLAGS   =-g -Wall $(INCLUDE) $(PTHREAD_CFLAGS) $(GITVERSION_FLAGS)
odr_muxmpe_CXXFLAGS =-Wall -std=c++11 $(PTHREAD_CXXFLAGS) $(INCLUDE) $(GITVERSION_FLAGS) $(BOOST_CPPFLAGS) $(TSDUCK_CFLAGS)
odr_muxmpe_LDADD    = $(BOOST_LDFLAGS) \
					 $(PTHREAD_CFLAGS) $(PTHREAD_LIBS) $(BOOST_SYSTEM_LIB) $(TSDUCK_LIBS)

odr_muxmpe_SOURCES  =src/MuxMPE.cpp \
					 src/utils.cpp \
					 src/utils.h \
					 lib/Globals.cpp \
					 lib/Log.h \
					 lib/Log.cpp \
					 lib/ThreadsafeQueue.h \
					 $(lib_charset_sources)

EXTRA_DIST	= COPYING NEWS README.md INSTALL.md LICENCE AUTHORS ChangeLog doc \
			  lib/fec/README.md lib/fec/LICENSE \
			  lib/charset/README



