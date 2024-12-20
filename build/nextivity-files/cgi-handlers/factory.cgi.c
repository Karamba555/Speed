
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>

#include <json-c/json.h>

#ifndef AWC_DISPLAY_NAME
#error "AWC_DISPLAY_NAME" not defined
#endif


/**
 *
 * Legacy Factory status page
 *
 * https://192.168.0.1/cgi-bin/factory.cgi?Command=testgetCommonInfo
 *
 * SDKVersion: CPEWT_AW12_00_v1.0.5.bin</br>
 * LTEVersion: Unknown</br>
 * G3Version: Unknown</br>
 * UsimStatus: Unknown</br>
 * WlanMacAddress: </br>
 * W5gMacAddress: </br>
 * WanMacAddress: 34:BA:9A:7B:4C:5C</br>
 * LanMacAddress: 34:BA:9A:7B:4C:5C</br>
 * SSID: ALR-LTE-7B4C5C</br>
 * IMEI: Unknown</br>
 * serialNumber: AT120820A010</br>
 * WAN IP Address: </br>
 * ICCID: </br>
 * MIN: </br>
 * USB: No USB Insert</br>
 * Build_time: 0 SMP Wed Sep 16 08:02:56 2020</br>
 * Port status: 1000:0:0:0:0</br>
 * MCU version: V1</br>
 * LTE AP Version: </br>
 * LTE CP Version: </br>
 * IDUResetDefault: 0</br>
 * IDUBackimage: CPEWT_AW12_00_v0.0.6T1.bin</br>
*/


static void outputValue(const char *name, const char *value, bool unknown) {
  
  if (unknown && ((value == NULL) || (value[0] == '\0'))) {
    value = "Unknown"; 
  }
  
  printf("%s: %s<br>\n", name, value ? value : "");
}


static bool getMacAddress(const char *interface, char *mac) { 
  struct ifreq s;
  bool success = false;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  if (!fd) return false;
  
  strcpy(s.ifr_name, interface);
  if (ioctl(fd, SIOCGIFHWADDR, &s) == 0) {
    bool set = false;

    for (size_t byte = 0; byte < 6; byte++) {
      if (s.ifr_addr.sa_data[byte]) {
         set = true;
         break;
      }
    }

    if (set) {
      for (size_t byte = 0; byte < 6; byte++) {
        snprintf(mac + (byte * 3), 3, "%02x", (unsigned char)s.ifr_addr.sa_data[byte]);
        mac[byte * 3 + 2] = (byte < 5) ? ':' : '\0';
      }
      //printf("MAC: %s\n", mac);
      success = true;
    }
  }

  close(fd);  
  return success;
} 



int main(void) {

  puts("Access-Control-Allow-Origin: *");
  puts("Content-type: text/plain");
  puts("");
  
  const char *awcVersion = "Unknown";
  const char *wanMac = "";
  const char *imei = "";
  const char *iccid = "";
  const char *serialNumber = "";
  const char *wanIP = "";
  
  json_object *modemStatus = json_object_from_file("/tmp/nextivity/modemstatus.json");
  
  if (modemStatus) {
    json_object *status = json_object_object_get(modemStatus, "status");
  
    if (status && (json_object_get_type(status) == json_type_array)) {
      size_t len = json_object_array_length(status);
  
      for (size_t num = 0; num < len; num++) {
        json_object *item = json_object_array_get_idx(status, num);
        
        if (item && (json_object_get_type(item) == json_type_object)) {
          //const char *name = json_object_get_string(item, 
          
          json_object *jname  = json_object_object_get(item, "name");
          json_object *jvalue = json_object_object_get(item, "value");
          
          if (jname && jvalue) {
            const char *name  = json_object_get_string(jname);
            const char *value = json_object_get_string(jvalue);
            
            if (strcmp(name, "SwVersion_s") == 0) {
              awcVersion = value;
              
            } else if (strcmp(name, "ModemMacAddr_s") == 0) {
              wanMac = value;
 
            } else if (strcmp(name, "IMEI_s") == 0) {
              imei = value;
            
            } else if (strcmp(name, "ICCID_s") == 0) {
              iccid = value;
            
           } else if (strcmp(name, "Sn_s") == 0) {
              serialNumber = value;

            } else if (strcmp(name, "PdpIpAddr_s") == 0) {
              wanIP = value;  
              
            }
          }
        }
      }
    }  
  }
    
  char sdkVersion[64];
  
  snprintf(sdkVersion, sizeof(sdkVersion), "CPEWT_AW12_00_v%s.bin", awcVersion /*? awcVersion : "Unknown"*/);
  
  
  outputValue("SDKVersion", sdkVersion, false);
  outputValue("LTEVersion", NULL, true);
  outputValue("G3Version",  NULL, true);
  outputValue("UsimStatus", NULL, true);
  outputValue("WlanMacAddress", NULL, false);
  outputValue("W5GMacAddress",  NULL, false);
  
  outputValue("WanMacAddress", wanMac, false);
  
  char lanMac[24] = "";
  getMacAddress("br-lan", lanMac); 
  outputValue("LanMacAddress", lanMac,  false);
  
  outputValue("SSID", NULL, true);
  
  outputValue("IMEI", imei, true);
  
  outputValue("serialNumber", serialNumber, true);

  outputValue("WAN IP Address", wanIP, false);

  outputValue("ICCID", iccid, false);
  
  outputValue("MIN", NULL, false);
  outputValue("USB", "No USB Insert", false);
  
  const char *buildTime = "Unknown";
  
  FILE *buildFile = fopen("/etc/nextivity_build_info", "r");
  if (buildFile) {
    char buffer[1024];
     
    int result = fread(buffer, 1, sizeof(buffer) - 1, buildFile);
    fclose(buildFile);
     
    if (result > 0) {
      buffer[result] = '\0';
    
      char *label = strstr(buffer, "Build date: ");
      
      if (label) {
        label += 12;
        
        char *end = strchr(label, '\n');
        
        if (end) {
          *end = '\0';
          
          buildTime = label;
        }
      }
    }
  }
  
  outputValue("Build_time", buildTime, false);
  
  char portStatus[32] = "1000:0:0:0:0";
  outputValue("Port status", portStatus, false);
  
  char mcuVersion[8] = "Unknown";
  
  FILE *mcuFile = fopen("/tmp/nextivity/mcu_version", "r");
  if (mcuFile) {
    int result = fread(mcuVersion, 1, sizeof(mcuVersion) - 1, mcuFile);
    
    if (result > 0) mcuVersion[result] = '\0';
    
    fclose(mcuFile); 
  }
  
  outputValue("MCU version", mcuVersion, false);
  
  outputValue("LTE AP Version", NULL, false);
  outputValue("LTE CP Version", NULL, false);
  outputValue("IDUResetDefault", "0", false);
  
  const char *backVersion = sdkVersion;
  outputValue("IDUBackimage", backVersion, false);
  outputValue("FactoryMode", "0", false);

  json_object_put(modemStatus);
  
  return EXIT_SUCCESS;
}
