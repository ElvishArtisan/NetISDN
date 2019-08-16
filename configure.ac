dnl configure.ac
dnl
dnl Autoconf configuration for NetISDN.
dnl Use autoconf to process this into a configure script
dnl
dnl   (C) Copyright 2002-2019 Fred Gleason <fredg@paravelsystems.com>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License version 2 as
dnl  published by the Free Software Foundation.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public
dnl  License along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

AC_INIT(netisdn,m4_esyscmd(cat PACKAGE_VERSION))
AM_INIT_AUTOMAKE
AC_SUBST(RPM_RELEASE,1)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

#
# Some Fully Resolved Install Paths
# (for the packaging system and friends)
#
if test ${prefix} = "NONE" ; then
  AC_SUBST(LOCAL_PREFIX,/usr/local)
else
  AC_SUBST(LOCAL_PREFIX,${prefix})
fi

#
# Basic Compiler Checks
#
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG(C++)

# to avoid rpath usage :
# http://wiki.debian.net/index.cgi?RpathIssue
case ${host} in
  *-pc-linux-gnu)
    AC_MSG_RESULT([Fixing libtool for -rpath problems.])
    sed < libtool > libtool-2 \
    's/^hardcode_libdir_flag_spec.*$/hardcode_libdir_flag_spec=" -D__LIBTOOL_IS_A_FOOL__ "/'
    mv libtool-2 libtool
    chmod 755 libtool
  ;;
esac

#
# Determine the target architecture
#
AR_GCC_TARGET()
AC_SUBST(VENDOR,$ar_gcc_distro)
AC_SUBST(ARCH,$ar_gcc_arch)

#
# Determine Distro
#
AR_GET_DISTRO()

#
# Option Switches
#
#AC_ARG_ENABLE(l3,[  --disable-mpeg-l3       disable MPEG Layer 3 support],
#		      [MPEG_L3_DISABLED=yes],[])
#AC_ARG_ENABLE(speex,[  --disable-speex         disable Speex support],
#		      [SPEEX_DISABLED=yes],[])
#AC_ARG_ENABLE(vorbis,[  --disable-vorbis        disable Vorbis support],
#		      [VORBIS_DISABLED=yes],[])

#
# Check for Qt (Mandatory)
#
BNV_HAVE_QT
if test $have_qt = "no" ; then
  AC_MSG_ERROR([*** Qt toolkit not found, unable to continue ***])
fi

#
# Check for Secret Rabbit Code (Mandatory)
#
AC_CHECK_HEADER(samplerate.h,[],[AC_MSG_ERROR([*** libsamplerate not found ***])])
AC_CHECK_FUNC(src_short_to_float_array,[SRC_CONV_FOUND=yes],[])

#
# Check for PortAudio (Mandatory)
#
AC_CHECK_HEADER(portaudio.h,[],
	[AC_MSG_ERROR([*** PortAudio not found ***])])

#
# Check for GeoIP (Mandatory)
#
AC_CHECK_HEADER(GeoIPCity.h,[],
	[AC_MSG_ERROR([*** GeoIPCity not found ***])])

#
# Check for NetFilter (Needed for packet loss simulator)
#
AC_CHECK_HEADER(libnetfilter_queue/libnetfilter_queue.h,[NETFILTER_FOUND=yes],[NETFILTER_FOUND=no])
if test $NETFILTER_FOUND ; then
  AM_CONDITIONAL(BUILD_NETFILTER,true)
else
  AM_CONDITIONAL(BUILD_NETFILTER,false)
fi
AC_CONFIG_MACRO_DIR([m4])

#
# Check for Vorbis
#
AC_CHECK_HEADER(vorbis/vorbisfile.h,[VORBIS_FOUND=yes],[VORBIS_FOUND=no])
if test $VORBIS_FOUND = yes ; then
  if test -z $VORBIS_DISABLED ; then
    AC_DEFINE(HAVE_VORBIS,yes)
    AC_SUBST(LIBVORBIS,"-lvorbisfile -lvorbisenc")
    USING_VORBIS=yes
  else
    AC_SUBST(LIBVORBIS,"")
  fi
