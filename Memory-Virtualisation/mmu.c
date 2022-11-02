#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

# define MB (1024*1024)
# define KB (1024)

// just a random array to be passed to ps_create
unsigned char code_ro_data[10 * MB];

// byte addressable memory
unsigned char RAM[RAM_SIZE];


// OS's memory starts at the beginning of RAM.
// Store the process related info, page tables or other data structures here.
// do not use more than (OS_MEM_SIZE: 72 MB).
unsigned char* OS_MEM = RAM;

// memory that can be used by processes.
// 128 MB size (RAM_SIZE - OS_MEM_SIZE)
unsigned char* PS_MEM = RAM + OS_MEM_SIZE;
# define PS_MEM_SIZE (128 * 1024 * 1024)

// This first frame has frame number 0 and is located at start of RAM(NOT PS_MEM).
// We also include the OS_MEM even though it is not paged. This is
// because the RAM can only be accessed through physical RAM addresses.
// The OS should ensure that it does not map any of the frames, that correspond
// to its memory, to any process's page.
int NUM_FRAMES = ((RAM_SIZE) / PAGE_SIZE);

// Actual number of usable frames by the processes.
int NUM_USABLE_FRAMES = ((RAM_SIZE - OS_MEM_SIZE) / PAGE_SIZE);

// To be set in case of errors.
int error_no;



struct globalOS{
    int phy_list[32768]; 
    struct PCB proc[MAX_PROCS+1]; 
};

void os_init() {

    struct globalOS cpy;
    memcpy( OS_MEM, &cpy, sizeof(struct globalOS));
    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    for(int i = 0 ; i < NUM_USABLE_FRAMES ; i++){
        OS->phy_list[i] = 0; 
                         
    }
    for(int i = 0 ; i < 101 ; i++){
        OS->proc[i].pid = -1;
    }
}





// ----------------------------------- Functions for managing memory --------------------------------- //

/**
 *  Process Virtual Memory layout:
 *  ---------------------- (virt. memory start 0x00)
 *        code
 *  ----------------------
 *     read only data
 *  ----------------------
 *     read / write data
 *  ----------------------
 *        heap
 *  ----------------------
 *        stack
 *  ----------------------  (virt. memory end 0x3fffff)
 *
 *
 *  code            : read + execute only
 *  ro_data         : read only
 *  rw_data         : read + write only
 *  stack           : read + write only
 *  heap            : (protection bits can be different for each heap page)
 *
 *  assume:
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all in bytes
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all multiples of PAGE_SIZE
 *  code_size + ro_data_size + rw_data_size + max_stack_size < PS_VIRTUAL_MEM_SIZE
 *
 *
 *  The rest of memory will be used dynamically for the heap.
 *
 *  This function should create a new process,
 *  allocate code_size + ro_data_size + rw_data_size + max_stack_size amount of physical memory in PS_MEM,
 *  and create the page table for this process. Then it should copy the code and read only data from the
 *  given `unsigned char* code_and_ro_data` into processes' memory.
 *
 *  It should return the pid of the new process.
 *
 */

int find_in_phy_mem(){
    
    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    for(int i = 0 ; i < NUM_USABLE_FRAMES ; i++){
    
        if(OS->phy_list[i] != 1){
            return i;
        }
    }
return -1;
}

