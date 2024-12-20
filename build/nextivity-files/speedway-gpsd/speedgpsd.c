#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <syslog.h>


#define GPS_BUFF_SIZE 2048
// The maxmimum number of characters in a GPS message
#define MAX_MSG_SIZE 128
// The GPS message must contain the 7-char head, the 3-char tail
// and at least one character of payload:
// $GPGSA,A*2D
#define MIN_MSG_SIZE 11
// The number of GPS messages comes from
// the average number of GPS messages which can fit into the size
// of the GPS  buffer on the microcontroller.
// Roughly 30 GPS messages fit into 1536 bytes.
// Roughly 15 GPS messages fit into 768 bytes.
// Roughly 11 GPS messages fit into 512 bytes.
#define NUM_GPS_MESSAGES 30
// Miles per hour per knots
#define MPH_PER_KNOTS 1.15078

#define WRITE_DEBUG_MSG_FILES 0
#define WRITE_DEBUG_MSG_TO_LOG 0
#define PRINT_MSG_TO_HOST 0

// NMEA message prefixes found in  Megafi GPS output:
//  $GPGGA
//
//  $GNGLL
//
//  $GAGSA
//  $GBGSA
//  $GNGSA
//  $GPGSA
//
//  $GAGSV
//  $GBGSV
//  $GPGSV
//
//  $GNRMC
//
//  $GNVTG
//
//  $GNZDA

// Formats requested in Jira Ticket AW12FI-175  GPS Output Needs update
//  $GPGSV (GPS satellites in view)
//  $GPGGA (GPS fix data and undulation)
//  $GPVTG (Track made good and ground speed)
//  $GPRMC (GPS specific information)
//  $GPGSA (GPS DOP and active satellites)

// NMEA message prefixes:
// GGA -> Time, position, and fix related data of the receiver.
// Requested in Jira Ticket AW12FI-175
//  $GNGGA
//  $GPGGA
// GLL -> Position, time and fix status.
// NOT Requested in Jira Ticket AW12FI-175
//  $GNGLL
//  $GPGLL
// GSA -> Used to represent the ID’s of satellites which are used for position fix.
// Requested in Jira Ticket AW12FI-175
//  $GAGSA
//  $GBGSA
//  $GNGSA
//  $GPGSA
//  $GLGSA
// GSV -> Satellite information about elevation, azimuth and CNR
// Requested in Jira Ticket AW12FI-175
//  $GAGSV
//  $GBGSV
//  $GPGSV
//  $GLGSV
// RMC - > Time, date, position, course and speed data
// Requested in Jira Ticket AW12FI-175
//  $GNRMC
//  $GPRMC
// VTG - > Course and speed relative to the ground.
// Requested in Jira Ticket AW12FI-175
//  $GNVTG
//  $GPVTG
// ZDA - > Time, date, position, course and speed data
// NOT Requested in Jira Ticket AW12FI-175
//  $GNZDA
//  $GPZDA

// Example NMEA Messages:
// $GAGSV,1,1,01,05,00,000,33,7*47
// $GBGSV,2,1,05,26,56,329,31,45,48,075,46,29,40,060,46,14,15,293,35,1*74
// $GPGGA,193916.000,3301.18522,N,11705.25612,W,1,13,,N,11705.25617,W,000.1,000.0,140224,,,A,V*1A
// $GPGSV,3,1,10,17,75,173,39,30,68,101,46,14,62,012,45,22,6242,29,41,060,42,33,00,000,31,5*76
// $GNGSA,A,3,02,14,22,30,,,,,,,,,6.0,3.8,4.6,1*3A
// $GNGLL,3301.18388,N,11705.25559,W,193806.000,A,A*57
// $GNRMC,193806.000,A,3301.18388,N,11705.25559,W,000.1,000.0,140224,,,A,V*1A
// $GNVTG,000.0,T,,M,000.1,N,000.1,K,A*13
// $GNZDA,193806.000,14,02,2024,00,00*4E

// GGA Format:
//  GPS fix data and undulation
// 1    $GPGGA  Log header  $GPGGA
// 2    utc     UTC time status of position (hours/minutes/seconds)     hhmmss.ss   202134.00
// 3    lat     Latitude (DDmm.mm)                          llll.ll     5106.9847
// 4    lat dir Latitude direction (N = North, S = South)   a           N
// 5    lon     Longitude (DDDmm.mm)                        yyyyy.yy    11402.2986
// 6    lon dir Longitude direction (E = East, W = West)    a           W
// 7    quality refer to Table: GPS Quality Indicators      x           1
// 8    # sats  Number of satellites in use.                xx          10
// 9    hdop    Horizontal dilution of precision            x.x         1.0
// 10   alt     Antenna altitude above/below mean sea level x.x         1062.22
// 11   a-units Units of antenna altitude (M = metres)      M           M
// 12   undulation  Undulation - the relationship between   x.x         -16.271
//          the geoid and the WGS84 ellipsoid
// 13   u-units Units of undulation (M = metres)            M           M
// 14   age     Age of correction data (in seconds)         xx
//          The maximum age reported here is limited to 99 seconds.
// 15   stn ID  Differential base station ID                xxxx
// 16   *xx     Check sum                                   *hh         *48
// 17   [CR][LF]Sentence terminator                         [CR][LF]

// GLL Format:
//  Geographic position
// 1    Message ID $GPGLL
// 2    Latitude in dd mm,mmmm format (0-7 decimal places)
// 3    Direction of latitude N: North S: South
// 4    Longitude in ddd mm,mmmm format (0-7 decimal places)
// 5    Direction of longitude E: East W: West
// 6    UTC time of position in hhmmss.ss format
// 7    Status indicator: A: Data valid V: Data not valid
//      This value is set to V (Data not valid) for all Mode Indicator
//      values except A (Autonomous) and D (Differential)
// 8    Mode indicator:
//          A: Autonomous mode
//          D: Differential mode
//          E: Estimated (dead reckoning) mode
//          M: Manual input mode
//          S: Simulator mode
//          N: Data not valid
// 9    *xx     Check sum                                   *hh         *48
// 10   [CR][LF]Sentence terminator                         [CR][LF]

// GSA Format:
//  GPGSA - GPS DOP and active satellite
//  We are not going to parse this so we do not care about the field defintions

// GSV Format:
//  GPGSV - GPS satellites in view
//  We are not going to parse this so we do not care about the field defintions

// RMC Format:
//  GPRMC - GPS specific information
// 1   $GPRMC      Log header                                                      $GPRMC
// 2   utc         UTC time of position hhmmss.ss                      144326.00
// 3   pos status  Position status (A = data valid, V = data invalid)  A           A
// 4   lat         Latitude (DDmm.mm)                                  llll.ll     5107.0017737
// 5   lat dir     Latitude direction: (N = North, S = South)          a           N
// 6   lon         Longitude (DDDmm.mm)                                yyyyy.yy    11402.3291611
// 7   lon dir     Longitude direction: (E = East, W = West)           a           W
// 8   speed Kn    Speed over ground, knots                            x.x         0.080
//                 One Knots is equal to 1.15078 Miles Per Hour
// 9   track true  Track made good, degrees True                       x.x         323.3
// 10  date        Date: dd/mm/yy                                      xxxxxx      210307
// 11  mag var     Magnetic variation, degrees                         x.x         0.0
//                 Note that this field is the actual magnetic variation and will always be positive.
//                 The direction of the magnetic variation is always positive.
// 12  var dir     Magnetic variation direction E/W                    a           E
//                 Easterly variation (E) subtracts from True course.
//                 Westerly variation (W) adds to True course.
// 13  mode ind    Positioning system mode indicator, see Table:       a           A
//                 NMEA Positioning System Mode Indicator
// 14  *xx         Check sum                                           *hh         *20
// 15  [CR][LF]    Sentence terminator                                             [CR][LF]

