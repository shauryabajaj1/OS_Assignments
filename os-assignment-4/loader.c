#include "loader.h"
#include <signal.h>

#define PAGE_SIZE 4096


Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
Elf32_Phdr *program_headers;

int fd;
int ehdr_read;
void *v_mem;


int total_pages = 0;
int page_faults = 0;
int fragmentation = 0;

/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
  free(ehdr);
  free(phdr);
  close(fd);
  munmap(v_mem, phdr->p_memsz);

}


void segfault_handler(int signum, siginfo_t *info, void *context) {
    page_faults++;
    void *fault_address = info->si_addr;
    size_t page_size = PAGE_SIZE;
    int fault = (int)fault_address;

      for (int i = 0; i < ehdr->e_phnum; i++) {
          phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
          int phdr_read = read(fd, phdr, sizeof(Elf32_Phdr));
        if (fault >= phdr->p_vaddr && fault < phdr->p_vaddr + phdr->p_memsz) {
            int num_of_pages = (phdr->p_memsz + 4095)/4096;
            void *page_start = (void *)((uintptr_t)info->si_addr & ~(page_size - 1));
            void *seg_memory = mmap(page_start, PAGE_SIZE * num_of_pages, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, phdr->p_offset);
            fragmentation += (num_of_pages * PAGE_SIZE) - phdr->p_memsz;
        }
    }
}

void load_and_run_elf(char **exe) {

  fd = open(exe[1], O_RDONLY);

  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr)); // 1. Load entire binary content into the memory from the ELF file.
  ehdr_read = read(fd, ehdr, sizeof(Elf32_Ehdr));
  read(fd, program_headers, ehdr->e_phentsize * ehdr->e_phnum);
  int eh_ent = ehdr->e_entry;
  int ph_offset = ehdr->e_phoff;
  int ph_size = ehdr->e_phentsize;
  int ph_qty = ehdr->e_phnum;
  lseek(fd, ph_offset, SEEK_SET);

  
  typedef int (*etp_fnc)();
  etp_fnc _start;
  _start = (etp_fnc)eh_ent; // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.

  int result = _start(); // 6. Call the "_start" method and print the value returned from the "_start"
  printf("User _start return value = %d\n", result);
  printf("Total page faults: %d\n", page_faults);
  printf("Fragmentation: %d\n bytes", fragmentation);

}

int main(int argc, char **argv)
{
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = segfault_handler;
  sigaction(SIGSEGV, &sa, NULL);

  if (argc != 2)
  {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }

  
  // 1. carry out necessary checks on the input ELF file
  if(fd == -1 ){
    printf("Error Opening the File!");
  }

  if(ehdr_read == -1){
    printf("Error Reading the File!");
  }

  

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);

  // 3. invoke the cleanup routine inside the loader
  loader_cleanup();
  return 0;
}