int create_ps(int code_size, int ro_data_size, int rw_data_size, int max_stack_size, unsigned char* code_and_ro_data)
{

    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    int arr[1024];
    int final_pid = -1;
    for(int i = 1; i <= 100 ; i++){
        if(OS->proc[i].pid == -1){
            OS->proc[i].pid = i;
            final_pid = i;
            break;
        }
    }
    printf("Entered create_ps for %d\n",final_pid);

    
    page_table_entry pagetable[1024];
    
    int num_pages = (code_size + ro_data_size + rw_data_size + max_stack_size)/(4*KB);
    int code_pages = code_size/(4*KB);
    int ro_pages = ro_data_size/(4*KB);
    int rw_pages = rw_data_size/(4*KB);
    int stack_pages = max_stack_size/(4*KB);

    
    int start_frame = find_in_phy_mem();
  

    int assign_address = start_frame; 

    int index = 0;

     for(int i = 0 ; i < code_pages ; i++){
        start_frame = find_in_phy_mem();
        OS->phy_list[start_frame] = 1;
        start_frame = start_frame << 1 | 1;
        start_frame = start_frame << 3;
        start_frame = start_frame | 1 | 4;
        pagetable[i+index] = start_frame;
    }

    index += code_pages;
    for(int i = 0 ; i < ro_pages ; i++){

        start_frame = find_in_phy_mem();
        OS->phy_list[start_frame] = 1;
        start_frame = start_frame << 1 | 1;
        start_frame = start_frame << 3;
        start_frame = start_frame | 1 ;
        pagetable[i+index] = start_frame;

    }
    index += ro_pages;

    for(int i = 0 ; i < rw_pages ; i++){
        start_frame = find_in_phy_mem();
        OS->phy_list[start_frame] = 1;
        start_frame = start_frame << 1 | 1;
        start_frame = start_frame << 3;
        start_frame = start_frame | 1 | 2;
        pagetable[i+index] = start_frame;

    }
    index += rw_pages;
    for(int i = 0 ; i < stack_pages ; i++){ 
        start_frame = find_in_phy_mem();
        OS->phy_list[start_frame] = 1;
        start_frame = start_frame << 1 | 1;
        start_frame = start_frame << 3;
        start_frame = start_frame | 1 | 2;
        pagetable[1023 - i] = start_frame;

    }


    for(int i = index ; i < 1024 - stack_pages ; i++){
        pagetable[i] = 0;
    }

    memcpy((PS_MEM + assign_address*(4 * KB)), code_and_ro_data, code_size+ro_data_size);

    for(int i = 0 ; i < 1024 ; i++)
    OS->proc[final_pid].page_table[i] = pagetable[i];

    return final_pid;
}

int pte_to_frame_num(page_table_entry pte);


void exit_ps(int pid)
{
   
    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    for(int i = 0 ; i < 1024 ; i++){
        OS->phy_list[pte_to_frame_num(OS->proc[pid].page_table[i])] = 0;
        OS->proc[pid].page_table[i] = 0;
    }
    OS->proc[pid].pid = -1;
  
}

int fork_ps(int pid) {

    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    
    int final_pid = -1;
    for(int i = 1; i <= 100 ; i++){
        if(OS->proc[i].pid == -1){
            OS->proc[i].pid = i;
            final_pid = i;
            break;
        }
    }
    int code_data = 0;
    int ro_data_size = 0;
    
    for(int i = 0 ; i < 1024 ; i++){
        int address = OS->proc[pid].page_table[i];

        if((address >> 1) & 1 && !((address >> 2) & 1) && (address >> 3) & 1){
            OS->proc[final_pid].page_table[i] = OS->proc[pid].page_table[i];
            code_data++;
            continue;
        }
        if((address >> 1) & 1 && !((address >> 2) & 1) && !((address >> 3) & 1)){
            OS->proc[final_pid].page_table[i] = OS->proc[pid].page_table[i];
            ro_data_size++;
            continue;
        }
        break;
    }

    for(int i = code_data + ro_data_size ; i < 1024 ; i++){
        
        if(OS->proc[pid].page_table[i] == 0){
            OS->proc[final_pid].page_table[i] = 0;
            continue;
        }
        
        page_table_entry pte = OS->proc[pid].page_table[i];
        int parent_frame = pte_to_frame_num(pte);
        int phy_frame = find_in_phy_mem();
        
        memcpy(PS_MEM + phy_frame*4*KB, PS_MEM + parent_frame*4*KB, 4*KB);

        phy_frame = phy_frame >> 4;
        if((pte & 1) == 1)
            phy_frame = phy_frame | 1;
        if((pte & 2) == 2)
            phy_frame = phy_frame | 2;
        if((pte & 4) == 4)
            phy_frame = phy_frame | 4;
        if((pte & 8) == 8)
            phy_frame = phy_frame | 8;

        OS->proc[final_pid].page_table[i] = phy_frame;

    }
    return final_pid;
}

