
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <uci.h>

#ifndef AWC_DISPLAY_NAME
#error "AWC_DISPLAY_NAME" not defined
#endif


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


static void reportResult(const char *status) {
  printf("Content-type: text/html\n");
  printf("\n");
  printf("{\"status\":\"%s\"}", status);
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
  
  lookupValue("awc.api.reboot", apiEnabled, sizeof(apiEnabled), uci, &p);
 
  uci_free_context(uci);
  
  if (strcmp(apiEnabled, "enabled") != 0) {
    reportResult("Error - " AWC_DISPLAY_NAME " reboot API disabled");
  } else {
    reportResult("OK - " AWC_DISPLAY_NAME " reboot in progress");
    
    execl("/sbin/reboot", "reboot", NULL);
  }
  
  return EXIT_SUCCESS;
}
