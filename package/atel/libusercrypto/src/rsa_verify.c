
#include "rsa.h"
#include "vb21_struct.h"
#include "rwsig.h"

 
/* Public key */

unsigned char key_vbpubk2[] = {
  0x56, 0x62, 0x32, 0x50, 0x03, 0x00, 0x00, 0x00, 0x40, 0x04, 0x00, 0x00,
  0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
  0x08, 0x04, 0x00, 0x00, 0x04, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,
  0xc5, 0xf0, 0x6c, 0x0b, 0xd7, 0x7b, 0x32, 0xb5, 0xf9, 0xcd, 0x22, 0x1b,
  0xba, 0x70, 0x91, 0x19, 0xd4, 0x00, 0x76, 0xe7, 0x80, 0x00, 0x00, 0x00,
  0x69, 0x85, 0x14, 0x96, 0x27, 0xa5, 0x6b, 0x36, 0x72, 0x7b, 0x6f, 0x26,
  0x0d, 0x53, 0x38, 0x7c, 0x2c, 0x39, 0xfa, 0x4a, 0x48, 0x69, 0x22, 0x9b,
  0x52, 0xa7, 0xfa, 0xb6, 0xbf, 0x46, 0xe4, 0x56, 0xaf, 0x4c, 0x34, 0xb6,
  0xa9, 0x60, 0x12, 0xe9, 0x74, 0xdf, 0xab, 0x81, 0x3a, 0x63, 0x40, 0xfd,
  0x90, 0x87, 0x64, 0x35, 0x8b, 0x8a, 0xb3, 0x3d, 0x35, 0xcb, 0xbd, 0xbd,
  0x2a, 0xb9, 0x85, 0xfe, 0xdb, 0x22, 0xee, 0x1e, 0x08, 0x1e, 0x77, 0x22,
  0x97, 0x06, 0x9a, 0xab, 0x11, 0x21, 0x09, 0xed, 0xba, 0x07, 0xc6, 0xc1,
  0xc3, 0xf4, 0x56, 0x57, 0xf8, 0x26, 0xb5, 0xba, 0x2f, 0x0c, 0xb4, 0x84,
  0x86, 0x20, 0x97, 0x11, 0x3b, 0x76, 0x6d, 0x4e, 0xf5, 0x00, 0x90, 0x07,
  0xeb, 0x8b, 0xbb, 0x0a, 0xa7, 0x41, 0xc7, 0xc6, 0xc3, 0xcf, 0x50, 0x5c,
  0x90, 0x94, 0xb2, 0x35, 0x76, 0x7e, 0x38, 0xf4, 0x03, 0xd3, 0xb6, 0x8d,
  0xe1, 0xc1, 0xd1, 0x0e, 0x7f, 0xf2, 0x68, 0xfc, 0xcf, 0x6c, 0x4e, 0x54,
  0x8a, 0xdd, 0xf0, 0xc8, 0x35, 0xcf, 0xde, 0x4e, 0x4e, 0x7b, 0xcd, 0xa5,
  0x9a, 0x7b, 0xb4, 0x48, 0x33, 0xb8, 0x95, 0x9c, 0x19, 0xed, 0x08, 0x78,
  0x08, 0x51, 0xb5, 0xae, 0xea, 0xaa, 0xb1, 0xdb, 0x87, 0x3d, 0x65, 0x70,
  0x4d, 0x4c, 0x81, 0x94, 0x9f, 0x5d, 0x9d, 0xf6, 0x7b, 0x3e, 0x08, 0x7a,
  0xd7, 0x50, 0x4c, 0x85, 0x20, 0x44, 0x50, 0xb5, 0x8d, 0x5f, 0x8a, 0x42,
  0xdf, 0x17, 0x2a, 0x4e, 0x40, 0xcc, 0xeb, 0xb3, 0x63, 0xe0, 0xe5, 0xf6,
  0xa7, 0x1f, 0x98, 0x53, 0x11, 0x9c, 0x08, 0x4d, 0xf0, 0xbc, 0xd6, 0x9d,
  0x79, 0xa8, 0x12, 0xef, 0xe6, 0x7c, 0x2e, 0xe5, 0x3b, 0x15, 0xcf, 0xf7,
  0xe8, 0x72, 0x84, 0x1f, 0x4b, 0x99, 0x41, 0x5c, 0x60, 0xb6, 0xed, 0xe3,
  0xc0, 0x3e, 0x25, 0x13, 0x4c, 0x18, 0xb5, 0x1c, 0x17, 0xc0, 0x2d, 0xfd,
  0xc9, 0x26, 0x91, 0xe5, 0x2e, 0xda, 0x90, 0x44, 0x3f, 0x0a, 0xe4, 0x8c,
  0x20, 0x64, 0xa8, 0x6f, 0x98, 0x62, 0x83, 0xb4, 0x10, 0xe3, 0xf6, 0x96,
  0x6c, 0x1f, 0xa1, 0x43, 0x76, 0x64, 0x34, 0xd9, 0xa5, 0x36, 0x14, 0xe1,
  0xd5, 0x4c, 0x9e, 0x7b, 0xfa, 0xa6, 0x7d, 0x8a, 0xef, 0x49, 0xff, 0x00,
  0x15, 0x31, 0xe4, 0x3b, 0x74, 0xe5, 0x49, 0x4c, 0xce, 0x1b, 0x0a, 0xc4,
  0x6a, 0x5c, 0x5a, 0x1d, 0xa8, 0x1f, 0x45, 0xc1, 0xb4, 0x28, 0xbd, 0x07,
  0x56, 0xcb, 0x41, 0x16, 0x19, 0x76, 0xa3, 0x91, 0x7a, 0x91, 0xe7, 0x1f,
  0xb9, 0xa6, 0x01, 0x64, 0xdd, 0x02, 0xca, 0x1b, 0x15, 0xf2, 0x80, 0xac,
  0x3a, 0x06, 0xfe, 0xa5, 0x11, 0x13, 0xd3, 0x31, 0x07, 0xed, 0xe0, 0x1d,
  0xa7, 0x4d, 0xb1, 0x04, 0x70, 0x62, 0xbe, 0x28, 0x91, 0xe9, 0x18, 0xc4,
  0x9a, 0xb7, 0x4e, 0x63, 0x39, 0x4a, 0x49, 0x12, 0xd7, 0xa2, 0xfb, 0xcf,
  0xf4, 0xe9, 0x59, 0x9e, 0xcc, 0x58, 0x4b, 0xc0, 0x94, 0xbe, 0x9a, 0x04,
  0x59, 0x03, 0x1c, 0xe6, 0xf8, 0x66, 0xde, 0xc1, 0x3d, 0xa2, 0x08, 0x67,
  0xb5, 0x0e, 0x7a, 0x32, 0xa5, 0xe2, 0xda, 0x05, 0x20, 0x37, 0x09, 0x64,
  0x21, 0x94, 0xca, 0xcc, 0x7b, 0x55, 0x52, 0xed, 0x81, 0xa6, 0x2b, 0x73,
  0xd2, 0xbd, 0xc8, 0xfd, 0x85, 0x1d, 0x78, 0x8f, 0x85, 0xf5, 0xc8, 0xd7,
  0x93, 0x16, 0x00, 0xd3, 0x93, 0x8c, 0xbb, 0x4f, 0xe1, 0x5b, 0x42, 0xe2,
  0xe1, 0xb7, 0xdc, 0x07, 0xc9, 0xc2, 0x51, 0x34, 0x6b, 0xd7, 0x0c, 0x8d,
  0x85, 0xea, 0xfc, 0xdf, 0xae, 0x0c, 0x74, 0x95, 0x16, 0xc2, 0x10, 0x51,
  0x92, 0xcb, 0x5f, 0xb6, 0xcf, 0x6a, 0xd1, 0x25, 0x16, 0x74, 0x32, 0xa2,
  0xc8, 0xfc, 0x95, 0xab, 0xed, 0x7d, 0x46, 0xd9, 0xfc, 0x27, 0x9f, 0xc9,
  0xed, 0x67, 0x1f, 0x37, 0x0e, 0xc5, 0x80, 0x8c, 0x8a, 0x25, 0xbd, 0x3f,
  0x8d, 0x93, 0xdc, 0xa9, 0x13, 0xca, 0x6d, 0x24, 0xb7, 0x8d, 0xdc, 0x35,
  0xb3, 0x98, 0xef, 0x04, 0x8f, 0x28, 0x99, 0x6c, 0xc2, 0x28, 0x46, 0x92,
  0x89, 0x1e, 0x04, 0x6a, 0x11, 0x4e, 0x08, 0xc2, 0xa0, 0x46, 0x02, 0x96,
  0x06, 0xb4, 0xd8, 0xce, 0xfc, 0x22, 0x4a, 0xf6, 0x7f, 0x1a, 0x82, 0x69,
  0xbd, 0xb7, 0x15, 0x9e, 0x0b, 0x41, 0x07, 0x4d, 0x7e, 0xbf, 0xc6, 0x84,
  0xd4, 0x50, 0xda, 0x18, 0x42, 0x55, 0x77, 0xaa, 0x84, 0x2b, 0xae, 0x07,
  0x33, 0xa6, 0x73, 0xb4, 0xc4, 0xd9, 0x75, 0x0a, 0x63, 0xd2, 0x43, 0x15,
  0x25, 0xb6, 0x22, 0x34, 0xf1, 0xe8, 0xcb, 0x3d, 0x42, 0x32, 0xbd, 0x47,
  0x03, 0x22, 0x26, 0x7b, 0x2f, 0x53, 0x46, 0x42, 0x25, 0xef, 0x6e, 0x47,
  0x4e, 0xae, 0xe5, 0x53, 0x6a, 0xc7, 0x96, 0x8d, 0x87, 0xb3, 0x5d, 0x64,
  0x09, 0xa3, 0x46, 0xed, 0x14, 0x4e, 0xfe, 0x2f, 0xc5, 0x1b, 0x88, 0x66,
  0x6f, 0x18, 0x20, 0x8a, 0x23, 0x98, 0xbf, 0x44, 0x36, 0xf8, 0xcd, 0xba,
  0x5b, 0x2d, 0x41, 0xef, 0x09, 0xe3, 0x46, 0x75, 0x35, 0x48, 0x99, 0x44,
  0x1c, 0xc3, 0xb2, 0x3a, 0x1e, 0x08, 0x1c, 0x13, 0x04, 0xc7, 0x2c, 0x79,
  0x88, 0x39, 0xc3, 0x5b, 0xef, 0x32, 0x4f, 0x71, 0x0f, 0x28, 0x76, 0xba,
  0x83, 0x86, 0x99, 0x1a, 0xef, 0xe6, 0x3b, 0x3c, 0x44, 0x8a, 0xdf, 0xcb,
  0x6b, 0x68, 0x27, 0xee, 0x04, 0xab, 0xf5, 0xf1, 0x7e, 0x33, 0x94, 0xf5,
  0x6d, 0xec, 0x7f, 0x13, 0xa2, 0x03, 0x03, 0xaf, 0x0b, 0x70, 0x59, 0x97,
  0xb0, 0xe3, 0xad, 0x29, 0x41, 0x9c, 0x22, 0x4b, 0xe0, 0x9e, 0x5a, 0xfa,
  0x7d, 0x75, 0x56, 0x4b, 0xff, 0x12, 0x9a, 0x36, 0xe1, 0x9e, 0xc5, 0x13,
  0xf7, 0x3b, 0x09, 0x99, 0xdc, 0x2b, 0xcd, 0xae, 0xe4, 0x96, 0x8b, 0x53,
  0x5a, 0xea, 0x8b, 0x7c, 0x5c, 0xdf, 0x6f, 0x10, 0x56, 0x91, 0x1b, 0xe3,
  0xe4, 0x5c, 0x64, 0x99, 0x76, 0x10, 0x96, 0xcd, 0xc9, 0x1b, 0xec, 0x6d,
  0xf5, 0x21, 0x68, 0x76, 0x1c, 0x11, 0x67, 0xc2, 0xff, 0x43, 0x97, 0x22,
  0x9d, 0xca, 0xaf, 0x64, 0x85, 0x17, 0x31, 0xa1, 0x5f, 0x78, 0x2d, 0xc3,
  0x30, 0x5d, 0xd9, 0x95, 0xd0, 0x82, 0xab, 0xd7, 0xb2, 0x6e, 0x54, 0x6c,
  0x45, 0xbd, 0x41, 0xa5, 0x76, 0x02, 0x28, 0x2d, 0x4c, 0x0d, 0x09, 0x58,
  0x11, 0x61, 0x6f, 0xcf, 0x50, 0xa2, 0xb5, 0xd2, 0xbb, 0x75, 0xf8, 0x7e,
  0xb6, 0x69, 0x66, 0x67, 0x5c, 0xf1, 0xba, 0xd0, 0x8d, 0x91, 0x80, 0x54,
  0x4d, 0xd6, 0x9a, 0x13, 0xe0, 0x7d, 0x7c, 0xe7, 0x1f, 0x83, 0x92, 0xfc,
  0xb7, 0x25, 0x8b, 0x0f, 0x19, 0x4a, 0xce, 0xa4, 0x56, 0xb3, 0xf7, 0xe3,
  0x47, 0x5d, 0x39, 0x39, 0x96, 0xc4, 0x19, 0x2c, 0x85, 0x46, 0x78, 0x7c,
  0x11, 0x91, 0x67, 0x02, 0x05, 0x8f, 0x07, 0xf0, 0x37, 0xe3, 0xf9, 0x27,
  0x70, 0xc3, 0x9f, 0xcc, 0x93, 0x31, 0x6c, 0x11, 0xd9, 0x63, 0x71, 0x3e,
  0x35, 0xd8, 0x3a, 0x2c, 0x65, 0xbe, 0x3e, 0x5c, 0xbf, 0xa5, 0xa7, 0xab,
  0xe3, 0xbd, 0x3b, 0x8a, 0xf9, 0x60, 0xc1, 0xaa, 0xc2, 0x26, 0x6e, 0xec,
  0xf1, 0x1f, 0x0b, 0x71, 0xc1, 0x8c, 0x18, 0x96, 0xcd, 0xe0, 0x3d, 0x5e,
  0x92, 0x2e, 0x2b, 0x37, 0x7d, 0x6e, 0x9f, 0x13, 0xdc, 0xd7, 0x48, 0x82,
  0xa1, 0xb4, 0xc7, 0x0c, 0x42, 0xb6, 0xf2, 0xb3, 0x16, 0x40, 0x6b, 0x6d,
  0x5e, 0x1d, 0x56, 0x19, 0xe7, 0x92, 0xc6, 0x5f, 0xa7, 0xab, 0xfa, 0x3f,
  0xbf, 0xad, 0x02, 0xf9, 0x7e, 0x31, 0xc4, 0xfe, 0xd2, 0xce, 0x76, 0x74,
  0x57, 0x80, 0x18, 0x37, 0x43, 0x69, 0x1e, 0x0d
};
void debugHEX(char *load_addr, int len)
{
        int i = 0;
#if 1
        for(i = 0; i < len;i++)
        {
               LIBDBG_MSG("%02X ",*(load_addr+i));
        }
#endif
       LIBDBG_MSG("\n-------\n");
}

