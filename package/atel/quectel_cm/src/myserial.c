/*
 * Copyright (C) 2005 Asiatelco. All rights reserved.
 * 
 * filename: myserial.c
 *
 * Version: 1.0
 * Author: chenglei
 */

#include <string.h>
#include	"myserial.h"

int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = { 115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200,  300};

/*
 * funcation: OpenDev
 * input: char* devport
 * output: int* devfd
 * return: -1 or 0
 */
int OpenDev(int* devfd, char* devport)
{
	int	fd = -1;
	
	if (devfd == NULL)
		return -1;

	fd = open( devport, O_RDWR | O_NDELAY );         //| O_NOCTTY | O_NDELAY	
	if (fd == -1 )	
	{ 			
		perror(devport);
		return -1;		
	}	
	else
	{
//		fcntl(fd, F_SETFL, FNDELAY);
		*devfd = fd;
		return 0;
	}
}

/*
 * funcation: SetSpeed
 * input: int fd, int speed
 * output: null
 * return -1 or 0
 */
int SetSpeed(int fd, int speed)
{
	size_t   i; 
	int   status; 
	struct termios   Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{ 
		if(speed == name_arr[i])
		{     
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0)
			{        
				perror("tcsetattr fd");  
				return -1;     
			 }    
			tcflush(fd,TCIOFLUSH);   
		 }  
	 }
	return 0;
}

/*
 * funcation: SetParity
 * input: int fd, int databits, int stopbits, int parity
 * output: null
 * return: -1 or 0
 */
int SetParity(int fd,int databits,int stopbits,int parity)
{ 
	struct termios options; 
	if ( tcgetattr( fd,&options)  !=  0)
	{ 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
	case 7:		
		options.c_cflag |= CS7; 
		break;
	case 8:     
		options.c_cflag |= CS8;
		break;   
	default:    
		fprintf(stderr,"Unsupported data size\n");
		return -1;  
	}
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
			options.c_iflag |= INPCK;             /* Disnable parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S': 
		case 's':  /*as no parity*/   
		    options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;  
		default:   
			fprintf(stderr,"Unsupported parity\n");    
			return -1;  
		}  
	/* 设置停止位*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
		   break;
		default:    
			 fprintf(stderr,"Unsupported stop bits\n");  
			 return -1; 
	} 
	/* Set input parity option */ 
	if (parity != 'n')   
		options.c_iflag |= INPCK; 
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	///Update to match Altair new firmware 33/35 version
	options.c_cflag |= (CLOCAL | CREAD);
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= ~OPOST; 

	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		perror("SetupSerial 3");   
		return -1;  
	} 
	return 0;  
}

/*
 *	funcation: check_command()
 *	input: const char* command
 *	output: null
 *	return: BOOL
 */
static int check_command( const char* command )
{
	if ( NULL != command )
	{
		size_t length = strlen( command );
		if ( length >= 3 )
		{
			if ( ( ('a' == command[0]) || ('A' == command[0]) )
			     && ( ('t' == command[1]) || ('T' == command[1]) )
			     && ('\r' == command[length - 1]) )
			{
				return(1);
			}
		}
	}
	printf( "BAD AT CMD:%s\n", command ? command : "null" );
	return(0);
} /* END */

static void skiplinetable(char *line)
{
      int i = 0;       
      for(i = 0; i < 10; i++)
      {
           if(line[i] == '\r' || line[i] == '\n')
           {
                   
           }
           else{
                break;            
          }
      }

      if(i != 0)
      {
              size_t j;
  //           printf("i=%d---bline=[%s]\n",i,line);
             for(j = 0; j < strlen(line) - i; j++)
             {
                    line[j] = line[j+i];
             }
             line[j]='\0';
    //         printf("---fline=[%s]\n",line);
      }
}

void dump_returnbuf( const char* buf, int len )
{
	int	line_count	= 15;
	int	i		= 0;
	int	line		= 0;
	for ( i = 0; i < len; i++ )
	{
		printf( "%c", buf[i] );
		if ( i != 0 && (i % line_count) == 0 )
		{
			line++;
		}
	}
	printf( "\n" );
}


/*
 * funcation: at_handle()
 * input: int fd, const char* at_commands UINT wait_time
 * output: char* at_returns
 * return: STATUS status
 */
