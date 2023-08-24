#Compile the launch.c by linking it with the libsimpleloader.so
Primary:
	gcc -m32 -o launcher launch.c -L../loader -lsimpleloader
	export LD_LIBRARY_PATH=../loader:$$LD_LIBRARY_PATH
	./launcher ../test/fib

#Provide the command for cleanup
cleanup:
	-@rm -f launcher