#define IMAGE_DECRYPTED_NAME "/var/image_decrypted.bin"

int CheckNewIDUVersion(char DownloadVersion[])
{

        FILE *fp;
        char CurrentVersion[64] = {'\0'};
        char VersionCompareBuf[8] = {'\0'};
        int VersionCompareFlag = 0;
        int bRet = 0;
	fp = fopen("/etc_ro/lighttpd/www/idu/Version", "r");
        if (fp != NULL)
        {
                fgets(CurrentVersion, sizeof(CurrentVersion), fp); 
                CurrentVersion[strlen(CurrentVersion)-1] = '\0';
                fclose(fp);

        }
	fp = NULL;

	fp = fopen("/tmp/Upgraderollback", "r");
	if (fp != NULL)
	{
		fgets(VersionCompareBuf, sizeof(VersionCompareBuf), fp); 
		fclose(fp);

		if(VersionCompareBuf != NULL && VersionCompareBuf[0] == '1')
		{
			VersionCompareFlag = 1;	
		}
	}


	if(VersionCompareFlag != 1)
	{
		bRet = VersionCompare(DownloadVersion,CurrentVersion);
	}
	else{
		LIBDBG_MSG("Find Upgraderollback have been enabled\n");
		LIBDBG_MSG("Current Router Version allow to upgrade\n");
		VersionCompareFlag  = 1;
	}
        if(bRet < 0)
        {
               LIBDBG_MSG("CurrentVersion[%s] DownloadVersion[%s]\n",CurrentVersion,DownloadVersion);
               LIBDBG_MSG("Current Router Version can not be allow to upgrade\n");
        }
        else{
               LIBDBG_MSG("CurrentVersion[%s] DownloadVersion[%s]\n",CurrentVersion,DownloadVersion);
               LIBDBG_MSG("Current Router Version allow to upgrade\n");
        }
        return bRet;
}



