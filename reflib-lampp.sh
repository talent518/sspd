#!/bin/bash --login

cpus=$(cat /proc/cpuinfo | grep processor | wc -l)

INST_DIR=/opt/ssp74
PHPVER=7.4.11
EVVER=2.1.12

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
    echo Installing php ...
    if [ ! -d "/tmp/php-$PHPVER" ]; then
        tar -xvf php-$PHPVER.tar.bz2 -C /tmp/
    fi
    pushd /tmp/php-$PHPVER
    OPT_MAK="--prefix=$INST_DIR --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --enable-inline-optimization --enable-maintainer-zts --with-tsrm-pthreads --enable-embed --with-openssl --with-system-ciphers --with-zlib --enable-bcmath --enable-calendar --with-curl --enable-exif --enable-ftp --enable-gd --with-freetype --with-gettext --with-mhash --enable-mbstring --disable-mbregex --with-mysqli=mysqlnd --enable-pcntl --enable-pdo --with-pdo-mysql --with-readline --enable-shmop --enable-soap --enable-sockets --enable-sysvmsg --enable-sysvsem --enable-sysvshm --with-xmlrpc --with-xsl --enable-mysqlnd --with-pear --with-libxml --enable-dom --enable-xml"

    sed -i 's|ac_have_decl=1|ac_have_decl=0|g' configure
    PKG_CONFIG_PATH=/opt/lampp/lib/pkgconfig EXTENSION_DIR=$INST_DIR/lib/extensions ./configure CFLAGS=-O2 CXXFLAGS=-O2 ${OPT_MAK} && make -j$cpus && make install && cp -u php.ini-development $INST_DIR/etc/php.ini
    
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
    if [ ! -d "/tmp/libevent-$EVVER-stable" ]; then
        tar -zxvf libevent-$EVVER-stable.tar.gz -C /tmp/
    fi
    pushd /tmp/libevent-$EVVER-stable
    
    ./configure --prefix=$INST_DIR \
    && make -j$cpus \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libevent-$EVVER-stable
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
