
scache: scache.c
	gcc -o scache-temp scache.c
	sudo cp scache-temp scache
	rm scache-temp
	sudo chmod 4511 scache
