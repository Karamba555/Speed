
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <syslog.h>

#include <uci.h>

#include <json-c/json.h>


static void responseOK(void) {
  printf("Status: 200 OK\r\n");
  printf("Content-type: text/html\r\n\r\n");
  printf("<head><meta http-equiv=\"Refresh\" content=\"0; URL=/\"></head>\n");
}

static void responseError(const char *message) {
  char buffer[256];  
  
  snprintf(buffer, sizeof(buffer), "Nextivity Registration: Internal Error - %s", message);
  
  syslog(LOG_ERR, message);
    
  printf("Status: 400 Internal Error\r\n"); 
  exit(EXIT_FAILURE);
}


static bool setValue(const char *name, const char *value, struct uci_context *uci, struct uci_ptr *p) {
  char buffer[256];
  
  (void)uci;
  (void)p;
  
  snprintf(buffer, sizeof(buffer), "uci set %s=%s", name, value);
  system(buffer);
  system("uci commit");
  
  return true;
  
#if 0  
  /*char *name_copy = strdup(name);
  
  if (uci_lookup_ptr(uci, p, name_copy, true) != UCI_OK || !((*p).o)) {
    return false;
  }
  
  printf("value: %s %s\n", (*p).o->v.string, p->value);
  
  p->value = "yes";
  
  
  p->o         = NULL;
  p->section   = "awc_cloud";
  p->option    = name;
  p->value     = value;
  
//  printf("setValue: %s %s\n", name, value);
  
//  (void)uci;
  
//  bool result = (uci_set(uci, p) == UCI_OK);
  
  //return result;*/
#endif
}


int main(void) {
  struct uci_context *uci;
  struct uci_ptr ucip;
  
  memset(&ucip, 0, sizeof(struct uci_ptr));
    
  uci = uci_alloc_context();
    
  if (!uci) {
    fprintf(stderr, "Failed to allocate UCI context\n");
    EXIT_FAILURE;
  }
  
  char input[16 * 1024];
  
  int result = fread(input, 1, sizeof(input) - 1, stdin);
  
  if (result > 0) {
    input[result] = '\0';
      
  } else {
    responseError("Not enough input data");
  }
  
/*{
    FILE *out = fopen("/tmp/out", "w");
    
    if (out) {
        fwrite(input, 1, result, out);
        fclose(out);
    }
  }*/
  
//  bool gotFirst = false;
//  bool gotLast  = false;
//  bool gotEmail = false;
  
  json_object *userDetails = json_object_new_object();
  
  size_t offset = 0;
  size_t labelStart = 0;
  size_t valueStart = 0;
  
  while (true) {
    char c = input[offset];
    
    if ((c == '%') && input[offset + 1] && input[offset + 2]) {
      char hex[3] = { input[offset + 1], input[offset + 2], '\0' };
    
      c = strtoul(hex, NULL, 16);
      
      memmove(input + offset + 1, input + offset + 3, result - offset); // Include trailing \0
      input[offset] = c;
    }
    
    if (c == '=') {
      input[offset] = '\0';
      
      if (input[offset + 1] == '&') {
        valueStart = offset;
      } else {
        valueStart = ++offset; 
      }
    } else if (c == '&' || c == '\0') {
      input[offset] = '\0';
   
      const char *label = input + labelStart;
      const char *value = input + valueStart;
  
      /*
      char message[128];
      
      snprintf(message, sizeof(message), "label: %s value: %s", label, value);
      syslog(LOG_INFO, message);
      puts(message);
      */
      if (strcmp(label, "sessionid") == 0) {
        // Nothing right now 
      
      } else if (strcmp(label, "first") == 0) {
        //gotFirst = true;
        json_object_object_add(userDetails, "first_name", json_object_new_string(value));
        
      } else if (strcmp(label, "last") == 0) {
        //gotLast = true;
        json_object_object_add(userDetails, "last_name", json_object_new_string(value));
      
      } else if (strcmp(label, "company") == 0) {  
        json_object_object_add(userDetails, "company", json_object_new_string(value));
  
      } else if (strcmp(label, "phone") == 0) {   
        // Bypass check
        
        size_t poffset = 0;
        size_t noffset = 0;
        char number[11];
        
        while (true) {
          char p = value[poffset];
          
          if ((p == '-') || (p == '.')) {
            poffset++; 
            continue;
          }
          
          number[noffset++] = p;
          poffset++;
          
          if ((noffset == sizeof(number)) || (p == '\0')) {
            break; 
          }
        }
        
        printf("number: %s\n", number);
        
        if (strcmp(number, "8588592942") == 0) {
          FILE *eula = fopen("/tmp/nextivity/eula_bypass", "w+");
          
          if (eula) {
            fputs("yes", eula);
            fclose(eula);
            responseOK();
            exit(EXIT_SUCCESS);
            
          } else { 
            responseError("Failed to open eula_bypass file");
          }
        }
       
        json_object_object_add(userDetails, "phone", json_object_new_string(number));
        
      } else if (strcmp(label, "email") == 0) { 
  //      gotEmail = true;
        json_object_object_add(userDetails, "email", json_object_new_string(value));
         
      }
        
      labelStart = ++offset;
    }
    
    if (c == '\0') break;
    
    offset++;
  }
  
  FILE *registration = fopen("/etc/awc/user_registration", "w+");
  
  if (registration) {
     const char *output = json_object_to_json_string(userDetails);
    
//    printf("json: '%s'", json_object_to_json_string(userDetails));
    
    fputs(output, registration); 
    fclose(registration);
    
  } else {
    responseError("Failed to open user_registration file");
  }
  
  if (!setValue("awc_cloud.cloud.eula", "yes", uci, &ucip)) {
    //fprintf(stderr, "Failed to set eula UCI value\n");
    responseError("Failed to fetch eula status");
  }
  
  //puts("set value done");
  
  /*if (uci_commit(uci, NULL, true) != UCI_OK) {
    responseError();
  } */
  
  //puts("commit done");
    
  uci_free_context(uci);
  
  responseOK();
  fflush(stdout);
  
  syslog(LOG_NOTICE, "Nextivity Registration: Immediately polling cloud");
  
  system("/sbin/wakeup-cloud.sh poll &");
   
  return EXIT_SUCCESS;
}
