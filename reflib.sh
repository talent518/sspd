#!/bin/sh

INST_DIR=/opt/ssp

if [ ! -d $INST_DIR/bin ]; then
    mkdir -p $INST_DIR/bin
fi

if [ ! -d $INST_DIR/etc/php.d ]; then
    mkdir -p $INST_DIR/etc/php.d
fi

#begin reflib
pushd reflib

#libtool
if [ ! -f "/usr/bin/libtool" ]; then
    echo Installing libtool ...
    if [ ! -d "libtool-2.4" ]; then
        tar -zxvf libtool-2.4.tar.gz
    fi
    pushd libtool-2.4
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libtool-2.4
        echo Installed libtool Success.
    else
        popd
        echo Installed libtool error.
        exit 1
    fi
fi

#jpeg8
if [ ! -f "$INST_DIR/lib/libjpeg.so" ]; then
    echo Installing jpeg8 ...
    if [ ! -d "/tmp/jpeg-8c" ]; then
        tar -zxvf jpegsrc.v8c.tar.gz -C /tmp/
    fi
    pushd /tmp/jpeg-8c
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/jpeg-8c
        echo Installed jpeg8 Success.
    else
        popd
        echo Installed jpeg8 error.
        exit 1
    fi
fi

#libpng
if [ ! -f "$INST_DIR/lib/libpng.so" ]; then
    echo Installing libpng ...
    if [ ! -d "/tmp/libpng-1.5.4beta08" ]; then
        tar -jxvf libpng-1.5.4beta08.tar.bz2 -C /tmp/
    fi
    pushd /tmp/libpng-1.5.4beta08
    
    ./configure --prefix=$INST_DIR --enable-shared --enable-static \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libpng-1.5.4beta08
        echo Installed libpng Success.
    else
        popd
        echo Installed libpng error.
        exit 1
    fi
fi

#freetype
if [ ! -f "$INST_DIR/lib/libfreetype.so" ]; then
    echo Installing freetype ...
    if [ ! -d "/tmp/freetype-2.4.5" ]; then
        tar -jxvf freetype-2.4.5.tar.bz2 -C /tmp/
    fi
    pushd /tmp/freetype-2.4.5
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/freetype-2.4.5
        echo Installed freetype Success.
    else
        popd
        echo Installed freetype error.
        exit 1
    fi
fi

#gd(freetype)
if [ ! -f "$INST_DIR/lib/libgd.so" ]; then
    echo Installing gd ...
    if [ ! -d "/tmp/libgd-2.1.0" ]; then
        tar -zxvf libgd-2.1.0.tar.gz -C /tmp/
    fi
    pushd /tmp/libgd-2.1.0
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libgd-2.1.0
        echo Installed gd Success.
    else
        popd
        echo Installed gd error.
        exit 1
    fi
fi

#curl
if [ ! -f "$INST_DIR/lib/libcurl.so" ]; then
    echo Installing curl ...
    if [ ! -d "/tmp/curl-7.21.7" ]; then
        tar -jxvf curl-7.21.7.tar.bz2 -C /tmp/
    fi
    pushd /tmp/curl-7.21.7
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/curl-7.21.7
        echo Installed curl Success.
    else
        popd
        echo Installed curl error.
        exit 1
    fi
fi

#libxml
if [ ! -f "$INST_DIR/lib/libxml2.so" ]; then
    echo Installing libxml ...
    if [ ! -d "/tmp/libxml2-2.6.30" ]; then
        tar -jxvf libxml2-2.6.30.tar.bz2 -C /tmp/
    fi
    pushd /tmp/libxml2-2.6.30
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libxml2-2.6.30
        echo Installed libxml Success.
    else
        popd
        echo Installed libxml error.
        exit 1
    fi
fi

#libgcrypt
if [ ! -f "/usr/lib/libgcrypt.so" ]; then
    echo Installing libgcrypt ...
    if [ ! -d "/tmp/libgcrypt-1.4.5" ]; then
        tar -jxvf libgcrypt-1.4.5.tar.bz2 -C /tmp/
    fi
    pushd /tmp/libgcrypt-1.4.5
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libgcrypt-1.4.5
        echo Installed libgcrypt Success.
    else
        popd
        echo Installed libgcrypt error.
        exit 1
    fi