STATUS at_handle( int fd, const char* at_commands, char* at_returns_ret, unsigned int wait_time )
{
	STATUS	status = STATUS_OK;
	int i, length, len, time_count = 0;
	int nWriteCount = 0; //, nWriteRet;
	int nReadCount	= 0; //, nReadRet;
	int nReadCount1 = 0;
	int timeout_read	= wait_time * 100*2;
	int second_read = 0;
	char	at_returns[MAX_BUF_SIZE];
	memset( at_returns, 0, MAX_BUF_SIZE );


	/*
	 * int at_len = strlen(at_commands);
	 * can not use DEBUG_Lx process here
	 * printf("!!at_commands:%s\n",at_commands);
	 */

	 printf( "%s(%d),fd:%d,AT CMD:%s\n", __FUNCTION__, __LINE__, fd, at_commands ? at_commands : "null");

	if ( (NULL == at_commands) || (NULL == at_returns_ret) /*|| (wait_time < 0)*/ )
		return STATUS_BAD_ARGUMENTS;
	else if ( !check_command( at_commands ) )
		status = STATUS_BAD_AT_CMD;

	if ( status != STATUS_OK )
		return(status);
	//bReadAT = TRUE;
	memset( at_returns_ret, 0, strlen( at_returns_ret ) );
	/* write */
	length = len = strlen( at_commands );

	do
	{
		int nWriteRet = write( fd, at_commands + nWriteCount, len );
		if ( -1 == nWriteRet )
		{
			//bReadAT = FALSE;
			return(STATUS_ERR);
		}
		nWriteCount += nWriteRet;
		len 	-= nWriteRet;
	}
	while ( len != 0 );
	if ( strstr( at_commands, "CUSEXE" ) != NULL )
	{
		status = STATUS_OK;
		return(status);
	}


	/*
	 * printf("%s(%d) \n",__FUNCTION__,__LINE__);
	 * read
	 */
	len = MAX_BUF_SIZE;
	for ( i = 0; i < timeout_read; i++ )
	{
		/*
		 * printf("%s(%d) i=%d,timeout_read=%d\n",__FUNCTION__,__LINE__,i,timeout_read);
		 * read
		 */
		int nReadRet = read( fd, at_returns + nReadCount, len );

		if ( nReadRet > 0 )
		{
			nReadCount	+= nReadRet;
			len 	-= nReadRet;
		}
		if ( len < 0 )
		{
			status = STATUS_ERR;
			break;
		}

#if 0
				if(strstr(at_commands,"CLCK") != NULL || strstr(at_commands,"CPIN") != NULL)
				{
						
						printf( "%s(%d) nReadCount[%d] nReadRet[%d]\n", __FUNCTION__, __LINE__, nReadCount,nReadRet);
						printf( "%s(%d) at_returns[%s]\n", __FUNCTION__, __LINE__, at_returns);

				}
#endif

		if ( second_read == 0 )
		{
			if ( (nReadCount > 0) && (nReadRet <= 0) )
			{
				if ( (strncmp( at_returns, at_commands, length - 1 ) == 0) && nReadCount <= length && second_read == 0 )
				{
					printf( "%s(%d) nReadCount=%d.\n", __FUNCTION__, __LINE__, nReadCount );
					time_count	= 0;
					second_read = 1;
					nReadCount1 = nReadCount;
					continue;
				}
				if ( ++time_count >= 10 * 10 )
				{
					printf( "%s(%d) nReadCount=%d.\n", __FUNCTION__, __LINE__, nReadCount );
					break;
				}
			}/*else{
				second_read = 1;
				nReadCount1 = nReadCount;
				continue;
						}
						*/

		}else if ( second_read == 1 )
		{

			if ( (nReadCount > nReadCount1) && (nReadRet <= 0) )
			{
				if ( time_count >= 10 * 5 )
				{
					printf( "%s(%d) nReadCount=%d.\n", __FUNCTION__, __LINE__, nReadCount );
					break;
				}
			}

		}
		time_count++;
		usleep( 5000 ); /* wait_time * 100*2 * 5000 = wait_time seconds */ 
	}


	/*
	 * printf("%s(%d) \n",__FUNCTION__,__LINE__);
	 * read
	 */

	/* /if ((time_count < 3) && (i >= wait_time * 20)) */
	if ( (i >= timeout_read) )
		status = STATUS_TIME_OUT;


	at_returns[nReadCount]	= '\0';
	//bReadAT 		= FALSE;
	nReadCount++;

		if(nReadCount > 3)
				skiplinetable(at_returns);


	if ( (at_commands != NULL && strncmp( at_returns, at_commands, strlen( at_commands ) - 1 ) == 0) || (/*at_returns != NULL &&*/ strstr( at_returns, "ERROR" ) != NULL ))
	{
		memcpy( at_returns_ret, at_returns + strlen( at_commands ), nReadCount - strlen( at_commands ) );

//		if ( at_returns_ret != NULL )
		{
			char *ptrok = strstr( at_returns_ret, "OK\n" );
			if ( ptrok != NULL )
				at_returns_ret[nReadCount - strlen( at_commands ) - strlen( ptrok ) + 2] = '\0';
		}
	}


	printf( "at_returns[");
	if ( at_returns_ret != NULL && strlen( at_returns_ret ) > 1 )
		dump_returnbuf( at_returns_ret, strlen( at_returns_ret ) );
	printf( "]\n" );

	return(status);
}

