
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <uci.h>


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
  
  lookupValue("awc.api.power_cycle", apiEnabled, sizeof(apiEnabled), uci, &p);
 
  uci_free_context(uci);
  
  if (strcmp(apiEnabled, "enabled") != 0) {
    reportResult("Error - AW12 power cycle API disabled");
  } else {
    reportResult("OK - AW12 power cycle in progress");
    fflush(stdout);
    
    system("/sbin/awc-usb-reset.sh >/dev/null 2>/dev/null || true");
  }
  
  return EXIT_SUCCESS;
}
