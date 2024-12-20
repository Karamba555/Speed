#ifndef __UTILS_H__
#define __UTILS_H__

#if defined(CONFIG_USER_SECURITY)
#define __system		system_with_root_uid
#else
#define __system		system
#endif

enum process_exist {
	Non_Exist,
	Exist
};

int system_with_root_uid(char *cmd);
int check_process_exist(char *process);
void init_file_privilege(char *mode, char *file);
char *get_wan_interface();

#endif
