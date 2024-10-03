#include <lib/debug.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

/**
 * The initialization function for the allocation table AT.
 * It contains two major parts:
 * 1. Calculate the actual physical memory of the machine, and sets the number
 *    of physical pages (NUM_PAGES).
 * 2. Initializes the physical allocation table (AT) implemented in the MATIntro layer
 *    based on the information available in the physical memory map table.
 *    Review import.h in the current directory for the list of available
 *    getter and setter functions.
 */
void pmem_init(unsigned int mbi_addr)
{
    unsigned int nps;

    // TODO: Define your local variables here.

    unsigned int table_rows; 
    unsigned int last_row_start_addr;
    unsigned int last_row_size;
    unsigned int last_row_end_addr;

    unsigned int i;
    unsigned int start_addr;
    unsigned int range_len;
    unsigned int permission;
    unsigned int page_index;

    // Calls the lower layer initialization primitive.
    // The parameter mbi_addr should not be used in the further code.
    devinit(mbi_addr);

    /**
     * Calculate the total number of physical pages provided by the hardware and
     * store it into the local variable nps.
     * Hint: Think of it as the highest address in the ranges of the memory map table,
     *       divided by the page size.
     */
    // TODO
    
    table_rows = get_size(); 

    if(table_rows == 0) {
        nps = 0;
    } else {
        last_row_start_addr = get_mms(table_rows - 1); 
        last_row_size = get_mml(table_rows - 1);
        last_row_end_addr = last_row_start_addr + last_row_size - 1;
        nps = (last_row_end_addr + 1) / PAGESIZE;
    }

    set_nps(nps); 

    /**
     * Initialization of the physical allocation table (AT).
     *
     * In CertiKOS, all addresses < VM_USERLO or >= VM_USERHI are reserved by the kernel.
     * That corresponds to the physical pages from 0 to VM_USERLO_PI - 1,
     * and from VM_USERHI_PI to NUM_PAGES - 1.
     * The rest of the pages that correspond to addresses [VM_USERLO, VM_USERHI)
     * can be used freely ONLY IF the entire page falls into one of the ranges in
     * the memory map table with the permission marked as usable.
     *
     * Hint:
     * 1. You have to initialize AT for all the page indices from 0 to NPS - 1.
     * 2. For the pages that are reserved by the kernel, simply set its permission to 1.
     *    Recall that the setter at_set_perm also marks the page as unallocated.
     *    Thus, you don't have to call another function to set the allocation flag.
     * 3. For the rest of the pages, explore the memory map table to set its permission
     *    accordingly. The permission should be set to 2 only if there is a range
     *    containing the entire page that is marked as available in the memory map table.
     *    Otherwise, it should be set to 0. Note that the ranges in the memory map are
     *    not aligned by pages, so it may be possible that for some pages, only some of
     *    the addresses are in a usable range. Currently, we do not utilize partial pages,
     *    so in that case, you should consider those pages as unavailable.
     */
    // TODO

    for(i = 0; i < VM_USERLO_PI; i++) {
        at_set_perm(i, 1);  // Kernel reserved pages
    }
    for(i = VM_USERHI_PI; i < nps; i++) {
        at_set_perm(i, 1);  // Kernel reserved pages
    }

    for(i = VM_USERLO_PI; i < VM_USERHI_PI; i++) {
        at_set_perm(i, 0);  // Initialized as unavailable by default
    }

    for(i = 0; i < table_rows; i++) {
        start_addr = get_mms(i);
        range_len = get_mml(i);
        
        if(is_usable(i) == 1) permission = 2;
        else permission = 0;

        page_index = start_addr / PAGESIZE;
        if(page_index * PAGESIZE < start_addr) page_index++;
        
        while((page_index + 1) * PAGESIZE <= start_addr + range_len) {
            if(page_index >= VM_USERLO_PI && page_index < VM_USERHI_PI) {
                at_set_perm(page_index, permission);
            }
            
            page_index++;
            if(page_index >= VM_USERHI_PI) break;
        }
    }
}