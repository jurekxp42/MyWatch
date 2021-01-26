
// my_WiFi.h
//
#ifndef __MYWIFI_H__
#define __MYWIFI_H__

#ifdef __WIFIMAIN__
#define EXTERN
#define INIT(x) = x
#else
#define EXTERN extern
#define INIT(x)
#endif

typedef struct WiFiAP {
    char *ssid;
    char *pass;
    uint32_t tzone;
} WIFIAP;

EXTERN WIFIAP AccessPoints[]
#ifdef __WIFIMAIN__
= {
    { "MayBe5", "Lan4Emmerich", TZ_CET },		// Newark, NJ
    { "hal9000", "diebels23", TZ_CET },	// my phone hotspot
    { "IFM-GAST", "ifmGastX1999", TZ_CET }	// my job's WiFi
    
}
#endif
;

EXTERN int sizeof_Access_Points INIT(sizeof(AccessPoints)/sizeof(WIFIAP));

typedef struct WiFiAp {
    char ssid[33];
    char pass[65];
    uint8_t macAddr[6];
    uint8_t channel;
    uint32_t tzone;
} WiFiap;

EXTERN WiFiap BestAP;
EXTERN boolean connected INIT(false);
EXTERN int number_of_networks;
EXTERN int num_saved_ap, best_ap;

int read_acc_pts_file_and_compare_with_SSIDs (struct WiFiAp *);
int connect_to_wifi(boolean, struct WiFiAp *, boolean, boolean);
int connect_to_wifi_and_get_time (boolean);
void append_new_access_point (char *, char *, uint32_t );
void close_WiFi(void);

#endif
