//C Includes
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


//C++ Includes
#include <fstream>
#include <iostream>
#include <regex>


//Custom Includes

#define DEFAULT_BSIZE 128
#define DEFAULT_TSIZE 512

#define MIN_BSIZE 64
#define MIN_TSIZE 128

#define MAX_BSIZE 2048
#define MAX_TSIZE 4096

#define ALLOW_OVERWRITE 1


struct{
  unsigned int block_size; //4
  unsigned int total_size; //4
  unsigned int used;       //4
  unsigned int unused;     //4
}superblock;



using namespace std;

bool isPowTwo(unsigned int a);

int main(int argc, char** argv){
  unsigned int block_size = DEFAULT_BSIZE;
  unsigned int total_size = DEFAULT_TSIZE;

  int opt = -1;
  opterr = 0;

  while((opt = getopt(argc, argv, "ut:b:")) != -1){
    unsigned int check = 0;
    switch(opt){
    case 'u':
      cout << "This function will allocate a partition in memory to store blocks of data." << endl;
      cout << "To run: ./mk_disk -u -t <unsigned int> -b <unsigned int>" << endl;
      cout << "-u will print the usage string" << endl;
      cout << "-t will indicate the total number of blocks that will make up the created disk. Must be a power of two, and at most " << MAX_TSIZE << endl;
      cout << "-b will indicate the size of each block that will make up the created disk. Must be a power of two, and at most " << MAX_BSIZE << endl;
      cout << "Must contain a filename (alphanumeric) with one '.' to be generated." << endl;
      exit(EXIT_SUCCESS);

    case 't':
      check = atoi(optarg);
      if(check > MAX_TSIZE || check < MIN_TSIZE){
	cerr << "Number of blocks must be between " << MIN_TSIZE << " and " << MAX_TSIZE << endl;
	exit(EXIT_FAILURE);
      }else if(!isPowTwo(check)){
	cerr << "Number of blocks must be a power of two" << endl;
	exit(EXIT_FAILURE);
      }
      total_size = check;
      break;

    case 'b':
      check = atoi(optarg);
      if(check > MAX_BSIZE || check < MIN_BSIZE){
	cerr << "Size of blocks must be between " << MIN_BSIZE << " and " << MAX_BSIZE << endl;
	exit(EXIT_FAILURE);
      }else if(!isPowTwo(check)){
	cerr << "Size of blocks must be a power of two" << endl;
	exit(EXIT_FAILURE);
      }
      block_size = check;
      break;

    case '?':
      if(optopt == 'b' || optopt == 't'){
	cerr << "Option -" << optopt << " requires an argument" << endl;
      }else if(isprint(optopt)){
	cerr << "Unkown option character -" << optopt << endl;
      }else{
	cerr << "Unknown option character" << endl;
      }
      exit(EXIT_FAILURE);
      break;

    default:
      exit(EXIT_FAILURE);
    }
  }

  string filename(argv[optind]);
  regex allowed_naming("([a-zA-Z0-9]+).([a-zA-Z0-9])+");

  if(!regex_match(filename, allowed_naming)){
    cerr << "The name of the file must be alphanumeric containing a '.'" << endl;
    cerr << filename << endl;
    exit(EXIT_FAILURE);
  }

  //Disk should not already exist, may not overwrite previous data
  if(access(filename.c_str(), F_OK) == 0 && !ALLOW_OVERWRITE){
    cerr << "This file already exists" << endl;
    cerr << "Overwriting has been disallowed" << endl;
    exit(EXIT_FAILURE);
  }

  int fh;

  fh = open(filename.c_str(), O_RDWR | O_CREAT, 0666);

  if(fh == -1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  cout << "Disk successfully generated." << endl;

  
  if(ftruncate(fh, total_size * block_size) == -1){
    perror("ftruncate");
    close(fh);
    exit(EXIT_FAILURE);
  }

  
  //Check that filesize is accurate
  struct stat file_status;
  fstat(fh, &file_status);
  if(file_status.st_size != block_size * total_size){
    cerr << "File size does not match input parameters" << endl;
    cerr << "Expected file size: " << block_size * total_size << endl;
    cerr << "Allocated space:  " << file_status.st_size << endl;
    close(fh);
    exit(EXIT_FAILURE);
  }

  
  cout << "Disk has " << block_size * total_size << " in bytes." << endl;
  cout << "Disk is named " << filename << endl;
  cout << "Disk initialized to '\\0' characters in every location" << endl;

  
  //Initializing super block
  superblock.total_size = total_size;
  superblock.block_size = block_size;
  superblock.used = 0;
  superblock.unused = total_size;


  unsigned int write_count = pwrite(fh, &superblock, sizeof(superblock), 0);
  if(write_count == -1){
    perror("pwrite");
    close(fh);
    exit(EXIT_FAILURE);
  }else if(write_count != sizeof(superblock)){
    cerr << "Expected to write " << sizeof(superblock) << " but " << write_count << " were written instead" << endl;
    close(fh);
    exit(EXIT_FAILURE);
  }

  cout << "Superblock initialized and written with the values:" << endl;
  cout << "Total size: " << superblock.total_size << endl;
  cout << "Block size: " << superblock.block_size << endl;
  cout << "Used blocks: " << superblock.used << endl;
  cout << "Unused blocks: " << superblock.unused << endl;
  
  
  close(fh);
    
  
  return 0;
}


bool isPowTwo(unsigned int a){
  return ((a^(a-1)) == ((a * 2)-1))?true:false;
}
