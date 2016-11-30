/* kernel.c - snrkos primary C file
 *
 * Author: Rebecca Krupp (beka.krupp@gmail.com)
 */
void kprint(const char * str);

void kmain(void * multiboot_pointer) {

}

void kprint(const char * str) {
    static char * vga = 0xB8000;

}
