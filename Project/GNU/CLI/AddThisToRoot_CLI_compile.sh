#!/bin/sh

##########################################################################
Parallel_Build () {
    local numprocs=1
    case $OS in
    'linux')
        numprocs=`grep -c ^processor /proc/cpuinfo 2>/dev/null`
        ;;
    'mac')
        if type sysctl &> /dev/null; then
            numprocs=`sysctl -n hw.ncpu`
        fi
        ;;
    #'freebsd')
    #    ;;
    #"solaris')
    #    on Solaris you need to use psrinfo -p instead
    #    ;;
    *) ;;
    esac
    if [ "$numprocs" = "" ] || [ "$numprocs" = "0" ]; then
        numprocs=1
    fi
    make -s -j$numprocs
}

##########################################################################
ZenLib () {
    if test -e ZenLib/Project/GNU/Library/configure; then
        cd ZenLib/Project/GNU/Library/
        test -e Makefile && rm Makefile
        chmod u+x configure
        if [ "$OS" = "mac" ]; then
            ./configure $MacOptions $ZenLib_Options $*
        else
            ./configure $ZenLib_Options $*
        fi
        if test -e Makefile; then
            make clean
            Parallel_Build
            if test -e libzen.la; then
                echo ZenLib compiled
            else
                echo Problem while compiling ZenLib
                exit
            fi
        else
            echo Problem while configuring ZenLib
            exit
        fi
    else
        echo ZenLib directory is not found
        exit
    fi
    cd $Home
}

##########################################################################
MediaInfoLib () {
    if test -e MediaInfoLib/Project/GNU/Library/configure; then
        cd MediaInfoLib/Project/GNU/Library/
        test -e Makefile && rm Makefile
        chmod u+x configure
        if [ "$OS" = "mac" ]; then
            ./configure $MacOptions $*
        else
            ./configure $*
        fi
        if test -e Makefile; then
            make clean
            Parallel_Build
            #if test "$(./libmediainfo-config la_name)" != "" && test -e $(./libmediainfo-config la_name); then
            if test -e libmediainfo.la; then
                echo MediaInfoLib compiled
            else
                echo Problem while compiling MediaInfoLib
                exit
            fi
        else
            echo Problem while configuring MediaInfoLib
            exit
        fi
    else
        echo MediaInfoLib directory is not found
        exit
    fi
    cd $Home
}

##########################################################################
MediaConch() {
    if test -e MediaConch/Project/GNU/CLI/configure; then
        cd MediaConch/Project/GNU/CLI/
        test -e Makefile && rm Makefile
        chmod u+x configure
        if [ "$OS" = "mac" ]; then
            ./configure $MacOptions --enable-staticlibs $*
        else
            ./configure --enable-staticlibs $*
        fi
        if test -e Makefile; then
            make clean
            Parallel_Build
            if test -e mediaconch; then
            echo "MediaConch (CLI) compiled"
            else
                echo "Problem while compiling MediaConch (CLI)"
                exit
            fi
        else
            echo "Problem while configuring MediaConch (CLI)"
            exit
        fi
    else
        echo MediaConch directory is not found
        exit
    fi
    cd $Home
}

#########################################################################

Home=`pwd`
ZenLib_Options=""
# For wx compilation
MacOptions="--with-macosx-version-min=10.5"

OS=$(uname -s)
# expr isn't available on mac
if [ "$OS" = "Darwin" ]; then
    OS="mac"
# if the 5 first caracters of $OS equal "Linux"
elif [ "$(expr substr $OS 1 5)" = "Linux" ]; then
    OS="linux"
#elif [ "$(expr substr $OS 1 7)" = "FreeBSD" ]; then
#    OS="freebsd"
#elif [ "$(expr substr $OS 1 5)" = "SunOS" ]; then
#    OS="solaris"
fi

ZenLib
MediaInfoLib
MediaConch

echo "MediaConch executable is MediaConch/Project/GNU/CLI/mediaconch"
echo "For installing, cd MediaConch/Project/GNU/CLI && make install"

unset -v Home ZenLib_Options MacOptions OS
