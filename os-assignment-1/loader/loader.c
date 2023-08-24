#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;

int fd;
int ehdr_read;
void *v_mem;

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

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe)
{
  fd = open(exe[1], O_RDONLY);

  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr)); // 1. Load entire binary content into the memory from the ELF file.
  ehdr_read = read(fd, ehdr, sizeof(Elf32_Ehdr));

  int eh_ent = ehdr->e_entry;
  int ph_offset = ehdr->e_phoff;
  int ph_size = ehdr->e_phentsize;
  int ph_qty = ehdr->e_phnum;
  lseek(fd, ph_offset, SEEK_SET);

  for (int i = 0; i < ph_qty; i++) // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  {
    phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
    int phdr_read = read(fd, phdr, sizeof(Elf32_Phdr));

    if (phdr->p_type == PT_LOAD && eh_ent >= phdr->p_vaddr && eh_ent < phdr->p_vaddr + phdr->p_memsz)
    { // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
      v_mem = mmap(NULL, phdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, phdr->p_offset);

      int new_entp = eh_ent - phdr->p_vaddr;
      void *new_adr = v_mem + new_entp; // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step

      typedef int (*etp_fnc)();
      etp_fnc _start;
      _start = (etp_fnc)new_adr; // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.

      int result = _start(); // 6. Call the "_start" method and print the value returned from the "_start"
      printf("User _start return value = %d\n", result);
    }

    lseek(fd, ph_offset + (ph_size) * (i + 1), SEEK_SET);
  }
}
