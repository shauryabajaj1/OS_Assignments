#include "../loader/loader.h"

int main(int argc, char **argv)
{
  int fd;
  int ehdr_read;
  Elf32_Ehdr *ehdr;
  fd = open(argv[1], O_RDONLY);
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  ehdr_read = read(fd, ehdr, sizeof(Elf32_Ehdr));


  if (argc != 2)
  {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }

  // 1. carry out necessary checks on the input ELF file
  if(fd == -1 ){
    printf("Error Opening the File!");
  }
  else if(ehdr_read == -1){
    printf("Error Reading the File!");
  }
  else{
    close(fd);
  }

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);

  // 3. invoke the cleanup routine inside the loader
  loader_cleanup();
  return 0;
}
