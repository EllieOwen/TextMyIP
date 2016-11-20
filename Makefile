ip_monitor: ip_monitor.c
	gcc -lcurl -o ip_monitor ip_monitor.c -I.
	
test: ipify
	@./ip_monitor && prinf "\n\033[0;32msuccess\033[0m\n || prinf "\n \033[0;31mfail\033[0m\n