// ZDA Format:
//  GPZDA - UTC time and date
// 1   $GPZDA           Log header                                          $GPZDA
// 2   utc             UTC time status                         hhmmss.ss   220238.00
// 3   day             Day, 01 to 31                           xx          15
// 4   month           Month, 01 to 12                         xx          07
// 5   year            Year                                    xxxx        1992
// 6   null            Local zone description—not available    xx          (empty when no data is present)
//                     Local time zones are not supported by OEM7 family receivers.
//                     Fields 6 and 7 are always null.
// 7   null            Local zone minutes description—not available    xx  (empty when no data is present)
// 8   *xx             Check sum                               *hh         *6F
// 9   [CR][LF]        Sentence terminator                                 [CR][LF]

// VTG Format:
//  Track made good and ground speed
// 1   $GPVTG          Log header                              $GPVTG
// 2   track true      Track made good, degrees True   x.x     24.168
// 3   T               True track indicator            T       T
// 4   track mag       Track made good, degrees Magnetic;
//                     Track mag = Track true + (MAGVAR correction)
//                     See the MAGVAR command          x.x     24.168
// 5   M               Magnetic track indicator        M       M
// 6   speed Kn        Speed over ground, knots        x.x     0.4220347
//                     One Knots is equal to 1.15078 Miles Per Hour
// 7   N               Nautical speed indicator
//                     (N = Knots)                     N       N
// 8   speed Km        Speed, kilometres/hour          x.x     0.781608
// 9   K               Speed indicator (K = km/hr)     K       K
// 10  mode ind        Positioning system mode indicator,
//                     see Table: NMEA Positioning
//                     System Mode Indicator           a       A
// 11  *xx             Check sum                       *hh      *7A
// 12  [CR][LF]        Sentence terminator

/**
 * Outgoing connections
 */

#define MAX_HOSTS 16
#define HOSTS_BUFFER_SIZE 2048
// How often to send the full buffer mode mesage
#define FULL_BUFF_MSG_PERIOD 5
#define MAX_ARG_FIELD_SIZE 80

#define GPS_STATUS_FILE "/tmp/nextivity/gpsstatus.json"
#define MAX_LAT 90.0
#define MIN_LAT -90.0
#define MAX_LONG 180.0
#define MIN_LONG -180.0
/* Normal 32 characer map used for geohashing */
static char char_map[32] = "0123456789bcdefghjkmnpqrstuvwxyz";
#define GEOHASHPRECISION 12

typedef enum
{
    gpsConnectionDisabled = 0,
    gpsConnectionInternal = 1,
    gpsConnectionTCP = 2,
    gpsConnectionUDP = 3
} gpsConnectionType;

typedef enum
{
    gpsOutputDisabled = 0,
    gpsOutputNMEA = 1,
    gpsOutputTAIP = 2,
} gpsOutputType;

typedef struct
{
    char ip[32];
    char gpsID[5];

    int port;
    int sock;
    struct sockaddr_in servaddr;

    gpsConnectionType connection;
    gpsOutputType output;

    size_t rate;
    time_t lastSend;
    time_t lastConnect;
} gpsHost;

typedef struct IntervalStruct
{
    double high;
    double low;

} Interval;

typedef enum {
    G_GGA = 1,
    G_GLL = 2,
    G_GSA = 3,
    G_GSV = 4,
    G_RMC = 5,
    G_VTG = 6,
    G_ZDA = 7,
    G_UNKWN = 8
} gpsMsgType;

// from statusgather
typedef struct status_info {
  int                 gpsFD;
  pthread_t           gpsThread;
  bool                bdebug;
} StatusInfo;

static bool exiting               = false;

static char gpsBuff[GPS_BUFF_SIZE + 1];
typedef struct {
    char message[MAX_MSG_SIZE + 1];
    gpsMsgType msgTyp;
    int utcTimeSec;
    float latitude;
    float longitude;
    float altitude;
    float speedMPH;
    float bearing;
    char stationID[5];
    bool sendThisMsg;
} gpsMessage;


// The keyword static is used to limit these variables to file scope.
static gpsMessage gpsMsgArray[NUM_GPS_MESSAGES];
static int idxOfNewestVTGMsg;
static int idxOfNewestGGAMsg;
// The longest TAIP sentence is less than 80 characters
// https://customer.cradlepoint.com/s/article/GPS-TAIP-Sentence-Guide
// Max size of a TAIP message
#define TAIP_MSG_SIZE 256
static char latestTAIPSentence[TAIP_MSG_SIZE];

char hostbuffer[HOSTS_BUFFER_SIZE];

static gpsHost hosts[MAX_HOSTS];
static size_t numHosts = 0;

/**
 * Incoming connections
 */
#define MAX_CLIENTS 32

typedef struct
{
    int sock;
} gpsClient;

static gpsClient clients[MAX_CLIENTS];
static int serverSocket = -1;

static struct sockaddr_un localAddress;

void LogMessage(const char *message)
{
    fprintf(stdout, "%s\n", message);
    syslog(LOG_DAEMON | LOG_NOTICE, "speedway-gpsd: %s\n", message);
}

void LogError(const char *message)
{
    fprintf(stderr, "%s\n", message);
    syslog(LOG_DAEMON | LOG_ERR, "speedway-gpsd: %s\n", message);
}

static void usage(const char *command)
{
    fprintf(stderr, "Usage (Legacy): %s [-d <serial device>|-dummy] [-p <port>] -c <client IP> [-f nmea|taip] [-t] [-id <NMEA/TAIP ID>] [-r rate]\n", command);
    fprintf(stderr, "Usage (Multiple): %s [-d <serial device>|-dummy] [-p <port>] [-t] -h <ip>,<port>,<proto>,<format>[,<rate>[,<id>]]] [ -h ... ]\n", command);

    exit(EXIT_FAILURE);
}

static int calcChecksumValue(const char *nmeaStr)
{
    unsigned char checksum = 0;

    // char logbuffer[368];
    // snprintf(logbuffer, sizeof(logbuffer), "calcChecksumValue(): input = %s", nmeaStr);
    // LogMessage(logbuffer);
    // snprintf(logbuffer, sizeof(logbuffer), "calcChecksumValue(): %d, firstchar = %c", (int)checksum, nmeaStr[0]);
    // LogMessage(logbuffer);

    if (!nmeaStr[0])
        return 0;
    size_t pos = 1; // Skip the '$'
    size_t len = strlen(nmeaStr);
    while (pos < len && nmeaStr[pos])
    {
        if (nmeaStr[pos] == '*')
            break;
        checksum ^= nmeaStr[pos];

        // snprintf(logbuffer, sizeof(logbuffer), "*calcChecksumValue(): %d: ", (int)checksum);
        // LogMessage(logbuffer);

        pos++;
    }
    return (int)checksum;
}

#if WRITE_DEBUG_MSG_TO_LOG
static int getChecksumStrVal(char *nmeaStr)
{
    
    // char logbuffer[368];
    // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): input = %s", nmeaStr);
    // LogMessage(logbuffer);

    if (!nmeaStr[0])
        return 0;

    char checksumStr[5];
    size_t pos = 1; // Skip the '$'
    size_t len = strlen(nmeaStr);
    while (pos < (len - 2) && nmeaStr[pos])
    {
        // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): char = %c", nmeaStr[pos]);
        // LogMessage(logbuffer);

        if (nmeaStr[pos] == '*')
        {
            // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): before copy checksum string");
            // LogMessage(logbuffer);

            checksumStr[0] = '0';
            checksumStr[1] = 'x';
            checksumStr[2] = nmeaStr[pos + 1];
            checksumStr[3] = nmeaStr[pos + 2];
            checksumStr[4] = '\0';

            // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): checksumStr = %s", checksumStr);
            // LogMessage(logbuffer);

            break;
        }
        pos++;
    }

    // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): before strtol checksumStr = %s", checksumStr);
    // LogMessage(logbuffer);

    int checksumStrVal = (int)strtol(checksumStr, NULL, 0);

    // snprintf(logbuffer, sizeof(logbuffer), "getChecksumStrVal(): before strtol checksumStrVal = %d", checksumStrVal);
    // LogMessage(logbuffer);

    return checksumStrVal;
}
#endif 

