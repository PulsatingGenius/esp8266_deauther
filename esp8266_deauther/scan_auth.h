/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#pragma once

void auth_sniffer(uint8_t* buf, uint16_t len) {
    if (data.auth_buffer.locked) return;

    SNIFFER_PREAMBLE();

    if (payload_len == 0) return;

    uint8_t type = payload[0]; // buf[12];

    // drop everything that isn't an authentication frame
    if (type != 0xb0) return;

    const uint8_t* receiver = &payload[4];  // &buf[16]; // To (Receiver)
    const uint8_t* sender   = &payload[10]; // &buf[22]; // From (Transmitter)
    const int8_t   rssi     = ctrl->rssi;

    memcpy(data.auth_buffer.mac, sender, 6);
    memcpy(data.auth_buffer.bssid, receiver, 6);
    data.auth_buffer.rssi   = rssi;
    data.auth_buffer.locked = true;
}

void start_auth_scan() {
    debugF("Scanning for authentications on ");
    debugln(strh::mac(data.bssid));
    debugln();

    data.auth_buffer.locked = false;

    debuglnF("RSSI Vendor   MAC-Address       SSID                             BSSID");
    debuglnF("==================================================================================");

    wifi_set_promiscuous_rx_cb(auth_sniffer);
    wifi_promiscuous_enable(true);
}

void stop_auth_scan() {
    if (data.auth) {
        wifi_promiscuous_enable(false);
        data.auth = false;

        debuglnF("Stopped beacon authentication scan");
        debugln();
    }
}

void update_auth_scan() {
    if (data.auth) {
        unsigned long current_time = millis();

        // Print scan results if something is in the buffer
        if (data.auth_buffer.locked) {
            // Copy data and unlock the buffer
            auth_buffer_t tmp = data.auth_buffer;
            data.auth_buffer.locked = false;

            // print st rssi, mac, vendor
            debug(strh::right(4, String(tmp.rssi)));
            debug(' ');
            debug(strh::left(8, vendor::search(tmp.mac)));
            debug(' ');
            debug(strh::left(17, alias::get(tmp.mac)));
            debug(' ');

            // Part of beacon attack
            if (memcmp(tmp.bssid, data.bssid, 5) == 0) {
                debug(strh::left(32, attack::getBeacon(tmp.bssid[5])));
                debug(' ');
                debugln(strh::left(17, alias::get(tmp.bssid)));
                return;
            }

            // Part of AP list
            AccessPoint* ap = data.ap_list.search(tmp.bssid);
            if (ap) {
                debug(strh::left(32, ap->getSSIDString()));
                debug(' ');
                debugln(strh::left(17, ap->getBSSIDString()));
                return;
            }

            // Unknown AP
            debug(strh::left(32, "*UNKNOWN*"));
            debug(' ');
            debugln(strh::left(17, alias::get(tmp.bssid)));
        }

        if ((data.ch_time > 0) && (current_time - data.ch_update_time >= data.ch_time)) {
            next_ch();
            data.ch_update_time = current_time;
        } else if ((data.timeout > 0) && (millis() - data.start_time >= data.timeout)) {
            stop_auth_scan();
        }
    }
}