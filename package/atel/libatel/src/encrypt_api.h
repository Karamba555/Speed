/*************************************************************************
	> File Name: encrypt_api.h
	> Author: 
	> Mail: 
	> Created Time: Fri Jan  3 10:22:15 2020
 ************************************************************************/

#ifndef _ENCRYPT_API_H
#define _ENCRYPT_API_H

typedef enum wifi_type {
	MAIN_WIFI,
	GUEST_WIFI
};

typedef struct expand_para {
	int type;
}Epara, *pEpara;

int GenerateSaltByTime(char salt[128]);
int AtelDigest(char *plainText, char *cipherText);
int EncryptPasswordWithSalt(const char *nvramName, const char *salt, const char *password);
int lib_get_string_in_file(char *file_name,	char *match_string,	char *match_end, char *output);

char *GetUUIDOnce(char para[64]);
char *CheckUUIDFromManufature(char para[64]);
char *CheckUUID(char para[64]);
char *GenerateUniqueWifiPassword(char password[16], const char atel_hwid[64], pEpara arg);
char *GenerateUniqueLoginPassword(char password[16], const char atel_hwid[64]);

extern int PackageShake256(char *plainText, char *cipherText);
extern char *GetHWIDbyUUID(const char atel_hwid[64], char hwid[32]);

#endif