static bool updateChecksumStr(char *nmeaStr)
{
    int calcCheckSum = calcChecksumValue(nmeaStr);
    char newCheckSumStr[4];
    sprintf(newCheckSumStr, "%02X", calcCheckSum);

#if WRITE_DEBUG_MSG_TO_LOG
    char logbuffer[368];
    snprintf(logbuffer, sizeof(logbuffer), "updateChecksumStr(): newCheckSumStr = %s", newCheckSumStr);
    LogMessage(logbuffer);
#endif

    size_t pos = 1; // Skip the '$'
    size_t len = strlen(nmeaStr);
    while (pos < (len - 2) && nmeaStr[pos])
    {
        if (nmeaStr[pos] == '*')
        {
            nmeaStr[pos + 1] = newCheckSumStr[0];
            nmeaStr[pos + 2] = newCheckSumStr[1];
            nmeaStr[pos + 3] = 0;

#if WRITE_DEBUG_MSG_TO_LOG
            snprintf(logbuffer, sizeof(logbuffer), "updateChecksumStr(): nmeaStr = %s", nmeaStr);
            LogMessage(logbuffer);
#endif

            return true;
        }
        pos++;
    }
    return false;
}

static bool connectServer(gpsHost *host)
{
    int sockfd = -1;

    if (host->connection == gpsConnectionTCP)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            LogError("Unable to create TCP socket");
        }

        bzero(&host->servaddr, sizeof(host->servaddr));

        host->servaddr.sin_family = AF_INET;
        host->servaddr.sin_addr.s_addr = inet_addr(host->ip);
        host->servaddr.sin_port = htons(host->port);

        if (connect(sockfd, (struct sockaddr *)&host->servaddr, sizeof(host->servaddr)) != 0)
        {
            printf("* Connection failed to %s port %d (%s), waiting\n", host->ip, host->port, strerror(errno));
            close(sockfd);
            return false;
        }

        printf("* Connected OK to %s port %d\n", host->ip, host->port);
    }
    else if (host->connection == gpsConnectionUDP)
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1)
        {
            LogError("Unable to create UDP socket");
            return false;
        }

        bzero(&host->servaddr, sizeof(host->servaddr));

        host->servaddr.sin_family = AF_INET;
        host->servaddr.sin_addr.s_addr = inet_addr(host->ip);
        host->servaddr.sin_port = htons(host->port);
    }
    else if (host->connection == gpsConnectionInternal)
    {
        sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);

        if (sockfd == -1)
        {
            char message[128];

            snprintf(message, sizeof(message), "Unable to create local socket: %s",
                     strerror(errno));

            LogError(message);
            return false;
        }
    }
    else
    {
        return false;
    }

    host->sock = sockfd;
    return sockfd != -1;
}

static bool createServer(const char *serverAddress, int serverPort)
{
    struct sockaddr_in server;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        LogError("Could not create server socket");
        return false;
    }

    // TODO: Look up LAN IP address
    server.sin_addr.s_addr = inet_addr(serverAddress);
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);

    fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // LogError("bind failed: %s", strerror(errno));
        LogError("Bind failed");
        close(serverSocket);
        serverSocket = -1;

        return false;
    }

    if (listen(serverSocket, 1) < 0)
    {
        // LogError("listen failed: %s", strerror(errno));
        LogError("Listen failed");
        close(serverSocket);
        serverSocket = -1;

        return false;
    }

    return true;
}

static bool acceptServer(void)
{
    if (serverSocket == -1)
        return false;

    int c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;

    int clientSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t *)&c);

    if (clientSocket < 0)
    {
        return false;
    }

    // Look for free slot
    for (size_t num = 0; num < MAX_CLIENTS; num++)
    {
        if (clients[num].sock == -1)
        {
            clients[num].sock = clientSocket;

            char message[64];
            snprintf(message, sizeof(message), "Client connected on socket %d", clientSocket);
            LogMessage(message);
            return true;
        }
    }

    // Out of slots, disconnect
    close(clientSocket);

    return false;
}

static int sendData(gpsHost *host, const char *buffer, size_t len)
{
    size_t offset = 0;
    while (len)
    {
        int sresult;

        if (host->connection == gpsConnectionTCP)
        {
            sresult = write(host->sock, buffer + offset, len);
        }
        else if (host->connection == gpsConnectionUDP)
        {
            sresult = sendto(host->sock, buffer + offset, len, 0,
                             (const struct sockaddr *)&host->servaddr, sizeof(host->servaddr));
        }
        else if (host->connection == gpsConnectionInternal)
        {
            sresult = sendto(host->sock, buffer + offset, len, 0,
                             (struct sockaddr *)&localAddress, sizeof(localAddress));

            // StatusGather not ready
            if (sresult < 0)
            {
                if ((errno == ENOENT) || (errno == ECONNREFUSED))
                {
                    return len;
                }
            }
        }
        else
        {
            return -1;
        }

        if (sresult < 0)
        {
            fprintf(stderr, "Error during write to %s: %s\n", host->ip, strerror(errno));
            close(host->sock);
            host->sock = -1;
            return -1;
        }
        else if (sresult == 0)
        {
            fprintf(stderr, "No bytes sent during write\n");
            return offset;
        }
        len -= sresult;
        offset += sresult;
    }
    return offset;
}

static void disconnectClient(gpsClient *client)
{
    char message[64];

    if (client->sock == -1)
        return;

    // -- logging --
    snprintf(message, sizeof(message), "Client disconnected from socket %d", client->sock);
    LogMessage(message);

    close(client->sock);
    client->sock = -1;
}

static void sendClients()
{
    if (serverSocket == -1)
        return;
    if (idxOfNewestGGAMsg == -1) 
		return;

    char *buffer = gpsMsgArray[idxOfNewestGGAMsg].message;
    size_t len = strlen(buffer);
    for (size_t client = 0; client < MAX_CLIENTS; client++)
    {
        int sock = clients[client].sock;

        if (sock != -1)
        {
                size_t offset = 0;
                size_t clientLen = len;
                int result;
                while (clientLen)
                {
                    result = write(sock, buffer + offset, clientLen);

                    if (result < 0)
                    {
                        fprintf(stderr, "Error during write to socket %d: %s\n", sock, strerror(errno));
                        disconnectClient(&clients[client]);
                        break;
                    }

                    clientLen -= result;
                    offset += result;
                }

                if (!clientLen)
                {
                    // No line end, so send here
                    result = write(sock, "\r\n", 2);

                    if (result != 2)
                    {
                        fprintf(stderr, "Error during write to socket %d: %s\n", sock, strerror(errno));
                        disconnectClient(&clients[client]);
                    }
                }
            }
        }
    }

static gpsMsgType getCPSMsgType(const char *nmea) {
    // GPS Message types
    // G_GGA = 1,
    // G_GLL = 2,
    // G_GSA = 3,
    // G_GSV = 4,
    // G_RMC = 5,
    // G_ZDA = 6,
    // G_UNKWN = 7
    gpsMsgType typ = G_UNKWN;
    if (strncmp(nmea + 3, "GGA", 3) == 0) {
        typ = G_GGA;
    } else if (strncmp(nmea + 3, "GLL", 3) == 0) {
        typ = G_GLL;
    } else if (strncmp(nmea + 3, "GSA", 3) == 0) {
        typ = G_GSA;
    } else if (strncmp(nmea + 3, "GSV", 3) == 0) {
        typ = G_GSV;
    } else if (strncmp(nmea + 3, "RMC", 3) == 0) {
        typ = G_RMC;
    } else if (strncmp(nmea + 3, "VTG", 3) == 0) {
        typ = G_VTG;
    } else if (strncmp(nmea + 3, "ZDA", 3) == 0) {
        typ = G_ZDA;
    }
    return typ;
}

