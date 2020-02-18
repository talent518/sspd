# sspd
本程序在ubuntu 16.04/centos 7.6.1810下编译成功！
### 编译说明
1. 首先执行./reflib.sh安装ssp服务支持库；
2. 使用make进行编译ssp服务程序；
3. 使用make test编译并安装到/tmp/ssp下并启动服务进行测试；
4. 使用make retest重新测试应用php代码；
5. 如果使用./reflib-lampp.sh编译支持库，并且出现"/opt/ssp/lib/libphp5.so: undefined reference to `SSLv2_client_method@OPENSSL_1.0.1'"样的错误信息，就在Makefile中的第4行的末尾添加"-Wl,-rpath,/opt/lampp/lib"，即可（不包括引号"）。
6. 压力测试make bench,make bench2两种方式，默认连接是make retest启动的端口号8086
7. 使用make retest3启动多ssp服务，并使用make bench3进行压力测试

**注意：** ssp必须的库包括php5,glib(redhat的rpm包或手动安装),pthread,libevent
