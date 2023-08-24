export LD_LIBRARY_PATH := ../loader:$(LD_LIBRARY_PATH)

#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
Primary:
	$(MAKE) -C loader
	$(MAKE) -C test
	$(MAKE) -C launcher
	gcc -m32 -o bin/launch launcher/launch.c -Lloader -lsimpleloader
	@echo 
	@echo "    ***Success***"
	@echo

#Provide the command for cleanup
cleanup:
	$(MAKE) -C loader cleanup
	$(MAKE) -C test cleanup
	$(MAKE) -C launcher cleanup
	@echo 
	@echo "    ***Success***"
	@echo
	-@rm -f bin/*