static void resetGPSMessages() {
    for (int i = 0; i < NUM_GPS_MESSAGES; ++i) {
        memset(gpsMsgArray[i].message, 0, MAX_MSG_SIZE + 1);
        strcpy(gpsMsgArray[i].stationID, "0000");
        gpsMsgArray[i].msgTyp = G_UNKWN;
        gpsMsgArray[i].utcTimeSec = 0;
        gpsMsgArray[i].latitude = 0.0;
        gpsMsgArray[i].longitude = 0.0;
        gpsMsgArray[i].altitude = 0.0;
        gpsMsgArray[i].speedMPH = 0.0;
        gpsMsgArray[i].bearing = 0.0;
        gpsMsgArray[i].sendThisMsg = false;
    }
}

static void parseNMEA(gpsMessage *gpsMessage) 
{
    char inputStr[MAX_MSG_SIZE];
    char *fld;
    int fieldIndex = 1;

    strcpy(inputStr, gpsMessage->message);
    // printf("+++++++++++++++++++++\n");

    gpsMessage->msgTyp = getCPSMsgType(gpsMessage->message);
    fld = strtok(inputStr, ",");

    while (fld != NULL) 
    {
        // printf("Field %d: %s\n", fieldIndex, fld);
        //  Is this a UTC time field?
        if (((fieldIndex == 2) &&
             ((G_GGA == gpsMessage->msgTyp) ||
              (G_RMC == gpsMessage->msgTyp) ||
              (G_ZDA == gpsMessage->msgTyp))) ||
            ((fieldIndex == 6) &&
             (G_GLL == gpsMessage->msgTyp))) 
        {
            int utcTime = atoi(fld);
            gpsMessage->utcTimeSec = (((utcTime / 10000) * 60) + ((utcTime / 100) % 100)) * 60 + (utcTime % 100);
            // Is this a latitude field?
        } 
        else if (((fieldIndex == 3) &&
                    (G_GGA == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 4) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 2) &&
                    (G_GLL == gpsMessage->msgTyp))) {
                        float latitude = atof(fld); // latitude DDmm.mm format
                        int lat_dec = (int)latitude / 100; // decimal part of latitude
                        latitude = (float)lat_dec + ((latitude - (float)(lat_dec * 100)) / 60); // degrees value
                        gpsMessage->latitude = latitude;
            // Is this a latitude direction field?
        } 
        else if (((fieldIndex == 4) &&
                    (G_GGA == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 5) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 6) &&
                    (G_GLL == gpsMessage->msgTyp))) 
        {
            if (strcmp(fld, "S") == 0)
                gpsMessage->latitude = -gpsMessage->latitude;
            // Is this a longitude field?
        } 
        else if (((fieldIndex == 5) &&
                    (G_GGA == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 6) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 4) &&
                    (G_GLL == gpsMessage->msgTyp))) {
                        float longitude = atof(fld); // longitude DDDmm.mm format
                        int long_dec = (int)longitude / 100; // decimal part of longitude
                        longitude = (float)long_dec + ((longitude - (float)(long_dec * 100)) / 60); // degrees value
                        gpsMessage->longitude = longitude;
            // Is this a longotude direction field?
        } 
        else if (((fieldIndex == 6) &&
                    (G_GGA == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 7) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 5) &&
                    (G_GLL == gpsMessage->msgTyp))) 
        {
            if (strcmp(fld, "W") == 0)
                gpsMessage->longitude = -gpsMessage->longitude;
            // Is this a track (bearing) field?
        } 
        else if (((fieldIndex == 9) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 2) &&
                    (G_VTG == gpsMessage->msgTyp))) 
        {
            gpsMessage->bearing = atof(fld);
            // Is this a speed in knots field?
        } 
        else if (((fieldIndex == 8) &&
                    (G_RMC == gpsMessage->msgTyp)) ||
                   ((fieldIndex == 7) &&
                    (G_VTG == gpsMessage->msgTyp))) 
        {
            gpsMessage->speedMPH = MPH_PER_KNOTS * atof(fld);
        } 
        else if (((fieldIndex == 10) &&
                    (G_GGA == gpsMessage->msgTyp))) 
        {
            gpsMessage->altitude = atof(fld);
        }

        // get field string
        fld = strtok(NULL, ",");

        fieldIndex++;
    }
    // printf("+++++++++++++++++++++\n");
}

static unsigned char checksumNMEA(const char *nmea, size_t len) {
    unsigned char checksum = 0;
    if (!nmea[0]) return 0;
    size_t pos = 1;
    while (pos < len && nmea[pos]) {
        checksum ^= nmea[pos];
        pos++;
    }
    return checksum;
}

static bool geohash_encode(double lat, double lng, char *hash, int precision) {
    if (precision <= 1)
    {
        return false;
    }

    if (lat <= 90.0 && lat >= -90.0 && lng <= 180.0 && lng >= -180.0) {
        precision *= 5.0;

        Interval lat_interval = {MAX_LAT, MIN_LAT};
        Interval lng_interval = {MAX_LONG, MIN_LONG};

        Interval *interval;
        double coord;
        int is_even = 1;
        unsigned int hashChar = 0;
        int i;
        for (i = 1; i <= precision; i++)
        {
            double mid;

            if (is_even)
            {
                interval = &lng_interval;
                coord = lng;
            }
            else
            {
                interval = &lat_interval;
                coord = lat;
            }

            mid = (interval->low + interval->high) / 2.0;
            hashChar = hashChar << 1;

            if (coord > mid)
            {
                interval->low = mid;
                hashChar |= 0x01;
            }
            else
                interval->high = mid;

            if (!(i % 5))
            {
                hash[(i - 1) / 5] = char_map[hashChar];
                hashChar = 0;
            }
            is_even = !is_even;
        }
        
        // printf("geohash_encode() hash = %s\n", hash);
    }
    return true;
}

static void replaceStation(gpsHost *host, char *buffer) {
     printf("host GPS ID = %s\n",  host->gpsID);
    // Should we overwrite the station ID?
    if (strlen(host->gpsID) == 4) {
        int comma_cnt = 0;
        for (size_t char_cnt = 0; char_cnt < (strlen(buffer) - 4); ++char_cnt) {
            if (',' == buffer[char_cnt]) {
                if (++comma_cnt == 14) {
                    printf("********************Found 14th***************************\n");
                    printf("host GPS ID = %s\n",  host->gpsID);
                    printf("GPS String = %s\n", buffer);
                    printf("GPS idx = %d\n",  comma_cnt);
                    memcpy(buffer+char_cnt+1, host->gpsID,4);
                    updateChecksumStr(buffer);
                    printf("GPS String update = %s\n", buffer);
                }
            }
        }
    }    
}

// Write GPS info to file so htat it can be read by statusgather.sh
static void writeGPSonoffFile(void)
{
    FILE *fpGPS = fopen("/tmp/nextivity/gps_onoff", "w");
    if (fpGPS)
    {
        bool gpson = false;
    
        if (idxOfNewestGGAMsg >= 0) 
        {
            if ((fabs(gpsMsgArray[idxOfNewestGGAMsg].latitude) > 0.0001) || 
                (fabs(gpsMsgArray[idxOfNewestGGAMsg].longitude) > 0.0001))
            {
                gpson = true;
            }
        }

        if (gpson) 
        {
            fprintf(fpGPS, "ON\n");
        }
        else
        {
            fprintf(fpGPS, "OFF\n");
        }
        fclose(fpGPS);
    }

}

