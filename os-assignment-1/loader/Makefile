#Create libsimpleloader.so from loader.c
Primary: 
	gcc -m32 -shared -o libsimpleloader.so loader.c -fPIC

#Provide the command for cleanup
cleanup:
	-@rm -f libsimpleloader.so
