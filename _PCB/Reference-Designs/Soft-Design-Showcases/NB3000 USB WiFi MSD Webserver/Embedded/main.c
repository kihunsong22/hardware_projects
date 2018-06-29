
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <timing.h>
#include <util_string.h>

#include "devices.h"

#include <per_ioport.h>

#include <lwip.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include "lwip/api.h"

#include "profile.h"

#include <ethernet.h>

#include <usbhost.h>
#include <swp_ifconfig.h>
#include "WebServerDemo.h"

////////////////////////////////////////////////////////////////////////////////
#define LEDS (*IOPORT_BASE8(per_ioport_get_base_address(PRTIO)))
#define LED_ON(nr) LEDS = (LEDS & ~(1 << nr)) | (1 << nr)
#define LED_OFF(nr) LEDS = (LEDS & ~(1 << nr))
#define LED_STATUS(nr) (LEDS & (1 << nr))
#define LED_INVERT(nr) { if (LED_STATUS(nr)) LED_OFF(nr); else LED_ON(nr); }

////////////////////////////////////////////////////////////////////////////////
pthread_mutex_t led_mutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////
static usbhost_t  *USBHost;

////////////////////////////////////////////////////////////////////////////////
void *blink1(void *arg)
{
    struct timespec ts;

    for (;;)
    {
        ts.tv_sec = 0;
        ts.tv_nsec = 1000 * 1000 * 250;
        nanosleep(&ts, NULL);

        pthread_mutex_lock(&led_mutex);
        LED_INVERT(0);
        pthread_mutex_unlock(&led_mutex);
    }

    /* never reached */
}

////////////////////////////////////////////////////////////////////////////////
void *blink2(void *arg)
{
    struct timespec ts;

    for (;;)
    {
        ts.tv_sec = 0;
        ts.tv_nsec = 1000 * 1000 * 225;
        nanosleep(&ts, NULL);

        pthread_mutex_lock(&led_mutex);
        LED_INVERT(1);
        pthread_mutex_unlock(&led_mutex);
    }

    /* never reached */
}

////////////////////////////////////////////////////////////////////////////////
static void print_ip(lwip_t * lwip)
{
    struct ip_addr addr;
    static struct ip_addr last_addr;

    addr = lwip_get_local_addr(lwip);

    if(addr.addr != 0 && memcmp(&last_addr, &addr, sizeof(struct ip_addr))){
        printf ("addr = %hu.%hu.%hu.%hu\n",
                          ip4_addr1(&addr),
                          ip4_addr2(&addr),
                          ip4_addr3(&addr),
                          ip4_addr4(&addr));
        last_addr = addr;
    }

}

////////////////////////////////////////////////////////////////////////////////
static void wait_for_and_print_ip_address(lwip_t * lwip)
{
    struct timespec ts = {1,0};
    struct ip_addr addr;

    do{
        addr = lwip_get_local_addr(lwip);
        nanosleep(&ts, NULL);
    }while(addr.addr == 0);

    print_ip(lwip);
}

////////////////////////////////////////////////////////////////////////////////
void lwip_setup(void)
{
    lwip_t * lwip;
    printf("Starting TCP/IP example server...\n");
    lwip = lwip_open(LWIP_1);
    if (!lwip)
    {
        printf("LWIP open failed.\n");
        return;

    }
    if (lwip_start(lwip)!=ERR_OK)
    {
        printf("LWIP start failed.\n");
        return;
    }
    printf("Server Started.\n");

    wait_for_and_print_ip_address(lwip);

    lwip_set_netif_callback(lwip, &print_ip);
}

#define WIFI_CONFIG_FILENAME    "alt_wifi.ini"

#define SECT_WIFI_CONFIG    "WIFI_CONFIG"
#define SECT_WEP_CONFIG     "WEP_CONFIG"
#define SECT_WPA_CONFIG     "WPA_CONFIG"

