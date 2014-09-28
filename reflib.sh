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
    if [ ! -d "libgcrypt-1.4.5" ]; then
        tar -jxvf libgcrypt-1.4.5.tar.bz2
    fi
    pushd libgcrypt-1.4.5
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libgcrypt-1.4.5
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
    if [ ! -d "libxslt-1.1.22" ]; then
        tar -jxvf libxslt-1.1.22.tar.bz2
    fi
    pushd libxslt-1.1.22
    
    ./configure --prefix=$INST_DIR --with-libxml-prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf libxslt-1.1.22
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
    if [ ! -d "openssl-1.0.0d" ]; then
        tar -zxvf openssl-1.0.0d.tar.gz
    fi
    pushd openssl-1.0.0d
    
    ./config --prefix=$INST_DIR \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf openssl-1.0.0d
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
    if [ ! -d "imap-2007a" ]; then
        tar -zxvf imap-2007a1.tar.gz
    fi
    pushd imap-2007a
    
    make lr5 \
    && cp -u c-client/c-client.a $INST_DIR/lib/libc-client.a \
    && cp -u c-client/*.h $INST_DIR/include \
    && cp -u ipopd/ipop2d $INST_DIR/bin \
    && cp -u ipopd/ipop3d $INST_DIR/bin \
    && cp -u imapd/imapd $INST_DIR/bin
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf imap-2007a
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
if [ ! -f "$INST_DIR/lib/libmhash.so" ]; then
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
        echo Installed mhash Success.
    else
        popd
        echo Installed mhash error.
        exit 1
    fi
fi

#php
if [ ! -f "$INST_DIR/lib/libphp5.so" ]; then
    echo Installing php ...
    if [ ! -d "php-5.3.15" ]; then
        tar -jxvf php-5.3.15.tar.bz2
    fi
    pushd php-5.3.15

    OPT_MAK="--build=i386-redhat-linux-gnu --host=i386-redhat-linux-gnu --target=i686-redhat-linux-gnu --program-prefix= --prefix=$INST_DIR --exec-prefix=$INST_DIR --bindir=$INST_DIR/bin --sbindir=$INST_DIR/sbin --sysconfdir=$INST_DIR/etc --datadir=$INST_DIR/share --includedir=$INST_DIR/include --libdir=$INST_DIR/lib --libexecdir=$INST_DIR/libexec --localstatedir=$INST_DIR/var --sharedstatedir=$INST_DIR/var/lib --mandir=$INST_DIR/share/man --infodir=$INST_DIR/share/info --with-libdir=lib --with-config-file-path=$INST_DIR/etc --with-config-file-scan-dir=$INST_DIR/etc/php.d --with-exec-dir=$INST_DIR/bin --disable-debug"
    OPT_EXT="--disable-rpath --without-pear --with-ldap=shared --with-bz2=shared --enable-zip=shared --with-freetype-dir=$INST_DIR --with-png-dir=$INST_DIR --with-xpm-dir=$INST_DIR --enable-gd-native-ttf --with-jpeg-dir=$INST_DIR --with-gd=shared,$INST_DIR --without-gdbm --with-iconv --with-openssl=shared,$INST_DIR --with-zlib=shared --with-layout=GNU --enable-exif=shared --disable-magic-quotes --enable-sockets --enable-shmop --with-sqlite=shared --with-sqlite3=shared --with-xsl=shared,$INST_DIR --with-libxml-dir=$INST_DIR --enable-xml --disable-simplexml --disable-dba --without-unixODBC --enable-xmlreader=shared, --enable-xmlwriter=shared --enable-json=shared --without-pspell --with-curl=shared,$INST_DIR --enable-bcmath=shared --with-mcrypt=shared,$INST_DIR --with-mhash=shared,$INST_DIR --enable-mbstring=all --enable-mbregex --with-mysql --with-mysqli --with-pdo-mysql --with-pdo-sqlite=shared --enable-posix --enable-pcntl --enable-sysvsem --enable-sysvshm --enable-sysvmsg --enable-maintainer-zts  --with-tsrm-pthreads --enable-zend-multibyte --enable-inline-optimization --disable-ctype --disable-tokenizer --disable-session --disable-phar --disable-fileinfo"
    OPT_OTH="--enable-embed"

    export EXTENSION_DIR=$INST_DIR/lib/extensions
    ./configure ${OPT_MAK} ${OPT_EXT} ${OPT_OTH} && make && make install && cp -u php.ini-* $INST_DIR/etc/
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf php-5.3.15
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
        echo Installed libevent Success.
    else
        popd
        echo Installed libevent error.
        exit 1
    fi
fi

if [ ! -d "/usr/lib" ]; then
	ln -s /usr/lib64 /usr/lib
fi

#libffi
pkg-config --exists libffi
if [ "$?" != "0" ]; then
    echo Installing libffi ...
    if [ ! -d "/tmp/libffi-3.1" ]; then
        tar -zxvf libffi-3.1.tar.gz -C /tmp
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

#libglib
pkg-config --exists glib-2.0
if [ "$?" != "0" ]; then
    echo Installing libglib ...
    if [ ! -d "/tmp/glib-2.40.0" ]; then
        tar -xvf glib-2.40.0.tar.xz -C /tmp
    fi
    pushd /tmp/glib-2.40.0
    
    ./configure --prefix=/usr \
    && make \
    && make install
    
    if [ "$?" = "0" ]; then
        popd
        rm -rf /tmp/glib-2.40.0
        echo Installed libglib Success.
    else
        popd
        echo Installed libglib error.
        exit 1
    fi
fi

#procps
if [ ! -f "/usr/include/proc/readproc.h" ]; then
    echo Installing procps ...
    if [ ! -d "/tmp/procps-3.2.8" ]; then
        tar -zxvf procps-3.2.8_835.tar.gz -C /tmp
    fi
    pushd /tmp/procps-3.2.8
    
    make && make install
    
    if [ "$?" = "0" ]; then
		mkdir /usr/include/proc
		cp proc/*.h /usr/include/proc/
		ln -s /lib/libproc-3.2.8.so /usr/lib/libproc.so
        popd
        rm -rf /tmp/procps-3.2.8
        echo Installed procps Success.
    else
        popd
        echo Installed procps error.
        exit 1
    fi
fi

echo All library installation has been completed.

#end reflib
popd

exit 0