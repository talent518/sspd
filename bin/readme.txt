用户以下命令清除共享内存
	ipcs -m | awk '$2 ~/[0-9]+/ {print $2}' | while read s; do ipcrm -m $s; done
