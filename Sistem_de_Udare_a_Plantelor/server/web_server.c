#include "web_server.h"
#include "lwip/apps/httpd.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"

float umidity_percentage = 0.0; //variabila globala pentru a stoca procentul de umiditate

static const char *ssi_tags[] = {"humidity", "status", "uptime"};

//trimitem date catre pagina web folosind Server Side Includes (SSI)
u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    if (iIndex == 0) { // "humidity" tag
        return snprintf(pcInsert, iInsertLen, "%.2f", umidity_percentage);
    }
    else if (iIndex == 1) { // "status" tag
        return snprintf(pcInsert, iInsertLen, "Online");
    }
    else if (iIndex == 2) { // "uptime" tag
        uint32_t secunde = to_ms_since_boot(get_absolute_time()) / 1000;
        return snprintf(pcInsert, iInsertLen, "%d sec", secunde);
    }
    return 0; // tag necunoscut
}

//primit date de la pagina web folosind Common Gateway Interface (CGI)
const char *cgi_handler_irigate(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    for(int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "state") == 0) {
            if (strcmp(pcValue[i], "1") == 0) {
                printf("Sistem de udare pornit!\n");
            }
            else if (strcmp(pcValue[i], "0") == 0) {
                printf("Sistem de udare oprit!\n");
            }
        }
    }
   return "/index.shtml";
}

static const tCGI cgi_handlers[] = {
    {"/irigate", cgi_handler_irigate}
};

void start_web_server(const char *ssid, const char *pass) {
    if (cyw43_arch_init()) {
        printf("Eroare Wi-Fi\n");
        return;
    }

    cyw43_arch_enable_ap_mode(ssid, pass, CYW43_AUTH_WPA2_AES_PSK);

    httpd_init();
    http_set_ssi_handler(ssi_handler, ssi_tags, 3);
    http_set_cgi_handlers(cgi_handlers, 1);
}