// Write GPS info to file so htat it can be read by statusgather.sh
static void writeGPSStatusFile(void)
{
    if (idxOfNewestGGAMsg < 0) 
	{
        return;
    }
    FILE *fp = fopen(GPS_STATUS_FILE, "wb");

    if (!fp)
    {
        fprintf(stderr, "Failed to open GPS Status file: %s\n", strerror(errno));
        return;
    }

    // Initialize GPS to zeros
    float lat_f = 0.0;
    float lon_f = 0.0;
    float alt_f = 0.0;
    char NMEA_message[MAX_MSG_SIZE + 1];
    strcpy(NMEA_message, "$GPGGA,000000.000,0000.00000,N,00000.00000,E,0,00,0.0,0.0,M,0.0,M,,0000*6D");

    // Use most recent $GPGGA Message
    if (idxOfNewestGGAMsg >= 0) {
        lat_f = gpsMsgArray[idxOfNewestGGAMsg].latitude;
        lon_f = gpsMsgArray[idxOfNewestGGAMsg].longitude;
        alt_f = gpsMsgArray[idxOfNewestGGAMsg].altitude;
        strcpy(NMEA_message, gpsMsgArray[idxOfNewestGGAMsg].message);
        printf("writeGPSStatusFile(): %.6f, %.6f\n", lat_f, lon_f);
    } else {
        return;
    }

    int precision = GEOHASHPRECISION;
    char hash[GEOHASHPRECISION + 2];
    memset(hash, 0, GEOHASHPRECISION + 2);
    if (!geohash_encode(lat_f, lon_f, hash, precision) ||
        ((fabs(lat_f) < 0.0001) && (fabs(lon_f) < 0.0001))) 
		{
        strcpy(hash, "7zzzzzzzzzzz");
    }

    time_t now = time(NULL);

    char msg_buffer[TAIP_MSG_SIZE];
    if (hosts[0].output == gpsOutputTAIP)
    {
        strcpy(msg_buffer, latestTAIPSentence);
    }
    else 
    {
        strcpy(msg_buffer,  gpsMsgArray[idxOfNewestGGAMsg].message);
        replaceStation(&hosts[0], msg_buffer);
    }

    fprintf(fp, "{\n");
    fprintf(fp, "    \"name\": \"Lat_f\",\n");
    fprintf(fp, "    \"value\": \"%.6f\",\n", lat_f);
    fprintf(fp, "    \"lastChange\": \"%ld\",\n", now);
    fprintf(fp, "    \"lastUpdate\": \"%ld\",\n", now);

    fprintf(fp, "},\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    \"name\": \"Lon_f\",\n");
    fprintf(fp, "    \"value\": \"%.6f\",\n", lon_f);
    fprintf(fp, "    \"lastChange\": \"%ld\",\n", now);
    fprintf(fp, "    \"lastUpdate\": \"%ld\",\n", now);
    fprintf(fp, "},\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    \"name\": \"Alt_f\",\n");
    fprintf(fp, "    \"value\": \"%.6f\",\n", alt_f);
    fprintf(fp, "    \"lastChange\": \"%ld\",\n", now);
    fprintf(fp, "    \"lastUpdate\": \"%ld\",\n", now);
    fprintf(fp, "},\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    \"name\": \"GeoHash_s\",\n");
    fprintf(fp, "    \"value\": \"%s\",\n", hash);
    fprintf(fp, "    \"lastChange\": \"%ld\",\n", now);
    fprintf(fp, "    \"lastUpdate\": \"%ld\",\n", now);
    fprintf(fp, "},\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    \"name\": \"NMEA_s\",\n");
    fprintf(fp, "    \"value\": \"%s\",\n", msg_buffer);
    fprintf(fp, "    \"lastChange\": \"%ld\",\n", now);
    fprintf(fp, "    \"lastUpdate\": \"%ld\",\n", now);
    fprintf(fp, "}\n");

    fclose(fp);
}

#if WRITE_DEBUG_MSG_FILES
static void writeGPSBuffToFile(void) {
    static int fileIndex = 1;

    char fileName[64];
    sprintf(fileName, "/tmp/nextivity/uart_buff_%d", fileIndex);
    if (fileIndex++ >= 32)
        fileIndex = 1;

    FILE *fp = fopen(fileName, "wb");

#if WRITE_DEBUG_MSG_TO_LOG
    // -- logging -- for debugging only!
    char logbuffer[256];
    snprintf(logbuffer, sizeof(logbuffer), "Wrtiting UART buffer to file: %s", fileName);
    LogMessage(logbuffer);
#endif

    if (!fp) {
        fprintf(stderr, "Failed to open post status: %s\n", strerror(errno));
        return;
    }
    for (int i = 0; i < GPS_BUFF_SIZE; ++i)
        fprintf(fp, "%c", gpsBuff[i]);
    fclose(fp);

    sleep(1);
}

static void writeGPSMsgsToFile(void) {
    static int gpsfileIndex = 1;

    char fileName[64];
    sprintf(fileName, "/tmp/nextivity/gps_msgs_%d", gpsfileIndex);
    if (gpsfileIndex++ >= 32)
        gpsfileIndex = 1;

#if WRITE_DEBUG_MSG_TO_LOG
    // -- logging -- for debugging only!
    char logbuffer[256];
    snprintf(logbuffer, sizeof(logbuffer), "Wrtiting GPS msgs to file: %s", fileName);
    LogMessage(logbuffer);
#endif

    FILE *fp = fopen(fileName, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open post status: %s\n", strerror(errno));
        return;
    }

    time_t now = time(NULL);
    fprintf(fp, "Current Time = %d\n", (int)now);
    for (int i = 0; i < NUM_GPS_MESSAGES; ++i) {
        if (gpsMsgArray[i].sendThisMsg) {
            fprintf(fp, "GPS Message   %d: %s\n Type           : %d\n Time           : %d\n Lat            : %.3f\n Lon            : %.3f\n Speed          : %.1f\n Bearing        : %.1f\n-\n",
                    i,
                    gpsMsgArray[i].message,
                    gpsMsgArray[i].msgTyp,
                    gpsMsgArray[i].utcTimeSec,
                    gpsMsgArray[i].latitude,
                    gpsMsgArray[i].longitude,
                    gpsMsgArray[i].speedMPH,
                    gpsMsgArray[i].bearing);
        }
    }
    fprintf(fp, "\nTAIP sentence:\n%s\n\n", latestTAIPSentence);
    fclose(fp);
}
#endif

static void buildTAIPSentence(gpsHost *host)
{
    memset(latestTAIPSentence, 0, TAIP_MSG_SIZE);

    // NMEA:             4124.8963,N   = 41 degrees, 24.8963 minutes North.
    // DMS.s:            412453.778,N  = 41 degrees, 25 minutes, 54 seconds North
    // Degrees Decimal:  41.414938

    // TAIP uses degrees decimal * 1000, with a leading +/ and 7 digits for
    // latitude and 8 digits for longitude with leading 0 padding in both cases.
    if (idxOfNewestGGAMsg < 0) {
        return;
    }

    int timeOfDay = gpsMsgArray[idxOfNewestGGAMsg].utcTimeSec;

    char lat[16];
    double curr_latitude = fabs(gpsMsgArray[idxOfNewestGGAMsg].latitude);

    double lat_f = curr_latitude * 100000.0;
    snprintf(lat, sizeof(lat), "%+08d", (int) lat_f);

    char lon[16];
    double curr_longitude = fabs(gpsMsgArray[idxOfNewestGGAMsg].longitude);
    double lon_f = curr_longitude * 100000.0;
    snprintf(lon, sizeof(lon), "%+09d", (int)lon_f);

    size_t id = atoi(gpsMsgArray[idxOfNewestGGAMsg].stationID); // "0001"; // 4 digits
    char stationID[5];
    snprintf(stationID, sizeof(stationID), "%04zu", id);

    /*
     * This is the format we are aiming for - 47 bytes
     *
     * >RPV15714+3739438-1220384601512612;ID=1234;*7F<
     * >RPV76393+4301810-0743503600001132;ID=3214;*74<
     *
     */

    double mph = 0;  // Not in NMEA message  = "6.70557"; // double mph = _speed_ms * 2.23694; //convert M/S to MPH
    int heading = 0; // Not in NMEA message - 3 Digits
    if (idxOfNewestVTGMsg > -1) {
        mph = gpsMsgArray[idxOfNewestVTGMsg].speedMPH;
        heading = gpsMsgArray[idxOfNewestVTGMsg].bearing;
    }

    size_t dataSource = 3; // 0 = 2d gps, 1 = 3d gps, 2 = 2d gps, 3=3d gps, 6 = dr, 8 = degraded DR, 9 = unknown
    size_t dataAge = 2;    // 0 = not available, 1 = old > 10 seconds, 2 = fresh < 10 sec

    size_t blen = snprintf(latestTAIPSentence, sizeof(latestTAIPSentence), ">RPV%05d%s%s%03d%03d%zu%zu;ID=%s;*",
                           timeOfDay,
                           lat,
                           lon,
                           (int)mph,
                           heading,
                           dataSource,
                           dataAge,
                           strlen(host->gpsID) ? host->gpsID : stationID);

    unsigned char checksum = 0;
    for (size_t pos = 0; pos < blen; pos++)
    {
        checksum ^= latestTAIPSentence[pos];
    }
    snprintf(latestTAIPSentence + blen, sizeof(latestTAIPSentence) - blen, "%02X<", checksum);
}

