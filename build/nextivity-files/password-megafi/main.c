
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define passwordLength 10


int main(int argc, const char *argv[]) {
 
  if (argc < 2) exit(1);
  
  const char *serial = argv[1]; 

  if (strlen(serial) < 12) {
    fprintf(stderr, "Must be at least 12 characters\n");
    exit(1);
  }
  
  char buffer[12];
  memset(buffer, 0, sizeof(buffer));
  size_t offset = 0;
  
  int64_t sum = 0;
  
  for (size_t ch = 0; ch < 12; ch++) {
         
    char c = serial[11 - ch];
    //printf("%d %c ", ch, c);
    
    if (c == 'z' || c == 'Z') {
      c = '0';
    } else if (c >= 'A' && c < 'Z') {
      c = c - 'A' + 1;
    } else if (c >= 'a' && c < 'z') {
      c = c - 'a' + 1; 
    } else if (c >= '0' && c <= '9') {  
      c = c - '0';
    } else {
      fprintf(stderr, "Invalid character value %d\n", c);   
    }
    
    int m = c + 1 + (9 * (ch + 1));
    double fm = pow((double)m, M_PI); 
    
    sum += (int64_t)fm;
    
  }
  
  int64_t first  = (int64_t)(sum * 342);
  int64_t second = (int64_t)(sum * 525);
  
  
  double values[12];
  
  values[0] = sqrt(first);
  values[1] = first / M_PI;
  values[2] = log(first) / log(3);
  values[3] = first * 57.8585;
  values[4] = first * 384.62;
  values[5] = first / 1.65644;
  values[6] = cbrt(second);
  values[7] = second  / 2.424242;
  values[8] = second * M_PI;
  values[9] = (int)second;
  values[10] = log2(second);
  values[11] = second / M_PI;


  for (size_t pos = 0; pos < 12; pos++) {

    if (pos == 7 || pos == 9) continue;
       
    //double value = values[pos];
     
    int64_t value = (int64_t)values[pos];
    
    //printf("Value: %lld\n", value);
    
    int64_t m3 = (int64_t)((double)value * 3.87644);
    int64_t ms = (int64_t)sqrt(m3);
    ms = (int64_t)((double)ms + 12.8);
    int64_t mp = (int64_t) pow(ms, 2);
    
    // First two digitis
/*    char dbuffer[64];
    snprintf(dbuffer, sizeof(dbuffer), "%0.10f",  mp);
    dbuffer[2] = 0;
    
    int64_t num = atoi(dbuffer) % 26;*/

    int64_t l2 = mp % 100;
    int64_t num = l2 % 26;
    
//    printf("dbuffer: %s\n", dbuffer);
    
    //int64_t num = (int64_t)l2 % 26;

    //printf("m3: %f mp: %f l2: %lld num: %02lld\n", m3, mp, (int64_t)l2, num);
    
    char c;
    
    if (pos <= 2) {
      // Upper
        
      if (num == 0) {
        c = 'Z';
      } else {
        c = num + 'A' - 1;
      }
 
    } else if (pos == 3 || pos == 4) {
      // Lower
        
      if (num == 0) {
        c = 'z';
      } else {
        c = num + 'a' - 1;
      }
 
    } else {
      // First digit
      c = ((int)l2 / 10) + '0';  
      //c = dbuffer[0];
    }

    buffer[offset++] = c;
  }
  
  printf("%s", buffer);
  
  return 0;  
}
