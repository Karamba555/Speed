
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <time.h>

#include <uci.h>


#define modemStatus "/tmp/nextivity/modemstatus.json"

static bool lookupValue(const char *name, char *value, size_t valueLen, struct uci_context *uci, struct uci_ptr *p) {
  bool result = true;
  char *name_copy = strdup(name);
  
  if (!name) {
    fprintf(stderr, "Value strdup failure\n");   
    return false;
  }
          
  if (uci_lookup_ptr(uci, p, name_copy, true) != UCI_OK || !((*p).o)) {
    result = false;
    fprintf(stderr, "Lookup for name: '%s' failed\n", name);
  } else {
    //printf("Lookup for name: '%s' = '%s'\n", name, (*p).o->v.string);
      
    strncpy(value, (*p).o->v.string, valueLen - 1);  
  }
    
  free(name_copy);
  return result;
}



int main(void) {
  struct uci_context *uci;
  struct uci_ptr p;
  
  memset(&p, 0, sizeof(struct uci_ptr));
    
  uci = uci_alloc_context();
    
  if (!uci) {
    fprintf(stderr, "Failed to allocate UCI context\n");
    EXIT_FAILURE;
  }

  char apiEnabled[16];
  char periodS[16];
  
  lookupValue("awc.api.status", apiEnabled, sizeof(apiEnabled), uci, &p);
  lookupValue("awc.status_gather.period", periodS, sizeof(periodS), uci, &p);

  uci_free_context(uci);
  
  printf("Access-Control-Allow-Origin: *\n");
  printf("Content-type: application/json\n");
  printf("Cache-Control: no-cache\n");
  printf("\n");
  
  if (strcmp(apiEnabled, "enabled") != 0) {
    printf("{\"status\":\"Error - Modem Status API disabled\"}");
    exit(EXIT_SUCCESS);
  }

#if 0
  size_t period = atoi(periodS);
  
  // Check not too old
  if (period == 0) period = 60;
  
  // Now longer than 1.5x the date
  period = (period * 150) / 100;
#endif  
  
  struct stat stbuf; 
  time_t reportTime = 0 ;//time(NULL);
  
  if (!stat(modemStatus, &stbuf)) {
    FILE *status = fopen(modemStatus, "r");
    
    if (status) {
       char buffer[32 * 1024];
      int result = fread(buffer, 1, sizeof(buffer), status);
      fclose(status);
      
      if (result > 0) {
//        printf("result: %d\n", result);
        
        buffer[result] = '\0';
        const char *jtime = strstr(buffer, "\"time\":");
        
        if (jtime) {        
          jtime += 7;
          
          if (jtime[0] == ' ') jtime++;
        
          reportTime = atoi(jtime);
        
          if (reportTime > 0) {
  //          time_t now = time(NULL);  
              
//            int diff = now - reportTime;
          
  //          printf("diff: %d\n", diff);
          
//            if (diff <= (int)period) {
              fwrite(buffer, 1, result, stdout);
              return EXIT_SUCCESS;
//            }
          }
        }
      }
    }
  }
    
  printf("{\"time\":%lu,\"status\":[]}", reportTime);
  
  return EXIT_SUCCESS;
}
