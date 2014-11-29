## Compile

Change directory to src, then execute cmd:

	```
	gcc -I../common -I. ../common/*.c *.c -DLINUX -lpthread
	```

## Note

* Sample is in main.c.
* Only support Linux now.
