本程序在redhat enterprise 6.2下编译成功！
1.首先执行./reflib.sh安装ssp服务支持库；
2.使用make进行编译ssp服务程序；
3.使用make test编译并安装到/tmp/ssp下并启动服务进行测试；
4.使用make retest重新测试应用php代码。

注意：ssp必须的库包括php5,glib(redhat的rpm包或手动安装),pthread,libevent