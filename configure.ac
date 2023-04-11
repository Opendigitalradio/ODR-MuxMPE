# Copyright (C) 2023 Andy Mace, http://opendigitalradio.org

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

AC_PREREQ([2.69])
AC_INIT([ODR-MuxMPE],[1.0.0],[andy.mace@mediauk.net])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_SRCDIR([src/MuxMPE.cpp])
AC_CONFIG_HEADERS([config.h])
# AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

AX_CXX_COMPILE_STDCXX(11,noext,mandatory)

# Checks for libraries.
AX_PTHREAD([], AC_MSG_ERROR([requires pthread]))
AX_BOOST_BASE([1.48.0], [], AC_MSG_ERROR([BOOST 1.48 or later is required]))
AX_BOOST_SYSTEM

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Options
# Outputs

# TS
PKG_CHECK_MODULES([TSDUCK], [tsduck >= 3.33.0], [
        TSDUCK_CPPFLAGS=`tsconfig --cflags`
        TSDUCK_LIBS=`tsconfig --libs`
        TSDUCK_PREFIX=`pkg-config --variable=prefix tsduck`
        AC_SUBST([TSDUCK_PREFIX])
])

OATPP_INCLUDE_DIRS="-I/usr/local/include/oatpp-1.3.0/oatpp -I/usr/local/include/oatpp-1.3.0/oatpp-swagger"
OATPP_LIBS="-latomic /usr/local/lib/oatpp-1.3.0/liboatpp-swagger.a /usr/local/lib/oatpp-1.3.0/liboatpp.a "
AC_DEFINE([OATPP_SWAGGER_RES_PATH], ["/usr/local/include/oatpp-1.3.0/bin/oatpp-swagger/res"], [Define the path to oatpp-swagger res])
AC_DEFINE([OATPP_BUILD_TESTS], [False], [Define to disable OATPP test build])

AC_SUBST(OATPP_INCLUDE_DIRS)
AC_SUBST(OATPP_LIBS)

AC_LANG_PUSH([C++])
AX_CHECK_COMPILE_FLAG([-Wduplicated-cond], [CXXFLAGS="$CXXFLAGS -Wduplicated-cond"], [], ["-Werror"])
AX_CHECK_COMPILE_FLAG([-Wduplicated-branches], [CXXFLAGS="$CXXFLAGS -Wduplicated-branches"], [], ["-Werror"])
AX_CHECK_COMPILE_FLAG([-Wlogical-op], [CXXFLAGS="$CXXFLAGS -Wlogical-op"], [], ["-Werror"])
AX_CHECK_COMPILE_FLAG([-Wrestrict], [CXXFLAGS="$CXXFLAGS -Wrestrict"], [], ["-Werror"])
AX_CHECK_COMPILE_FLAG([-Wdouble-promotion], [CXXFLAGS="$CXXFLAGS -Wdouble-promotion"], [], ["-Werror"])
AX_CHECK_COMPILE_FLAG(["-Wformat=2"], [CXXFLAGS="$CXXFLAGS -Wformat=2"], [], ["-Werror"])

# Linux defines MSG_NOSIGNAL, some other systems have SO_NOSIGPIPE instead
AC_MSG_CHECKING(for MSG_NOSIGNAL)
AC_COMPILE_IFELSE([ AC_LANG_PROGRAM([[
                    #include <sys/socket.h>
                    int f = MSG_NOSIGNAL;
                    ]])],
                   [ AC_MSG_RESULT(yes)
                     AC_DEFINE(HAVE_MSG_NOSIGNAL, 1, [Define this symbol if you have MSG_NOSIGNAL]) ],
                   [ AC_MSG_RESULT(no) ])

AC_MSG_CHECKING(for SO_NOSIGPIPE)
AC_COMPILE_IFELSE([ AC_LANG_PROGRAM([[
                    #include <sys/socket.h>
                    int f = SO_NOSIGPIPE;
                    ]])],
                   [ AC_MSG_RESULT(yes)
                     AC_DEFINE(HAVE_SO_NOSIGPIPE, 1, [Define this symbol if you have SO_NOSIGPIPE]) ],
                   [ AC_MSG_RESULT(no) ])

AC_LANG_POP([C++])


AM_CONDITIONAL([IS_GIT_REPO], [test -d '.git'])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
echo "***********************************************"
echo "Successfuly Configured."
echo "***********************************************"
echo

#We need to remove the #define VERSION as its used in the TSDUCK library.
#We don't use it anywhere in code.
sed -i '/^#define VERSION/d' ${srcdir}/config.h