static bool sendHostData(gpsHost *host, bool sendToLuci)
{
    int sresult = 0;
    size_t len = 0;
    size_t totalLength = 0;

#if PRINT_MSG_TO_HOST
    printf("**************************************************\n");
    printf("Sending the following messages ot the host - START\n");
#endif

    char msg_buffer[MAX_MSG_SIZE];

    if (host->output == gpsOutputNMEA)
    {
        if (sendToLuci)
        {
            if (idxOfNewestGGAMsg >= 0) 
            {
                strcpy(msg_buffer, gpsMsgArray[idxOfNewestGGAMsg].message);
                replaceStation(host, msg_buffer);
#if PRINT_MSG_TO_HOST
                printf("%s\n",msg_buffer);
#endif                
                len = strlen(msg_buffer);
                sresult = sendData(host, msg_buffer, len);
                
                if (sresult < 0) 
                {
                    return false;
                }
            } 
            else 
            {
                return false;
            }
        } 
        else 
        {
            for (int i = 0; i < NUM_GPS_MESSAGES; ++i) 
            {
                if (gpsMsgArray[i].sendThisMsg) 
                {
    				strcpy(msg_buffer, gpsMsgArray[i].message);

	                if (G_GGA == gpsMsgArray[i].msgTyp) {
	                    replaceStation(host, msg_buffer);
	                }					
					
#if PRINT_MSG_TO_HOST
                    printf("%s\n",msg_buffer);
#endif
                    size_t len = strlen(msg_buffer);
                    
                    if ((host->connection != gpsConnectionInternal) && (len + 2 < MAX_MSG_SIZE)) 
                    {
                        // Re-add the LF for NMEA, whether TCP or UDP. It shouldn't be needed for UDP, but GPSGate needs it.
	                    msg_buffer[len++] = '\r';
	                    msg_buffer[len++] = '\n';
	                    msg_buffer[len] = '\0';
                    }

                    // sresult = sendData(host, gpsMsgArray[i].message, len);
                    memcpy(&hostbuffer[totalLength], msg_buffer, len);
                    totalLength += len;
                    if (totalLength >= HOSTS_BUFFER_SIZE)
                        break;
                }
            }
            sresult = sendData(host, hostbuffer, totalLength);
        }
    }
    else if (host->output == gpsOutputTAIP)
    {
        buildTAIPSentence(host);
#if PRINT_MSG_TO_HOST
        printf("%s\n",latestTAIPSentence);
#endif
        len = strlen(latestTAIPSentence);
        if (len)
        {
            //printf("Send to LUCI %s\n",latestTAIPSentence);
            sresult = sendData(host, latestTAIPSentence, len);
        }
        else
        {
            sresult = 1;
        }
    }
    else
    {
        return false;
    }

#if PRINT_MSG_TO_HOST
    printf("Sending the following messages ot the host - END\n");
    printf("************************************************\n");
#endif

    return (sresult > 0);
}

static void parseGPSBuffer(void) 
{
    // printf("parseGPSBuffer() at %d\n",(int)time(NULL));
    // replace null characters with spaces so that string functions will work
    for (int i = 0; i < GPS_BUFF_SIZE; ++i) 
    {
        if (gpsBuff[i] < ' ') {
            gpsBuff[i] = ' ';
        }
    }

    int gpsMsgIDX = 0;
    char *gpsStart = strstr(&gpsBuff[0], "$");
    printf("GPS buf: \n %s\n", gpsBuff);

    // Clear gpsMsgArray[]
    resetGPSMessages();

    idxOfNewestVTGMsg = -1;
    idxOfNewestGGAMsg = -1;

    // Look for '$'
    while (gpsStart) 
    {
        char *gpsEnd = strstr(gpsStart + 1, "*");
        
        // Look for '*'
        if (gpsEnd) 
        {
            int offsetInBuff = 3 + gpsEnd - gpsBuff;
            // printf("offset in buffer = %d\n", offsetInBuff);
            if (offsetInBuff >= GPS_BUFF_SIZE) 
            {
                break;
            }

            // Add 3 to include the '$' and the two checksum chars
            int msgSize = 3 + gpsEnd - gpsStart;
            
            if (msgSize > MIN_MSG_SIZE && msgSize <= MAX_MSG_SIZE && gpsMsgIDX < NUM_GPS_MESSAGES) 
            {
                strncpy((char *)&gpsMsgArray[gpsMsgIDX].message, gpsStart, msgSize);
                gpsMsgArray[gpsMsgIDX].message[msgSize] = '\0';

                // Check the checksum value
                char chckSumStr[3];
                sprintf(chckSumStr, "%2X", checksumNMEA(gpsMsgArray[gpsMsgIDX].message, msgSize - 3));
                
                // printf("Checksum in string: %s\n", &gpsMsgArray[gpsMsgIDX].message[msgSize-2]);
                if (strcmp(chckSumStr, &gpsMsgArray[gpsMsgIDX].message[msgSize - 2]) == 0) 
                {
                    gpsMsgArray[gpsMsgIDX].sendThisMsg = true;

                    // printf("Found Message Checksum PASS: size = %d, type = %d, checksum = %s, msg = %s\n", msgSize, gpsMsgArray[gpsMsgIDX].msgTyp, chckSumStr, gpsMsgArray[gpsMsgIDX].message);
                    parseNMEA(&gpsMsgArray[gpsMsgIDX]);

                    if ((G_GGA == gpsMsgArray[gpsMsgIDX].msgTyp) && (idxOfNewestGGAMsg < gpsMsgIDX)) 
                    {
                        idxOfNewestGGAMsg = gpsMsgIDX;
                    }

                    if ((G_VTG == gpsMsgArray[gpsMsgIDX].msgTyp) && (idxOfNewestVTGMsg < gpsMsgIDX)) 
                    {
                        idxOfNewestVTGMsg = gpsMsgIDX;
                    }

                    // Convert Global Navigation to Global Positioning
                    if ((G_GGA == gpsMsgArray[gpsMsgIDX].msgTyp) || (G_VTG == gpsMsgArray[gpsMsgIDX].msgTyp) || (G_RMC == gpsMsgArray[gpsMsgIDX].msgTyp))
                    {
                        if ('N' == gpsMsgArray[gpsMsgIDX].message[2]) 
                        {
                            gpsMsgArray[gpsMsgIDX].message[2] = 'P';
                            updateChecksumStr(gpsMsgArray[gpsMsgIDX].message);
                        }
                    }

                    // Only if message is valid
                    gpsStart = gpsEnd;
                    gpsMsgIDX++;
                }
                // else 
                // {
                //     printf("Found Message Checksum FAIL: size = %d, checksum = %s, msg = %s\n", msgSize, chckSumStr, gpsMsgArray[gpsMsgIDX].message);
                // }
            }
        }
        gpsStart = strstr(gpsStart + 1, "$");
    }
}