////////////////////////////////////////////////////////////////////////////////
void config_wifi(ifconfig_t *ifconfig)
{
    int32_t i;
    int32_t chan;
    int8_t  tempBuf[128];
    int8_t  nameBuf[32];
    int32_t crypt=-1;
    bool    generateKeys=true;

    // Poll for a MSD device
    for(i=0;i<10;i++){
        usbhost_process(USBHost);
        delay_ms(100);
    }

    // Attempt to mount MSD
    if(chdir("/usb0") && chdir("/usb1") && chdir("/usb2")){
        printf("No MSD Found - Using SWP WiFi plugin configuration.\n");
        return;
    }

    printf("------------------------------------\n");
    printf("Loading WiFi configuration from MSD.\n");

    SetIniFile(WIFI_CONFIG_FILENAME);

    chan = GetProfileInteger(SECT_WIFI_CONFIG, "Channel", -1);
    GetProfileStr(SECT_WIFI_CONFIG, "SSID"      , "", tempBuf, sizeof(tempBuf));
    if(tempBuf[0]){
        // Set SSID
        printf("Setting SSID: %s\n", tempBuf);
        ifconfig_ieee80211_set_ssid(ifconfig, tempBuf);
    }

    GetProfileStr(SECT_WIFI_CONFIG, "PassPhrase", "", tempBuf, sizeof(tempBuf));
    if(tempBuf[0]){
        // Set Pass Phrase
        printf("Setting passphrase: %s\n", tempBuf);
        ifconfig_ieee80211_set_passphrase(ifconfig, tempBuf);
    }

    if(chan >= 0){
        // Set Channel
        printf("Setting channel: %i\n", chan);
        ifconfig_ieee80211_set_channel(ifconfig, chan);
    }


    GetProfileStr(SECT_WIFI_CONFIG, "Crypt"     , "", tempBuf, sizeof(tempBuf));
    if(tempBuf[0]){
        // Do crypt setup
        if(!strcasecmp(tempBuf, "OPEN")){
            crypt = IFCONFIG_IEEE80211_CRYPT_OPEN;
        }else if(!strcasecmp(tempBuf, "WEP")){
            crypt = IFCONFIG_IEEE80211_CRYPT_WEP;
        }else if(!strcasecmp(tempBuf, "WPA")){
            crypt = IFCONFIG_IEEE80211_CRYPT_WPAPSK;
        }else{
            printf("Invalid crypt option! - Please use only OPEN, WEP or WPA\n");
        }

        switch(crypt){
            case IFCONFIG_IEEE80211_CRYPT_OPEN:
            {
                // Do OPEN Config
                printf("Set IFCONFIG_IEEE80211_CRYPT_OPEN\n");
                generateKeys=false;
                break;
            }
            case IFCONFIG_IEEE80211_CRYPT_WEP:
            {
                // Do WEP Config
                printf("Set IFCONFIG_IEEE80211_CRYPT_WEP\n");
                for(i=0;i<4;i++){
                    sprintf(nameBuf, "Key%i", i);
                    GetProfileStr(SECT_WEP_CONFIG, nameBuf, "", tempBuf, sizeof(tempBuf));
                    if(tempBuf[0]){
                        printf("Set Key[%i]: %s\n", i+1, tempBuf);
                        ifconfig_ieee80211_set_wep_key(ifconfig, tempBuf, i+1);
                        generateKeys=false;
                    }
                }
                break;
            }
            case IFCONFIG_IEEE80211_CRYPT_WPAPSK:
            {
                // Do WPA Config
                printf("Set IFCONFIG_IEEE80211_CRYPT_WPAPSK\n");
                GetProfileStr(SECT_WPA_CONFIG, "Key", "", tempBuf, sizeof(tempBuf));
                if(tempBuf[0]){
                    printf("Set Key: %s\n", tempBuf);
                    ifconfig_ieee80211_set_wpapsk_key(ifconfig, tempBuf);
                    generateKeys=false;
                }

                break;
            }
            default:
            // Do Nothing
            break;
        }

        ifconfig_ieee80211_set_crypt(ifconfig, crypt, generateKeys);
    }
    printf("------------------------------------\n");

}

////////////////////////////////////////////////////////////////////////////////
void init_usb(void)
{
    uint32_t linkState;
    ifconfig_t *ifconfig = NULL;

    printf("Init USB subsystem\n");
    if ((USBHost = usbhost_open(USBHOST_1)) == NULL)
    {
        printf("Failed usbhost_open(USBHOST_1)\n");
        abort();
    }

    printf("Starting USB Host daemon.\n");
    usbhost_daemon();

    printf("Init plugged USB devices\n");
    do{
        ifconfig = ifconfig_get_interface(ethernet_open(ETHERNET_1));
    } while(!ifconfig);
    printf("Found wireless network device\n");

    config_wifi(ifconfig);

    printf("Starting interface....\n");
    ifconfig_link_start(ifconfig);
    do{
        ifconfig_link_get_state(ifconfig, &linkState);
    }while(!(linkState==IFCONFIG_LINK_S_UP));
    printf("Interface Started!\n");

}

////////////////////////////////////////////////////////////////////////////////
void main(void)
{
    pthread_t thread;
    pthread_attr_t attr;

#if 0
    while(1);
#endif

    LEDS = 0x00;

    printf("Kernel LWIP HTTP server test\n");

    init_usb();

    lwip_setup();

    pthread_attr_init(&attr);
    pthread_create(&thread, &attr, blink1, NULL);
    pthread_create(&thread, &attr, blink2, NULL);
    printf("threads running\n");

    http_server(USBHost);

    /* never reached */
}


