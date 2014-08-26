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
if [ ! -f libtool.lock ]; then
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
        touch libtool.lock
        echo Installed libtool Success.
    else
        popd
        echo Installed libtool error.
        exit 1
    fi
fi

#libgcrypt
if [ ! -f libgcrypt.lock ]; then
    echo Installing libgcrypt ...
    if [ ! -d "libgcrypt-1.4.5" ]; then
        tar -jxvf libgcrypt-1.4.5.tar.bz2
    fi
    pushd libgcrypt-1.4.5
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libgcrypt-1.4.5
        touch libgcrypt.lock
        echo Installed libgcrypt Success.
    else
        popd
        echo Installed libgcrypt error.
        exit 1
    fi
fi

#jpeg8
if [ ! -f jpeg8.lock ]; then
    echo Installing jpeg8 ...
    if [ ! -d "jpeg-8c" ]; then
        tar -zxvf jpegsrc.v8c.tar.gz
    fi
    pushd jpeg-8c
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf jpeg-8c
        touch jpeg8.lock
        echo Installed jpeg8 Success.
    else
        popd
        echo Installed jpeg8 error.
        exit 1
    fi
fi

#libpng
if [ ! -f libpng.lock ]; then
    echo Installing libpng ...
    if [ ! -d "libpng-1.5.4beta08" ]; then
        tar -jxvf libpng-1.5.4beta08.tar.bz2
    fi
    pushd libpng-1.5.4beta08
    
    ./configure --prefix=$INST_DIR --enable-shared --enable-static \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libpng-1.5.4beta08
        touch libpng.lock
        echo Installed libpng Success.
    else
        popd
        echo Installed libpng error.
        exit 1
    fi
fi

#freetype
if [ ! -f freetype.lock ]; then
    echo Installing freetype ...
    if [ ! -d "freetype-2.4.5" ]; then
        tar -jxvf freetype-2.4.5.tar.bz2
    fi
    pushd freetype-2.4.5
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf freetype-2.4.5
        touch freetype.lock
        echo Installed freetype Success.
    else
        popd
        echo Installed freetype error.
        exit 1
    fi
fi

#gd(freetype)
if [ ! -f gd.lock ]; then
    echo Installing gd ...
    if [ ! -d "pierrejoye-gd-libgd-5551f61978e3/src" ]; then
        tar -jxvf pierrejoye-gd-libgd-5551f61978e3.tar.bz2
    fi
    pushd pierrejoye-gd-libgd-5551f61978e3/src
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf pierrejoye-gd-libgd-5551f61978e3
        touch gd.lock
        echo Installed gd Success.
    else
        popd
        echo Installed gd error.
        exit 1
    fi
fi

#curl
if [ ! -f curl.lock ]; then
    echo Installing curl ...
    if [ ! -d "curl-7.21.7" ]; then
        tar -jxvf curl-7.21.7.tar.bz2
    fi
    pushd curl-7.21.7
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf curl-7.21.7
        touch curl.lock
        echo Installed curl Success.
    else
        popd
        echo Installed curl error.
        exit 1
    fi
fi

#libxml
if [ ! -f libxml.lock ]; then
    echo Installing libxml ...
    if [ ! -d "libxml2-2.6.30" ]; then
        tar -jxvf libxml2-2.6.30.tar.bz2
    fi
    pushd libxml2-2.6.30
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libxml2-2.6.30
        touch libxml.lock
        echo Installed libxml Success.
    else
        popd
        echo Installed libxml error.
        exit 1
    fi
fi

#libxslt
if [ ! -f libxslt.lock ]; then
    echo Installing libxslt ...
    if [ ! -d "libxslt-1.1.22" ]; then
        tar -jxvf libxslt-1.1.22.tar.bz2
    fi
    pushd libxslt-1.1.22
    
    ./configure --prefix=$INST_DIR --with-libxml-prefix=/usr/local/libxml2 \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libxslt-1.1.22
        touch libxslt.lock
        echo Installed libxslt Success.
    else
        popd
        echo Installed libxslt error.
        exit 1
    fi
fi

#openssl
if [ ! -f openssl.lock ]; then
    echo Installing openssl ...
    if [ ! -d "/tmp/openssl-1.0.0d" ]; then
        tar -zxvf openssl-1.0.0d.tar.gz -C /tmp/
    fi
    pushd /tmp/openssl-1.0.0d
    
    ./config --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/openssl-1.0.0d
        touch openssl.lock
        echo Installed openssl Success.
    else
        popd
        echo Installed openssl error.
        exit 1
    fi
fi

#imap
#alter files /etc/inetd.conf and /etc/services exclude pop3 and imap front # number:
if [ ! -f imap.lock ]; then
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
        rm -rf imap-2007a
        touch imap.lock
        echo Installed imap Success.
    else
        popd
        echo Installed imap error.
        exit 1
    fi