/*
 * substitution of getNthValue which dosen't destroy the original value
 */
int VersionCompare(char* pVer1, char* pVer2)
{
        int nCurPos = 0;
        int nDotPos = 0;
        int nPos = 0;
        char strSec1[16] = {0};
        char strSec2[16] = {0};
        const char* pTmp1 = pVer1;
        const char* pTmp2 = pVer2;

        if (pVer1 == NULL || pVer2 == NULL)
        {
                return 0;
        }

        while ((*pTmp1 != '\0') && (*pTmp2 != '\0') && (*pTmp1 == *pTmp2))
        {
                nCurPos++;
                if (*pTmp1 == '.' && *pTmp2 == '.')
                nDotPos = nCurPos;
                pTmp1++;
                pTmp2++;

        }

        if (*pTmp1 == '\0' && *pTmp2 == '\0')
        return 0;

        if(*pTmp1 == '\0' && *pTmp2 == '.')
        {
                pTmp2++;
                nPos = 0;
                while ((*pTmp2 != '\0') && (*pTmp2 != '.') && (nPos < 15))
                {
                        strSec2[nPos++] = *pTmp2++;
                }
                strSec2[nPos] = '\0';
                return (0-atoi(strSec2));
        }
        else if(*pTmp1 == '.' && *pTmp2 == '\0')
        {
                pTmp1++;
                nPos = 0;
                while ((*pTmp1 != '\0') && (*pTmp1 != '.') && (nPos < 15))
                {
                        strSec1[nPos++] = *pTmp1++;
                }
                strSec1[nPos] = '\0';
                return atoi(strSec1);
        }
        else
        {

                nPos = 0;
                pTmp1 = pVer1 + nDotPos;
                pTmp2 = pVer2 + nDotPos;

                while ((*pTmp1 != '\0') && (*pTmp1 != '.') && (nPos < 15))
                {
                        strSec1[nPos++] = *pTmp1++;
                }
                strSec1[nPos] = '\0';

                nPos = 0;
                while ((*pTmp2 != '\0') && (*pTmp2 != '.') && (nPos < 15))
                {
                        strSec2[nPos++] = *pTmp2++;
                }
                strSec2[nPos] = '\0';

                return (atoi(strSec1) - atoi(strSec2));
        }
}