static void pipeHandler(int signal) {
    (void)signal;
}

static void *gpsSocketHandler(void *arg) {
  StatusInfo *psi = (StatusInfo *)arg;
  struct pollfd pfd;
  
  while (!exiting) {  
    pfd.fd = psi->gpsFD;
    pfd.events = POLLIN;
  
    if (poll(&pfd, 1, 200) > 0) {
      char buffer[265];
    
      struct sockaddr_un peerSock;
      socklen_t addrLen;
      
      int result = recvfrom(psi->gpsFD, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&peerSock, &addrLen);
      
      if (psi->bdebug) printf("GPS recvfrom: %d\n", result);
    
      if (result > 0) {
        buffer[result] = '\0';
        // requires statusgather
        // LocationUpdate(psi, buffer);
      
        if (psi->bdebug) printf("GPS Value: '%s'\n", buffer);
      }
    }
  }
  
  LogMessage("Exit gpsSocketHandler");
  
  return NULL;
}


static bool gpsSocketInit(StatusInfo *si) {
  
  si->gpsFD = socket(PF_LOCAL, SOCK_DGRAM, 0);
    
  if (si->gpsFD == -1) {
    printf("Unable to create local socket");
    fprintf(stderr, "Unable to create local socket");
    return false;
  }
  
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));

  addr.sun_family = AF_UNIX;
  
  unlink("/tmp/nextivity/gps.socket");
  
  strncpy(addr.sun_path, "/tmp/nextivity/gps.socket", sizeof(addr.sun_path) - 1);
 
  if (bind(si->gpsFD, (struct sockaddr*)&addr, sizeof(addr))) {
    LogError("Could not bind GPS socket");
    return false;
  }
  
  printf("GPS socket created: %d\n", si->gpsFD);

  pthread_create(&si->gpsThread, NULL, gpsSocketHandler, si);
  
  return true;
} 

static long currentMillis() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void gpsSocketDeinit(StatusInfo *si) {
//  puts("Calling pthread_join\n");
  pthread_join(si->gpsThread, NULL);
  if (si->gpsFD != -1) {
    close(si->gpsFD);
    si->gpsFD = -1;
  }
 // puts("Done");
}