void allocate_pages(int pid, int vmem_addr, int num_pages, int flags)
{
   struct globalOS* OS = (struct globalOS*) ( OS_MEM);
   int arr[1024];

   for(int i = 0 ; i < 1024 ; i++){
        arr[i] = OS->proc[pid].page_table[i];

   }

   int start_page = vmem_addr / (4*KB);
   for(int i = start_page ; i < num_pages + start_page ; i++){
        
        if(arr[i] != 0){
            
            exit_ps(pid);
            error_no = ERR_SEG_FAULT;
            return;
        }
   }
   

   int start_frame = find_in_phy_mem(); 
   int assign = start_page;
   for(int i = 0 ; i < num_pages ; i++){
        start_frame = find_in_phy_mem();
        OS->phy_list[start_frame] = 1;
        start_frame = start_frame >> 4;
        start_frame = (start_frame << 1) | 1;
        start_frame = start_frame << 3;
        if(((flags >> 0) & 1) == 1){
            start_frame = start_frame | 1;
        }
        if(((flags >> 1) & 1) == 1){
            start_frame = start_frame | 2;
        }
        if(((flags >> 2) & 1) == 1){
            start_frame = start_frame | 4;
        }
        arr[i+assign] = start_frame;
        
    }



    for(int i = 0 ; i < 1024 ; i++){
        OS->proc[pid].page_table[i] = arr[i];

   }
    
    return ;
}




void deallocate_pages(int pid, int vmem_addr, int num_pages)
{
   
   struct globalOS* OS = (struct globalOS*) ( OS_MEM);
   int arr[1024];
   for(int i = 0 ; i < 1024 ; i++){
        arr[i] = OS->proc[pid].page_table[i];
   }

   int start_page = vmem_addr / (4*KB);
   
   for(int i = start_page ; i < num_pages + start_page ; i++){
        
        if(arr[i] == 0){
            
            exit_ps(pid);
            error_no = ERR_SEG_FAULT;
            return;
        }
   }
   
   int assign = start_page;
   for(int i = 0 ; i < num_pages ; i++){
        OS->phy_list[arr[i+assign] >> 4] = 0;
        arr[i+assign] = 0;
    }
    for(int i = 0 ; i < 1024 ; i++){
        OS->proc[pid].page_table[i] = arr[i];

   }
    
    return ;


}

unsigned char read_mem(int pid, int vmem_addr)
{
    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    int vmem_frame = vmem_addr / (4*KB);
    int current = vmem_addr % (4*KB);

    int arr[1024];
    for(int i = 0 ; i < 1024 ; i++){
        arr[i] = OS->proc[pid].page_table[i];
    }
    if(is_readable(arr[vmem_frame]) == 0){
        error_no = ERR_SEG_FAULT;
        exit_ps(pid);
        unsigned char ch;
        return ch;
    }
    int frame_num = pte_to_frame_num(arr[vmem_frame]);
    

    unsigned char ch = (unsigned char)*(PS_MEM + frame_num*4*KB + current);

    return ch;
}

void write_mem(int pid, int vmem_addr, unsigned char byte)
{
    
    struct globalOS* OS = (struct globalOS*) ( OS_MEM);
    int vmem_frame = vmem_addr / (4*KB);
    int current = vmem_addr % (4*KB);
    int arr[1024];
    for(int i = 0 ; i < 1024 ; i++){
        arr[i] = OS->proc[pid].page_table[i];
    }
    if(is_writeable(arr[vmem_frame]) == 0){
        error_no = ERR_SEG_FAULT;
        exit_ps(pid);
        return ;
    }
    int frame_num = pte_to_frame_num(arr[vmem_frame]);

    *(PS_MEM + frame_num*4*KB + current) = byte;

    return ;
}





// ---------------------- Helper functions for Page table entries ------------------ //


int pte_to_frame_num(page_table_entry pte)
{  
    int bit = (pte>>4);
    return bit;
}

int is_readable(page_table_entry pte) {
    int read = (pte >> 0) & 1;
    return read;

}

int is_writeable(page_table_entry pte) {
    int write = (pte >> 1) & 1;
    return write;

}

int is_executable(page_table_entry pte) {
    int exec = (pte >> 2) & 1;
    return exec;

}


int is_present(page_table_entry pte) {

    int valid = (pte >> 3) & 1;
    return valid;
}

// -------------------  functions to print the state  --------------------------------------------- //

void print_page_table(int pid)
{


    struct globalOS* obj = (struct globalOS*)(OS_MEM);
    page_table_entry page_table_start[1024]; 
    for(int i = 0 ; i < 1024 ; i++){
        page_table_start[i] = obj->proc[pid].page_table[i];
    }

    
    int num_page_table_entries = 1024;          
    
    puts("------ Printing page table-------");
    for (int i = 0; i < num_page_table_entries; i++)
    {

        page_table_entry pte = page_table_start[i];
        printf("Page num: %d, frame num: %d, R:%d, W:%d, X:%d, P%d\n",
                i,
                pte_to_frame_num(pte),
                is_readable(pte),
                is_writeable(pte),
                is_executable(pte),
                is_present(pte)
                );
    }

}








	