int version_verify_main(void)
{
    //uint8_t *rwdata;
    long rwlen;
    int status = 0;
    int verionstatus = 0;

#if 1
    // Image
    FILE *f = fopen(IMAGE_DECRYPTED_NAME, "r");
    fseek(f, 0, SEEK_END);
    rwlen = ftell(f);
 //  LIBDBG_MSG("%s(%d)%s %lu\n",__FUNCTION__,__LINE__,IMAGE_DECRYPTED_NAME,rwlen);
    rewind(f);


    char *rwdata = (uint8_t *)malloc(rwlen);
    if (1 != fread(rwdata, rwlen, 1, f)) {
       LIBDBG_MSG("Couldn't load %s\n", IMAGE_DECRYPTED_NAME);
        return -1;
    }
    fclose(f);
#endif
    int siglen = sizeof(const struct vb21_signature);
    char signaturestring[576] = {'\0'};
    memcpy(signaturestring,rwdata+rwlen-568,568);
    signaturestring[568]='\0';

    char versiontring[32] = {'\0'};
    memcpy(versiontring,rwdata+rwlen-568-27,27);
    versiontring[26]='\0';

   // debugHEX(rwdata,30);
   // debugHEX(rwdata+rwlen-30,30);
   // debugHEX(signaturestring,30);
   // debugHEX(signaturestring+568-30,30);

    status = vblock_check_signature(rwdata, (const struct vb21_packed_key *)key_vbpubk2, (const struct vb21_signature *)signaturestring);
    if (status)
    {
                status = 1;
    }
    free(rwdata);

    if(status == 1)
    {
                verionstatus =  CheckNewIDUVersion(versiontring);

                if(verionstatus < 0)
                {
                        status = -1;
                }
    }

    return status;
}