else
  AC_SUBST(LIBVORBIS,"")
fi

#
# Check for Speex
#
AC_CHECK_HEADER(speex/speex.h,[SPEEX_FOUND=yes],[SPEEX_FOUND=no])
if test $SPEEX_FOUND = yes ; then
  if test -z $SPEEX_DISABLED ; then
    AC_DEFINE(HAVE_SPEEX)
    AC_SUBST(LIBSPEEX,"-lspeex")
    USING_SPEEX=yes
  else
    AC_SUBST(LIBSPEEX,"")
  fi
else
  AC_SUBST(LIBSPEEX,"")
fi

#
# Check for MPEG L3 Libraries
#
AC_CHECK_HEADER(lame/lame.h,[LAME_FOUND=yes],[LAME_FOUND=no])
AC_CHECK_HEADER(mad.h,[MAD_FOUND=yes],[])
if test $LAME_FOUND ; then
  if test $MAD_FOUND ; then
    if test -z $MPEG_L3_DISABLED ; then
      AC_DEFINE(HAVE_MPEG_L3,yes)
      AC_SUBST(MPEG_L3_LIBS,"-lmp3lame -lmad")
      USING_MPEG_L3=yes
    else
      AC_SUBST(MPEG_L3_LIBS,"")
    fi
  else
    AC_SUBST(MPEG_L3_LIBS,"")
  fi
else
  AC_SUBST(MPEG_L3_LIBS,"")
fi

#
# Configure RPM Build
#
AC_CHECK_PROG(RPMBUILD_FOUND,rpmbuild,[yes],[])
if test -z $RPMBUILD_FOUND ; then
  AC_SUBST(RPMBUILD,rpm)
else
  AC_SUBST(RPMBUILD,rpmbuild)
fi
if test -d /usr/src/redhat ; then
  AC_SUBST(RPM_ROOT,/usr/src/redhat)
  AC_SUBST(VENDOR,redhat)
else
  AC_SUBST(RPM_ROOT,/usr/src/packages)
  AC_SUBST(VENDOR,suse)
fi 
if test $ar_gcc_arch = x86_64 ; then
  AC_SUBST(NET_LIB_PATH,lib64)
else
  AC_SUBST(NET_LIB_PATH,lib)
fi

AC_CONFIG_FILES([ \
    Makefile \
    rc.netisdn \
    netisdn.spec \
    slack-desc \
    make_slack \
    helpers/Makefile \
    icons/Makefile \
    icons/autorun.inf \
    lib/Makefile \
    netisdn/Makefile \
    netsipd/Makefile \
    netadmin/Makefile \
    netfilter/Makefile \
    client/Makefile \
    xdg/Makefile \
    docs/Makefile \
    docs/tables/Makefile ])
AC_OUTPUT()

chmod 755 ./rc.netisdn
chmod 755 ./make_slack

AC_MSG_NOTICE()
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("|         *** NETISDN CONFIGURATION SUMMARY ***       |")
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("| Configured Audio Codecs:                            |")

if test -z $USING_MPEG_L3 ; then
AC_MSG_NOTICE("|                              MPEG Layer 3 ... No    |")
else
AC_MSG_NOTICE("|                              MPEG Layer 3 ... Yes   |")
fi

if test -z $USING_SPEEX ; then
AC_MSG_NOTICE("|                                     Speex ... No    |")
else
AC_MSG_NOTICE("|                                     Speex ... Yes   |")
fi

if test -z $USING_VORBIS ; then
AC_MSG_NOTICE("|                                    Vorbis ... No    |")
else
AC_MSG_NOTICE("|                                    Vorbis ... Yes   |")
fi
AC_MSG_NOTICE("|                                                     |")
if test $NETFILTER_FOUND ; then
AC_MSG_NOTICE("|   Build packet loss simulator (netfilter) ... Yes   |")
else
AC_MSG_NOTICE("|   Build packet loss simulator (netfilter) ... No    |")
fi
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE()
AC_MSG_NOTICE(Now enter 'make' to build the software.)
AC_MSG_NOTICE()