#define MAX_DEV_STR_LEN 32
int main(int argc, const char *argv[]) {
    const char *command = argv[0];
    const char *device = NULL;

    bool testing = false;
    bool dummy = false;
    uint16_t serverPort = 0;
    const char *serverAddress = NULL;

    char logbuffer[256];

    LogMessage("Starting...");

    // Socket initialization from statusgather
    StatusInfo si;
    memset(&si, 0, sizeof(si));

    if (!gpsSocketInit(&si)) {
        LogError("GPS socket initialization failed");
        exit(EXIT_FAILURE);
    }

    LogMessage("GPS socket initialized");

    // Initialize GPS messages
    resetGPSMessages();
    memset(hosts, 0, sizeof(hosts));

    for (size_t hostNum = 0; hostNum < MAX_HOSTS; hostNum++) {
        gpsHost *host = &hosts[hostNum];
        host->sock = -1;
        host->port = 5555;
        host->connection = gpsConnectionDisabled;
        host->output = gpsOutputDisabled;
    }

    for (size_t clientNum = 0; clientNum < MAX_CLIENTS; clientNum++) {
        gpsClient *client = &clients[clientNum];

        client->sock = -1;
    }

    memset(&localAddress, 0, sizeof(localAddress));
    localAddress.sun_family = AF_UNIX;

    strncpy(localAddress.sun_path, "/tmp/nextivity/gps.socket", sizeof(localAddress.sun_path) - 1);

    // SIGPIPE causes an error, not a signal
    signal(SIGPIPE, pipeHandler);

    hosts[0].connection = gpsConnectionTCP;

    // Start parse command line arguments
    snprintf(logbuffer, sizeof(logbuffer), "%d %s\n", argc, argv[1]);
    while (argc > 1) {
        LogMessage(logbuffer);
        if (strcmp(argv[1], "-t") == 0) {
            // Don't send data; just output the conversion
            testing = true;
            argc--;
            argv++;
            continue;
        } else if (strcmp(argv[1], "-dummy") == 0) {
            dummy = true;
            argc--;
            argv++;
            continue;

        } else if (strcmp(argv[1], "-u") == 0) {
            hosts[0].connection = gpsConnectionUDP;

            argc--;
            argv++;
            continue;

        } else if (argc > 2) {
            if (strcmp(argv[1], "-sp") == 0) {
                serverPort = atoi(argv[2]);

            } else if (strcmp(argv[1], "-sa") == 0) {
                serverAddress = argv[2];

            } else if (strcmp(argv[1], "-d") == 0) {
                device = argv[2];
            } else if (strcmp(argv[1], "-p") == 0) {
                hosts[0].port = atoi(argv[2]);

            } else if (strcmp(argv[1], "-c") == 0) {
                strncpy(hosts[0].ip, argv[2], sizeof(hosts[0].ip) - 1);

            } else if (strcmp(argv[1], "-r") == 0) {
                hosts[0].rate = atoi(argv[2]);

            } else if ((strcmp(argv[1], "-tid") == 0) || (strcmp(argv[1], "-id") == 0)) {
                strncpy(hosts[0].gpsID, argv[2], sizeof(hosts[0].gpsID) - 1);

            } else if (strcmp(argv[1], "-f") == 0) {
                if (strcmp(argv[2], "nmea") == 0) {
                    hosts[0].output = gpsOutputNMEA;

                } else if (strcmp(argv[2], "taip") == 0) {
                    hosts[0].output = gpsOutputTAIP;

                } else {
                    LogError("Output format unspecified");
                    usage(command);
                }

            } else if (strcmp(argv[1], "-h") == 0) {
                size_t num = 0;
                size_t offset = 0;
                const char *start = argv[2];

                do {
                    char c = argv[2][offset];
                    if (c == ',' || c == '\0') {
                        size_t len = offset - (start - argv[2]);
                        switch (num) {
                            case 0:
                                strncpy(hosts[numHosts].ip, start, (len > sizeof(hosts[numHosts].ip) ? sizeof(hosts[numHosts].ip) - 1 : len));
                                break;

                            case 1:
                                hosts[numHosts].port = atoi(start);
                                break;

                            case 2:
                                if (strncmp(start, "tcp", 3) == 0) {
                                    hosts[numHosts].connection = gpsConnectionTCP;

                                } else if (strncmp(start, "udp", 3) == 0) {
                                    hosts[numHosts].connection = gpsConnectionUDP;

                                } else if (strncmp(start, "internal", 8) == 0) {
                                    hosts[numHosts].connection = gpsConnectionInternal;

                                } else {
                                    snprintf(logbuffer, sizeof(logbuffer), "Invalid protocol: %s\n", start);
                                    LogError(logbuffer);
                                    usage(command);
                                }
                                break;

                            case 3:
                                if (strncmp(start, "nmea", 4) == 0) {
                                    hosts[numHosts].output = gpsOutputNMEA;

                                } else if (strncmp(start, "taip", 4) == 0) {
                                    hosts[numHosts].output = gpsOutputTAIP;

                                } else {
                                    snprintf(logbuffer, sizeof(logbuffer), "Invalid GPS format: %s\n", start);
                                    LogError(logbuffer);
                                    usage(command);
                                }
                                break;

                            case 4:
                                hosts[numHosts].rate = atoi(start);
                                break;

                            case 5:
                                strncpy(hosts[numHosts].gpsID, start, sizeof(hosts[numHosts].gpsID) - 1);
                                break;

                            default:
                                snprintf(logbuffer, sizeof(logbuffer), "Excess arguments: %s\n", start);
                                LogError(logbuffer);
                                usage(command);
                                break;
                        }

                        num++;
                        if (c == '\0') break;
                        offset++;
                        start = argv[2] + offset;
                    }
                    offset++;
                } while (true);

                // Last 3 items are optional
                if (num < 4 || num > 6) {
                    snprintf(logbuffer, sizeof(logbuffer), "Number of arguments must be between 4 and 6 (%zu)\n", num);
                    LogError(logbuffer);
                    usage(command);
                }

                numHosts++;

            } else {
                LogError("Settings parsing");
                usage(command);
            }
        } else {
            LogError("More than 2 arguments required");
            usage(command);
        }

        argc -= 2;
        argv += 2;
    }
    // End parse command line arguments

    // Legacy single instance specification
    if (strlen(hosts[0].ip) && numHosts == 0) {
        numHosts = 1;
    }

    if ((argc > 1) || !(device || dummy) || (!numHosts && (serverPort == 0))) {
        LogError("Failed to get proper settings");
        usage(command);
    } 
    if (device) {
        snprintf(logbuffer, sizeof(logbuffer), "argc: %d, device: %s, dummy: %d, numHosts: %ld, serverPort: %d\n", 
            argc, device, dummy, numHosts, serverPort);
        LogMessage(logbuffer);
    }

    // -- logging --
    snprintf(logbuffer, sizeof(logbuffer), "Speedway GPS relay - Reading from %s sending to:", dummy ? "(test data)" : device);
    LogMessage(logbuffer);

    if (serverPort != 0) {
        if (!serverAddress) {
            LogError("No server address specified");
            usage(command);
        }

        if (createServer(serverAddress, serverPort)) {
            printf("    Server listen on address %s port %d\n", serverAddress, serverPort);
        } else {
            exit(EXIT_FAILURE);
        }
    }

    if (numHosts == 0) {
        printf("    No clients configured\n");
    }

    for (size_t hostNum = 0; hostNum < numHosts; hostNum++) {
        gpsHost *host = &hosts[hostNum];

        char rateString[64] = "";

        if (host->rate > 0) {
            snprintf(rateString, sizeof(rateString), "every %zu seconds", host->rate);
        } else {
            snprintf(rateString, sizeof(rateString), "continuously");
        }

        const char *connection;
        const char *output;

        if (host->connection == gpsConnectionTCP) {
            connection = "TCP";
        } else if (host->connection == gpsConnectionUDP) {
            connection = "UDP";
        } else if (host->connection == gpsConnectionInternal) {
            connection = "Internal";
        } else {
            connection = "Disabled";
        }

        if (host->output == gpsOutputNMEA) {
            output = "NMEA";
        } else if (host->output == gpsOutputTAIP) {
            output = "TAIP";
        } else {
            output = "Disabled";
        }

        if (host->connection == gpsConnectionInternal) {
            snprintf(logbuffer, sizeof(logbuffer), "    Internal connection with format %s %s",
                     output,
                     rateString);

        } else {
            snprintf(logbuffer, sizeof(logbuffer), "    Client at %.32s using port %d with protocol %s in format %s %s",
                     testing ? "(stdout testing)" : host->ip,
                     host->port,
                     connection,
                     output,
                     rateString);
        }

        // -- logging --
        LogMessage(logbuffer);
    }

    while (true) {
        // -- logging -- for debugging only!
#if WRITE_DEBUG_MSG_TO_LOG
        snprintf(logbuffer, sizeof(logbuffer), "Calling acceptServer() 1 ");
        LogMessage(logbuffer);
#endif
        acceptServer();  // Possible pending connections

        // -- logging -- for debugging only!
#if WRITE_DEBUG_MSG_TO_LOG
        snprintf(logbuffer, sizeof(logbuffer), "Calling fopen()");
        LogMessage(logbuffer);
#endif

        FILE *serial = fopen(device, "r");
        int serial_port = 0;

        if (serial != NULL) {
            serial_port = fileno(serial);
            if(serial_port < 0){
                perror("fileno error");
            }
            struct termios tty;
            if(tcgetattr(serial_port, &tty) != 0) {
                printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            cfsetispeed(&tty, B115200);
            if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
                printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        int strpos = -1;
        if (serial != NULL) 
        {
            // char timestr[32];
            char mgrTypStr[4];
            long lastbursttime = currentMillis();
            while (true) {
                // -- logging -- for debugging only!
#if WRITE_DEBUG_MSG_TO_LOG
                snprintf(logbuffer, sizeof(logbuffer), "Calling acceptServer() 2");
                LogMessage(logbuffer);
#endif
                acceptServer();
                time_t now = time(NULL);
                // Serial handling
                //long timestamp = currentMillis();
                memset(gpsBuff, 0, GPS_BUFF_SIZE);

                int result = 0;
                int prevresult = 0;
                size_t offset = 0;
                // Serial handling
                while (offset < sizeof(gpsBuff) - 1) 
                {
                    result = fgetc(serial);

                    if (result < 0) {
                        // Serial disconnect, etc
                        fprintf(stderr, "Error during serial port read: '%s' - reopening\n", strerror(errno));
                        break;
                    }

                    if (result == '$')
                    {
                        strpos = 1;
                        // sprintf(timestr, "%ld",  currentMillis());
                        // for (int ii=0; ii < (int)strlen(timestr); ++ii)
                        // {
                        //     gpsBuff[offset++] = timestr[ii];
                        // }
                        // gpsBuff[offset++] = ':';
                    }
                    else if (strpos >= 1)
                    {                         
                        if (strpos >= 3 && strpos <= 5)
                        {
                            mgrTypStr[strpos-3] = result;
                            mgrTypStr[strpos-3+1] = '\0';
                        }

                        if (strpos <= 5)
                            strpos++;
                        else
                            strpos=0;
                    }

                    gpsBuff[offset++] = result;

                    // The speedway GPS ends the sentences with "\n\n"
                    // This really should be "\r\n" but it is not.
                    if ((result == '\n') && ((prevresult == '\n') || (prevresult == '\r')))
                    {
                        // Burst ends with $GNZDA or if 2 sec or more has passed since the last burst
                        if ((strncmp(mgrTypStr,"ZDA",3) == 0) || (currentMillis() - lastbursttime > 2000))
                        {
                            break;
                        }
                    }

                    prevresult = result;
                }

                lastbursttime = currentMillis();
                
                // Parse the uart buffer, gpsBuff[], into the GPS messages, gpsMsgArray[]
                parseGPSBuffer();

#if WRITE_DEBUG_MSG_FILES
                // Write GPS messages to file - for debugging only!
                writeGPSMsgsToFile();
                // Write UART to file - for debugging only!
                writeGPSBuffToFile();
#endif
                // Write GPS on off file for LCD exe
                writeGPSonoffFile();

                // Write GPS info to file so that it can be read by statusgather.sh
                writeGPSStatusFile();

                // Send all data, without processing or rate control to all listening clients in NMEA format
                sendClients();

                size_t numSent = 0;

                for (size_t hostNum = 0; hostNum < numHosts; hostNum++) {
                    gpsHost *host = &hosts[hostNum];
                    //          printf("%d %d\n", hostNum, host->sock);

                    if (host->sock == -1) {
                        int diff = now - host->lastConnect;

                        if (diff >= 0 && diff <= 5) {
                            // 5 second delay before trying again
                            continue;
                        }

                        host->lastConnect = now;

                        if (!connectServer(host)) {
                            continue;
                        }
                    }

                    if (host->rate > 0) {
                        int diff = now - host->lastSend;

                        //            printf("last send diff: %d\n", diff);

                        // Time not up yet, don't send for this host
                        if (diff >= 0 && diff < (int)host->rate) {
                            // printf("Time not up: %d\n", diff);
                            continue;
                        }
                    }

                    // int lastPosDiff = now - lastPosTime;

                    if (sendHostData(host, 0 == hostNum)) {
                        host->lastSend = now;
                        numSent++;
                    }
                }
            }
            fclose(serial);
        }
        sleep(10);
    }

    exiting = true;
    gpsSocketDeinit(&si);

    return EXIT_SUCCESS;
}