fi

#libxslt
if [ ! -f "$INST_DIR/lib/libxslt.so" ]; then
    echo Installing libxslt ...
    if [ ! -d "/tmp/libxslt-1.1.22" ]; then
        tar -jxvf libxslt-1.1.22.tar.bz2 -C /tmp/
    fi
    pushd /tmp/libxslt-1.1.22
    
    ./configure --prefix=$INST_DIR --with-libxml-prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libxslt-1.1.22
        echo Installed libxslt Success.
    else
        popd
        echo Installed libxslt error.
        exit 1
    fi
fi

#openssl
if [ ! -f "$INST_DIR/lib/libssl.a" ]; then
    echo Installing openssl ...
    if [ ! -d "/tmp/openssl-1.0.0d" ]; then
        tar -zxvf openssl-1.0.0d.tar.gz -C /tmp/
    fi
    pushd /tmp/openssl-1.0.0d
    
	if [ `uname -p` = "x86_64" ]; then
		./Configure -fPIC --prefix=$INST_DIR \
		&& make \
		&& make install
	else
		./config --prefix=$INST_DIR \
		&& make \
		&& make install
	fi
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/openssl-1.0.0d
        echo Installed openssl Success.
    else
        popd
        echo Installed openssl error.
        exit 1
    fi
fi