fi

#mcrypt
if [ ! -f mcrypt.lock ]; then
    echo Installing mcrypt ...
    if [ ! -d "libmcrypt-2.5.7" ]; then
        tar -zxvf libmcrypt-2.5.7.tar.gz
    fi
    pushd libmcrypt-2.5.7
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libmcrypt-2.5.7
        touch mcrypt.lock
        echo Installed mcrypt Success.
    else
        popd
        echo Installed mcrypt error.
        exit 1
    fi
fi

#mhash
if [ ! -f mhash.lock ]; then
    echo Installing mhash ...
    if [ ! -d "mhash-0.9.9.9" ]; then
        tar -jxvf mhash-0.9.9.9.tar.bz2
    fi
    pushd mhash-0.9.9.9
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf mhash-0.9.9.9
        touch mhash.lock
        echo Installed mhash Success.
    else
        popd
        echo Installed mhash error.
        exit 1
    fi
fi

#php
if [ ! -f php.lock ]; then
    echo Installing php ...
    if [ ! -d "/tmp/php-5.3.15" ]; then
        tar -jxvf php-5.3.15.tar.bz2 -C /tmp/
    fi
    pushd /tmp/php-5.3.15

    OPT_MAK="--build=i386-redhat-linux-gnu --host=i386-redhat-linux-gnu --target=i686-redhat-linux-gnu --program-prefix= --prefix=$INST_DIR --exec-prefix=$INST_DIR --bindir=$INST_DIR/bin --sbindir=$INST_DIR/sbin --sysconfdir=$INST_DIR/etc --datadir=$INST_DIR/share --includedir=$INST_DIR/include --libdir=$INST_DIR/lib --libexecdir=$INST_DIR/libexec --localstatedir=$INST_DIR/var --sharedstatedir=$INST_DIR/var/lib --mandir=$INST_DIR/share/man --infodir=$INST_DIR/share/info --with-libdir=lib --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --with-exec-dir=$INST_DIR/bin --disable-debug"
    OPT_EXT="--with-pic --disable-rpath --without-pear --with-ldap=shared --with-bz2=shared --enable-zip=shared --with-freetype-dir=$INST_DIR --with-png-dir=$INST_DIR --with-xpm-dir=$INST_DIR --enable-gd-native-ttf --without-gdbm --with-gettext --with-iconv --with-jpeg-dir=$INST_DIR --with-imap --with-imap-ssl=$INST_DIR --with-openssl=shared,$INST_DIR --with-zlib --with-layout=GNU --enable-exif --enable-ftp --enable-magic-quotes --enable-sockets --with-kerberos --enable-ucd-snmp-hack --enable-shmop --enable-calendar --with-sqlite=shared --with-sqlite3=shared --with-xsl=shared,$INST_DIR --with-libxml-dir=$INST_DIR --enable-xml=shared --with-xmlrpc=shared --enable-dom=shared --enable-soap=shared --with-gd=shared,$INST_DIR --disable-dba --without-unixODBC --enable-xmlreader=shared --enable-xmlwriter=shared --enable-phar=shared --enable-fileinfo=shared --enable-json=shared --without-pspell --enable-wddx=shared --with-curl=shared,$INST_DIR --enable-bcmath=shared --with-mcrypt=shared,$INST_DIR --with-mhash=shared,$INST_DIR --enable-mbstring=shared,all --enable-mbregex=shared --with-mysql=shared --with-mysqli=shared --with-pdo-mysql=shared --with-pdo-sqlite=shared --with-pgsql=shared --with-pdo-pgsql=shared --enable-posix=shared --enable-pcntl=shared --enable-sysvsem=shared --enable-sysvshm=shared --enable-sysvmsg=shared --enable-maintainer-zts --enable-zend-multibyte --enable-inline-optimization"
    OPT_OTH="--enable-embed"

    export EXTENSION_DIR=$INST_DIR/lib/extensions
    ./configure ${OPT_MAK} ${OPT_EXT} ${OPT_OTH} && make && make install && cp -u php.ini-* $INST_DIR/etc/
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/php-5.3.15
        touch php.lock
        echo Installed php Success.
    else
        popd
        echo Installed php error.
        exit 1
    fi
fi

#libevent
if [ ! -f libevent.lock ]; then
    echo Installing libevent ...
    if [ ! -d "libevent-2.0.21-stable" ]; then
        tar -zxvf libevent-2.0.21-stable.tar.gz
    fi
    pushd libevent-2.0.21-stable
    
    ./configure --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libevent-2.0.21-stable
        touch libevent.lock
        echo Installed libevent Success.
    else
        popd
        echo Installed libevent error.
        exit 1
    fi
fi

#end reflib
popd

exit 0