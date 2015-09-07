#!/bin/sh

INST_DIR=/opt/ssp
export CPPFLAGS="-I$INST_DIR/include -I/usr/include -D_GNU_SOURCE $CPPFLAGS"
export LDFLAGS="-L$INST_DIR/lib -L/usr/lib64 -L/usr/lib $LDFLAGS"

if [ ! -d $INST_DIR/bin ]; then
    mkdir -p $INST_DIR/bin
fi

if [ ! -d $INST_DIR/etc/php.d ]; then
    mkdir -p $INST_DIR/etc/php.d
fi

#begin reflib
pushd reflib

#php
if [ ! -f "$INST_DIR/lib/libphp5.so" ]; then
    echo Installing php ...
    if [ ! -d "/tmp/php-5.6.2" ]; then
        tar -xvf php-5.6.2.tar.xz -C /tmp/
    fi
    pushd /tmp/php-5.6.2

	OPT_MAK="--prefix=/opt/ssp --with-config-file-path=/opt/ssp/etc --with-config-file-scan-dir=/opt/ssp/etc/php.d --with-mysql=mysqlnd --enable-inline-optimization --disable-debug --enable-bcmath --enable-calendar --enable-ctype --enable-ftp --enable-gd-native-ttf --enable-shmop --disable-sigchild --enable-sysvsem --enable-sysvshm --enable-wddx --with-gdbm=/opt/lampp --with-jpeg-dir=/opt/lampp --with-png-dir=/opt/lampp --with-freetype-dir=/opt/lampp --with-zlib=yes --with-zlib-dir=/opt/lampp --with-openssl=/opt/lampp --with-xsl=/opt/lampp --with-ldap=/opt/lampp --with-gd --with-imap=/opt/lampp/ --with-imap-ssl --with-gettext=/opt/lampp --with-mssql=/opt/lampp --with-sybase-ct=/opt/lampp --with-mysql-sock=/opt/lampp/var/mysql/mysql.sock --with-oci8=shared,instantclient,/opt/lampp/lib/instantclient --with-mcrypt=/opt/lampp --with-mhash=/opt/lampp --enable-sockets --enable-mbstring=all --with-curl=/opt/lampp --enable-mbregex --enable-exif --with-bz2=/opt/lampp --with-sqlite3=/opt/lampp --with-libxml-dir=/opt/lampp --enable-soap --enable-pcntl --with-mysqli=mysqlnd --with-iconv=/opt/lampp --with-pdo-mysql=mysqlnd --with-pdo-sqlite --with-icu-dir=/opt/lampp --enable-fileinfo --enable-phar --enable-zip --enable-intl"
    OPT_OTH="--enable-maintainer-zts  --with-tsrm-pthreads --enable-embed"

    export EXTENSION_DIR=$INST_DIR/lib/extensions
    ./configure ${OPT_MAK} ${OPT_OTH} && make && make install && cp -u php.ini-* $INST_DIR/etc/
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/php-5.6.2
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
    && make \
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

if [ ! -d "/usr/lib" -a -d "/usr/lib64" ]; then
	ln -s /usr/lib64 /usr/lib
elif [ ! -h "/usr/lib/pkgconfig" -a -d "/usr/lib64/pkgconfig" ]; then
	if [ -d "/usr/lib/pkgconfig" ]; then
		cp /usr/lib/pkgconfig/* /usr/lib64/pkgconfig/
		rm -rf /usr/lib/pkgconfig
	fi
	ln -s /usr/lib64/pkgconfig /usr/lib/pkgconfig
fi

#libffi
pkg-config --exists libffi
if [ "$?" != "0" ]; then
    echo Installing libffi ...
    if [ ! -d "/tmp/libffi-3.1" ]; then
        tar -zxvf libffi-3.1.tar.gz -C /tmp/
    fi
    pushd /tmp/libffi-3.1
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libffi-3.1
        echo Installed libffi Success.
    else
        popd
        echo Installed libffi error.
        exit 1
    fi
fi

#libpopt
if [ ! -f "usr/lib/libpopt.so" ]; then
    echo Installing libpopt ...
    if [ ! -d "/tmp/popt-1.7" ]; then
        tar -xvf popt-1.7.tar.gz -C /tmp/
    fi
    pushd /tmp/popt-1.7
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/popt-1.7
        echo Installed libpopt Success.
    else
        popd
        echo Installed libpopt error.
        exit 1
    fi
fi

#libgtop
pkg-config --exists popt-1.7
if [ "$?" != "0" ]; then
    echo Installing libgtop ...
    if [ ! -d "/tmp/libgtop-2.6.0" ]; then
        tar -zxvf libgtop-2.6.0.tar.gz -C /tmp/
    fi
    pushd /tmp/libgtop-2.6.0
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libgtop-2.6.0
        echo Installed libgtop Success.
    else
        popd
        echo Installed libgtop error.
        exit 1
    fi
fi

#libgtop
pkg-config --exists libgtop-2.0
if [ "$?" != "0" ]; then
    echo Installing libgtop ...
    if [ ! -d "/tmp/libgtop-2.6.0" ]; then
        tar -zxvf libgtop-2.6.0.tar.gz -C /tmp/
    fi
    pushd /tmp/libgtop-2.6.0
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libgtop-2.6.0
        echo Installed libgtop Success.
    else
        popd
        echo Installed libgtop error.
        exit 1
    fi
fi

echo All library installation has been completed.

#end reflib
popd

exit 0