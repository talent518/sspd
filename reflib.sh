#!/bin/bash --login

cpus=$(cat /proc/cpuinfo | grep processor | wc -l)
alias make="make -j$cpus"

INST_DIR=/opt/ssp74
PHPVER=7.4.2

if [ ! -d $INST_DIR/bin ]; then
    mkdir -p $INST_DIR/bin
fi

if [ ! -d $INST_DIR/etc/php.d ]; then
    mkdir -p $INST_DIR/etc/php.d
fi

#begin reflib
pushd reflib

#php
if [ ! -f "$INST_DIR/lib/libphp7.so" ]; then
    echo Installing library ...
    SUDO=sudo
    if [ "$USER" = "root" ]; then
        SUDO=
    fi
    OPT_EXT=
    which apt-get > /dev/null
    if [ $? -eq 0]; then
        OPT_EXT="$OPT_EXT --with-imap --with-tidy --with-zip --with-ldap"
        $SUDO apt-get install libcurl4-openssl-dev libonig-dev libenchant-dev libffi-dev libgd-dev libc-client2007e-dev libkrb5-dev libldap2-dev unixodbc-dev libreadline-dev libreadline6-dev libmm-dev libsnmp-dev snmp snmpd libtidy-dev libxslt1-dev libzip-dev libmysqlclient-dev
    else
        $SUDO yum -y install libXpm-devel bzip2 bzip2-devel gcc gcc-c++ kernel-devel libxml2-devel libxslt-devel krb5-devel openssl-devel sqlite-devel libcurl-devel enchant-devel libffi-devel gd-devel libwebp-devel gmp-devel libicu-devel readline-devel net-snmp net-snmp-devel expat-devel libzip-devel
        $SUDO yum -y install libc-client-devel
        OPT_EXT="$OPT_EXT --disable-mbregex"
        if [ -d "/usr/include/c-client" ]; then
            OPT_EXT="$OPT_EXT --with-imap"
        fi
        pkg-config --atleast-version=0.11 libzip
        if [ $? -eq 0 ]; then
            OPT_EXT="$OPT_EXT --with-zip"
        fi
    fi
    echo Installing php ...
    if [ ! -d "/tmp/php-$PHPVER" ]; then
        tar -xvf php-$PHPVER.tar.bz2 -C /tmp/
    fi
    pushd /tmp/php-$PHPVER
    OPT_MAK="--prefix=$INST_DIR --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --enable-inline-optimization --enable-maintainer-zts--with-tsrm-pthreads --enable-embed --with-openssl --with-system-ciphers --with-zlib --enable-bcmath --with-bz2 --enable-calendar --with-curl --with-enchant --enable-exif--with-ffi --enable-ftp --enable-gd --with-webp --with-jpeg --with-xpm --with-freetype --with-gettext --with-gmp --with-mhash --with-imap-ssl --enable-intl --enable-mbstring --with-mysqli=mysqlnd --enable-pcntl --enable-pdo --with-pdo-mysql --with-readline --enable-shmop --with-snmp --enable-soap --enable-sockets --enable-sysvmsg --enable-sysvsem --enable-sysvshm --with-expat --with-xmlrpc --with-expat --with-xsl --enable-mysqlnd --with-pear --with-kerberos --with-libxml --enable-dom --enable-xml"

    EXTENSION_DIR=$INST_DIR/lib/extensions ./configure CFLAGS=-O2 CXXFLAGS=-O2 ${OPT_MAK} ${OPT_EXT} && make -j$cpus && make install && cp -u php.ini-development $INST_DIR/etc/php.ini

    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/php-$PHPVER
        echo Installed php Success.
    else
        popd
        echo Installed php error.
        exit 1
    fi
fi

#libevent
if [ ! -f "$INST_DIR/lib/libevent.so" ]; then
    echo Installing libevent ...
    if [ ! -d "/tmp/libevent-2.0.21-stable" ]; then
        tar -zxvf libevent-2.0.21-stable.tar.gz -C /tmp/
    fi
    pushd /tmp/libevent-2.0.21-stable

    ./configure --prefix=$INST_DIR \
    && make -j$cpus \
    && make install

    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libevent-2.0.21-stable
        echo Installed libevent Success.
    else
        popd
        echo Installed libevent error.
        exit 1
    fi
fi

echo All library installation has been completed.

#end reflib
popd

exit 0
