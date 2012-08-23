#!/bin/sh

OPT_MAK="--build=i386-redhat-linux-gnu --host=i386-redhat-linux-gnu --target=i686-redhat-linux-gnu --program-prefix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --sysconfdir=/etc --datadir=/usr/share --includedir=/usr/include --libdir=/usr/lib --libexecdir=/usr/libexec --localstatedir=/var --sharedstatedir=/var/lib --mandir=/usr/share/man --infodir=/usr/share/info --with-libdir=lib --with-config-file-path=/etc --with-config-file-scan-dir=/etc/php.d --with-exec-dir=/usr/bin --disable-debug"

OPT_EXT="--with-pic --disable-rpath --without-pear --with-ldap=shared --with-bz2=shared --enable-zip=shared --with-freetype-dir=/usr --with-png-dir=/usr --with-xpm-dir=/usr --enable-gd-native-ttf --without-gdbm --with-gettext --with-iconv --with-jpeg-dir=/usr --with-imap --with-imap-ssl=/usr/local/openssl --with-openssl=shared,/usr/local/openssl --with-zlib --with-layout=GNU --enable-exif --enable-ftp --enable-magic-quotes --enable-sockets --with-kerberos --enable-ucd-snmp-hack --enable-shmop --enable-calendar --with-sqlite=shared --with-sqlite3=shared --with-xsl=shared,/usr/local/libxslt --with-libxml-dir=/usr --enable-xml=shared --with-xmlrpc=shared --enable-dom=shared --enable-soap=shared --with-gd=shared,/usr/local/gd2 --disable-dba --without-unixODBC --enable-xmlreader=shared --enable-xmlwriter=shared --enable-phar=shared --enable-fileinfo=shared --enable-json=shared --without-pspell --enable-wddx=shared --with-curl=shared,/usr/local/curl --enable-bcmath=shared --with-mcrypt=shared,/usr/local/libmcrypt --with-mhash=shared,/usr/local/mhash --enable-mbstring=shared,all --enable-mbregex=shared --with-mysql=shared --with-mysqli=shared --with-pdo-mysql=shared --with-pdo-sqlite=shared --with-pgsql=shared --with-pdo-pgsql=shared --enable-posix=shared --enable-pcntl=shared --enable-sysvsem=shared --enable-sysvshm=shared --enable-sysvmsg=shared --enable-maintainer-zts --enable-zend-multibyte --enable-inline-optimization"

#OPT_OTH="--with-apxs2=/usr/sbin/apxs"
OPT_OTH="--enable-embed"

PWD=`pwd`
cd $1
./configure ${OPT_MAK} ${OPT_EXT} ${OPT_OTH} && make && make install
cd $PWD

exit $?