#imap
#alter files /etc/inetd.conf and /etc/services exclude pop3 and imap front # number:
if [ ! -f "$INST_DIR/lib/libc-client.a" ]; then
    echo Installing imap ...
    if [ ! -d "/tmp/imap-2007a" ]; then
        tar -zxvf imap-2007a1.tar.gz -C /tmp/
    fi
    pushd /tmp/imap-2007a
    
    make lr5 \
    && cp -u c-client/c-client.a $INST_DIR/lib/libc-client.a \
    && cp -u c-client/*.h $INST_DIR/include \
    && cp -u ipopd/ipop2d $INST_DIR/bin \
    && cp -u ipopd/ipop3d $INST_DIR/bin \
    && cp -u imapd/imapd $INST_DIR/bin
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/imap-2007a
        echo Installed imap Success.
    else
        popd
        echo Installed imap error.
        exit 1
    fi
fi

#mcrypt
if [ ! -f "$INST_DIR/lib/libmcrypt.so" ]; then
    echo Installing mcrypt ...
    if [ ! -d "/tmp/libmcrypt-2.5.7" ]; then
        tar -zxvf libmcrypt-2.5.7.tar.gz -C /tmp/
    fi
    pushd /tmp/libmcrypt-2.5.7
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/libmcrypt-2.5.7
        echo Installed mcrypt Success.
    else
        popd
        echo Installed mcrypt error.
        exit 1
    fi
fi

#mhash
if [ ! -f "$INST_DIR/lib/libmhash.so" ]; then
    echo Installing mhash ...
    if [ ! -d "/tmp/mhash-0.9.9.9" ]; then
        tar -jxvf mhash-0.9.9.9.tar.bz2 -C /tmp/
    fi
    pushd /tmp/mhash-0.9.9.9
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/mhash-0.9.9.9
        echo Installed mhash Success.
    else
        popd
        echo Installed mhash error.
        exit 1
    fi
fi

#Berkeley DB
if [ ! -f "$INST_DIR/include/ldap.h" ]; then
    echo Installing Berkeley DB ...
    if [ ! -d "/tmp/db-4.7.25" ]; then
        tar -xvf db-4.7.25.tar.gz -C /tmp/
    fi
    pushd /tmp/db-4.7.25/build_unix
    
    ../dist/configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/db-4.7.25
        echo Installed Berkeley DB Success.
    else
        popd
        echo Installed Berkeley DB error.
        exit 1
    fi
fi

#openldap
if [ ! -f "$INST_DIR/include/ldap.h" ]; then
    echo Installing openldap ...
    if [ ! -d "/tmp/openldap-2.4.15" ]; then
        tar -xvf openldap-2.4.15.tgz -C /tmp/
    fi
    pushd /tmp/openldap-2.4.15
    
	export CPPFLAGS="-I$INST_DIR/include -D_GNU_SOURCE $CPPFLAGS"
	export LDFLAGS="-L$INST_DIR/lib $LDFLAGS"

    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/openldap-2.4.15
        echo Installed openldap Success.
    else
        popd
        echo Installed openldap error.
        exit 1
    fi
fi

if [ ! -d "$INST_DIR/lib64" -a `uname -p` = "x86_64" ]; then
	ln -s $INST_DIR/lib $INST_DIR/lib64
fi

#php
if [ ! -f "$INST_DIR/lib/libphp5.so" ]; then
    echo Installing php ...
    if [ ! -d "/tmp/php-5.6.13" ]; then
        tar -xvf php-5.6.13.tar.bz2 -C /tmp/
    fi
    pushd /tmp/php-5.6.13

	if [ `uname -p` = "x86_64" ]; then
		OPT_MAK="--prefix=$INST_DIR -bindir=$INST_DIR/bin --sbindir=$INST_DIR/sbin --sysconfdir=$INST_DIR/etc --datadir=$INST_DIR/share --includedir=$INST_DIR/include --libdir=$INST_DIR/lib --libexecdir=$INST_DIR/libexec --localstatedir=$INST_DIR/var --sharedstatedir=$INST_DIR/var/lib --mandir=$INST_DIR/share/man --infodir=$INST_DIR/share/info --with-libdir=lib64 --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --enable-shared"
	else
		OPT_MAK="--prefix=$INST_DIR -bindir=$INST_DIR/bin --sbindir=$INST_DIR/sbin --sysconfdir=$INST_DIR/etc --datadir=$INST_DIR/share --includedir=$INST_DIR/include --libdir=$INST_DIR/lib --libexecdir=$INST_DIR/libexec --localstatedir=$INST_DIR/var --sharedstatedir=$INST_DIR/var/lib --mandir=$INST_DIR/share/man --infodir=$INST_DIR/share/info --with-libdir=lib --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --enable-shared"
	fi

	OPT_EXT="--disable-rpath --without-pear --with-ldap=shared,$INST_DIR --with-bz2=shared --enable-zip=shared --with-freetype-dir=$INST_DIR --with-png-dir=$INST_DIR --with-xpm-dir=$INST_DIR --enable-gd-native-ttf --with-jpeg-dir=$INST_DIR --with-gd=shared,$INST_DIR --without-gdbm --with-iconv --with-openssl=shared,$INST_DIR --with-zlib=shared --with-layout=GNU --enable-exif=shared --enable-sockets --enable-shmop --with-sqlite3=shared --with-xsl=shared,$INST_DIR --with-libxml-dir=$INST_DIR --enable-xml --disable-simplexml --disable-dba --without-unixODBC --enable-xmlreader=shared, --enable-xmlwriter=shared --enable-json=shared --without-pspell --with-curl=shared,$INST_DIR --enable-bcmath=shared --with-mcrypt=shared,$INST_DIR --with-mhash=shared,$INST_DIR --enable-mbstring=all --enable-mbregex --with-mysql --with-mysqli --with-pdo-mysql --with-pdo-sqlite=shared --enable-posix --enable-pcntl --enable-sysvsem --enable-sysvshm --enable-sysvmsg --enable-maintainer-zts  --with-tsrm-pthreads --enable-inline-optimization --disable-ctype --disable-tokenizer --disable-session --disable-phar --disable-fileinfo"

    OPT_OTH="--enable-embed"

    export EXTENSION_DIR=$INST_DIR/lib/extensions
    ./configure ${OPT_MAK} ${OPT_EXT} ${OPT_OTH} && make && make install && cp -u php.ini-* $INST_DIR/etc/
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/php-5.6.13
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
