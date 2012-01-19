/* Find self-locating numbers in numbers, esp. in Pi.
 *
 *  Last Modified: Thu 19 Jan 2012 08:55:04 PM
 *  Tweaked/forked by @th507
 *
 * Copyright (c) Yongzhi Pan, Since Dec 7 2011
 *
 * License is GPL v3
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
// for uint64_t
#include <inttypes.h>

/* my notebook is not running 64bit OS :(
#if LONG_MAX <= 10000000
#error Your compiler`s long int type is too small!
#endif
*/

// have to be larger than LOOP
#define CACHESIZE 10000

// Number of bytes to bypass. 
// in order to make 1 the 1st digit of pi (3.1415...), we set OFFSET to 1.
// This is DIFFERENT from Peter's definition!
#define OFFSET 1
#define err_exit_en(en, msg) \
  do { errno=en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define err_exit(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while(0)

int fast_log10_minus1 (long v) {
  // needs to expand for larger cases
  return (v >= 10000000000) ? 10 : (v >= 1000000000) ? 9 : 
    (v >= 100000000) ? 8 : (v >= 10000000) ? 7 : (v >= 1000000) ? 6 : 
    (v >= 100000) ? 5 : (v >= 10000) ? 4 : (v >= 1000) ? 3 : 
    (v >= 100) ? 2 : (v >= 10) ? 1 : 0;
}

// replacing mod_litoa
void ascendingIntToString(char * t, int i, int len) {
  if (i > len + 1 || t[i] < 48) {
    t[i] = 49;
    return;    
  }
  
  if (t[i] == 57) {
    t[i] = 48;
    ascendingIntToString(t, i + 1, len);
  }
  else
    t[i] += 1;
}

// improved Brutal Force search
// skip string search if index digit mismatches.
// pattern is a reversed string
void searchByIndex(char * p, int m, char * t) {
  int i, j = 10, k = m - 1, l;
  while (j) {
    j--;
    if (t[m + j] - 48 != j) continue;
    for (i = 0; i <= m && p[k - i] == t[i + j]; ++i);
    if (i >= m) {
      for (l = k; l >= 0; l--)
        printf("%c", p[l]);
      printf("%d\n", j);      
    }
  }
}


static void * find(void * file) {
  char * infile = (char *)file;
  struct stat stat_buf;
  int fd;
  off_t N;
  //register off_t n = 1;
  int maxlen;

  // get file size
  stat(infile, &stat_buf);
  N = stat_buf.st_size;

  maxlen = fast_log10_minus1(N) + 1;

  // open pi data file
  fd = open(infile, O_RDONLY);
  // file won't open
  if (fd == -1) err_exit("open");

  // according to 
  // echo "" | gcc -E -dM -c - | grep linux
  // we should check for __linux, __linux__, __gnu_linux__, linux
  // we only cover the common case
  #ifdef __linux__
  int s;  
  s = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  if(s != 0)
    err_exit_en(s, "posix_fadvise");
  #endif 

  // failed to perform 'lseek'
  if (lseek(fd, OFFSET, SEEK_SET) == (off_t)-1)
    err_exit("lseek");

  // this works fine on Mac but crash on Linux
  char * cache = (char *)malloc(CACHESIZE + maxlen + 1);
  char * numberString = (char *)malloc(maxlen + 1);

  int j, len = 0;

  *numberString = 48; // '0'

  uint64_t n = 0;

  while (n < N) { // buf never gets reset, maybe it OUGHT TO
    lseek(fd, (n + OFFSET), SEEK_SET);
    read(fd, cache, len + CACHESIZE);// read to a larger cache

    len = fast_log10_minus1(n);

    // find the the string except the last digit
    j = 0;
    while (j < CACHESIZE) {
      // for the first CACHESIZE loop
      if (!n)
        len = fast_log10_minus1(j);

      // search through cache[j] to cache[j+len+10]
      searchByIndex(numberString, len, cache + j);

      j += 10;

      // update the numberString (index/offset except the last digit)
      // we do NOT reverse the string, nor do we append '\0'
      ascendingIntToString(numberString, 0, len);
    }
    n += CACHESIZE;
  }

  free(cache);
  free(numberString);

  return NULL;
}

int main(int argc, char *argv[]) {
  int num_cpu = 1;//sysconf(_SC_NPROCESSORS_ONLN);
  if (num_cpu == -1)
    err_exit("sysconf"); 
  printf("CPU number: %d\n", num_cpu);
  //char *files[num_cpu];
  const char * const files[2] = {"../../../pi-billion.txt", ""};
  pthread_t tid[num_cpu];
  int i;
  //int j;
  int s;

  if(num_cpu == 2) {
    // FIXME initialize once
    // FIXME get file names from argv
    //files[0] = "xaa";
    //files[1] = "xab";
    ;
  }

  for(i = 0; i < num_cpu; i++) {
    s = pthread_create(&tid[i], NULL, find, (void *)files[i]);
    if(s != 0)
      err_exit_en(s, "pthread_create");
  }

  /* now wait for all threads to terminate */
  for(i = 0; i < num_cpu; i++) {
    s = pthread_join(tid[i], NULL);
    if(s != 0)
      err_exit_en(s, "pthread_join");
  }

  printf("\n");
  return 0;
}
