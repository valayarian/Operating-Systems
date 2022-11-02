#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITS_ADDRESS 32
//size of one frame is 4KB give. Hence, 12 bits in offset
#define BITS_OFFSET 12
const int BITS_VPN = 20;

int verb, no_of_frames,no_of_hits=0, no_of_miss=0, no_of_writes=0, no_of_drops=0;
int linescount = 0;
int *array;

struct PTE{
    int virtual_pn;
    int dirty;
    int valid;
    int used_at;
    int used;
} pt_entry;

int do_random(){
	return rand()%(no_of_frames);
}

int do_lru(struct PTE frame_list[], int curr){
	int swap_index = 0;
	int length = curr - frame_list[0].used_at; 
	for(int i=1; i<no_of_frames; i++){
		int curr_distance = curr-frame_list[i].used_at;
		if(curr_distance > length){
			swap_index = i;
			length = curr_distance;
		}
	}
	return swap_index;
}

int do_clock(struct PTE frame_list[], int cp){
	for(int i=0 ;i<no_of_frames; i++){
		int k = (i + cp)%no_of_frames;
		if(frame_list[k].used == 1){
			frame_list[k].used = 0;
		}
		else{
			frame_list[k].used = 1;
			return (k);
		}
	}
	return cp;
}

int do_opt(struct PTE frame_list[], int curr, int address_file[], int linescount){
	int A[no_of_frames];
	for(int i=0; i<no_of_frames; i++){
		A[i] = -1;
	}
	for(int i=0; i<no_of_frames; i++){
		int j = curr;
		while(j<linescount){
            if(address_file[j]==frame_list[i].virtual_pn ){
                A[i]=j;
                break;
            }
            j=j+1;
		}
        if(A[i]==-1){
            return i;
        }
	}
	int si = 0;
	for(int i=1; i<no_of_frames; i++){
		if(A[i]>A[si]) si = i;
	}
	return si;
}

void handle_verbose(struct PTE frame_list[], int index, int VPN) {
  if (verb==0) return;
  if(frame_list[index].dirty==1){
  	printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n",VPN,frame_list[index].virtual_pn);
  }
  else if(frame_list[index].dirty==0){
  	printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n",VPN,frame_list[index].virtual_pn);
  }
  return;

}

void printfunc(int no_of_hits, int no_of_writes, int no_of_miss, int no_of_drops){
    printf("Number of memory accesses: %d\n",no_of_hits+no_of_miss);
	printf("Number of misses: %d\n",no_of_miss);
	printf("Number of writes: %d\n",no_of_writes);
	printf("Number of drops: %d\n",no_of_drops);
}


int cp = 0;
void Policy(struct PTE frame_list[], char tf[], char* policy){
  

  FILE *trace_file = fopen(tf,"r");

  char access_type;
  unsigned virtual_addr;

  int index = 0;
  int curr = 0;
  
  
  while (fscanf(trace_file, "%x %c", &virtual_addr, &access_type)!=EOF) {
  	curr = curr + 1;
  	int i = 0;
  	char opern;
  	opern = access_type;
  	int hit = 0;
  	int VPN = virtual_addr>>12;

    for(i=0; i<no_of_frames; i++){
     	if(frame_list[i].valid==1 && frame_list[i].virtual_pn==VPN){
     		hit = 1;
     		no_of_hits = no_of_hits + 1;
     		break;
     	}
    }

    if(hit==1){
     	if(opern=='W'){
     		frame_list[i].dirty = 1;
     	}
     	frame_list[i].used_at = curr;
     	frame_list[i].used = 1;
    }
    else{
	     	no_of_miss = no_of_miss + 1;
	     	if(frame_list[index].valid == 1){
	     		if(strcmp(policy,"RANDOM")==0){
	     			index = do_random();
	     		}
	     		else if(strcmp(policy,"LRU")==0){
	     			index = do_lru(frame_list,curr);
	     		}
	     		else if(strcmp(policy,"CLOCK")==0){
	     			index = do_clock(frame_list, cp);
	     			cp = (index+1)%(no_of_frames);
	     		}
	     		else if(strcmp(policy, "OPT")==0){
	     			index = do_opt(frame_list,curr,array,linescount);
	     		}
	     		handle_verbose(frame_list, index, VPN);
	     		if(frame_list[index].dirty==1){
	     			no_of_writes = no_of_writes + 1;
	     		} 
	     		else if(frame_list[index].dirty==0){
	     			no_of_drops = no_of_drops + 1;
	     		} 
	     	}

	     	frame_list[index].valid = 1;
	     	frame_list[index].virtual_pn = VPN;
	     	if(opern=='W'){
	     		frame_list[index].dirty = 1;
	     	}
	     	else{
	     		frame_list[index].dirty = 0;
	     	} 
	        frame_list[index].used_at=curr;
            frame_list[index].used=1;
            index = (index+1)%(no_of_frames);
        }


  }

  printfunc(no_of_hits,no_of_writes,no_of_miss,no_of_drops);
}





int main(int argc, char *argv[]){

	//no_of_frames = atoi(argv[2]);
  char *end;
  no_of_frames = strtol(argv[2], &end, 10);

	if(argc == 5){
		if(strcmp(argv[4],"-verbose")==0){
			verb = 1;
		}
		else verb = 0;
	}

    struct PTE frame_list[no_of_frames];
    
    for(int i=0;i<no_of_frames;i++){
        frame_list[i].valid=0;
	    frame_list[i].used=1;
    }


	char *policy = argv[3];
    char *tf    = argv[1];


    if(strcmp(policy,"OPT")==0){
    	    FILE * f1;
    		f1 = fopen(tf,"r");
			char ch = fgetc(f1);
			linescount = 0;
			while(ch!=EOF){
				if(ch=='\n'){
					linescount = linescount + 1;
				} 
				ch = fgetc(f1);
			}
			array = malloc((linescount+2)*sizeof(int));

			FILE *f2 = fopen(tf,"r");
			int i = 0;
	        char at;
            unsigned va;
			while(fscanf(f2, "%x %c", &va, &at)!=EOF){
				array[i]= va >> 12;
				i = i+1;
			}

    }   
    srand(5635);
	Policy(frame_list,tf,policy);
	return 0;

}
