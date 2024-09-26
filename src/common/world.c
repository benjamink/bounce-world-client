#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app_errors.h"
#include "data.h"
#include "debug.h"
#include "keyboard.h"
#include "fujinet-network.h"
#include "who.h"
#include "world.h"

char *world_state = "/ws";
char *who_endpoint = "/who";
char *cmd_endpoint = "/cmd/get/";
char *msg_endpoint = "/msg";

void get_world_state() {
	memset(url_buffer, 0, sizeof(url_buffer));
	strcat(url_buffer, endpoint);
	strcat(url_buffer, world_state);

    err = network_open(url_buffer, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
	handle_err("ws_get:open");

	// read directly into the 14 bytes of memory starting at world_width
	network_read(url_buffer, (uint8_t *) &world_width, 14);
	network_close(url_buffer);
}

// get up to 240 bytes for all connected clients. we live in hope
void get_world_clients() {
	memset(url_buffer, 0, sizeof(url_buffer));
	strcat(url_buffer, endpoint);
	strcat(url_buffer, who_endpoint);

	memset(clients_buffer, 0, 240);
	err = network_open(url_buffer, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
	handle_err("get:open:clients");
	network_read(url_buffer, clients_buffer, 240);
	network_close(url_buffer);
}

void get_broadcast() {
	int n;
	memset(url_buffer, 0, sizeof(url_buffer));
	strcat(url_buffer, endpoint);
	strcat(url_buffer, msg_endpoint);

	memset(broadcast_message, 0, 120);
	err = network_open(url_buffer, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
	handle_err("get:open:broadcast");
	n = network_read(url_buffer, broadcast_message, 119);
	network_close(url_buffer);

	if (n > 0) {
		// terminate the string. We only read 119 bytes to not overrun the buffer, but always add a nul terminator.
		broadcast_message[n] = '\0';
	}
}

// this client has some commands to process
void get_world_cmd() {
	int n;
	uint8_t i, cmd;
	memset(url_buffer, 0, sizeof(url_buffer));
	strcat(url_buffer, endpoint);
	strcat(url_buffer, cmd_endpoint);
	strcat(url_buffer, client_str);

	err = network_open(url_buffer, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
	handle_err("get:open:cmd");
	n = network_read(url_buffer, app_data, 240);
	network_close(url_buffer);

/*
    ENABLE_DARK_MODE(1, "enableDarkMode"),
    DISABLE_DARK_MODE(2, "disableDarkMode"),
    ENABLE_WHO(3, "enableWho"),
    DISABLE_WHO(4, "disableWho");
		ENABLE_BROADCAST(5, "enableBroadcast"),
    DISABLE_BROADCAST(6, "disableBroadcast"),

 */

	if (n > 0) {
		for (i = 0; i < n; i++) {
			cmd = app_data[i];
			switch (cmd) {
				case 1: // enable dark mode
					is_darkmode = true;
					do_darkmode();
					break;
				case 2: // disable dark mode
					is_darkmode = false;
					do_darkmode();
					break;
				case 3: // enable who
					is_showing_clients = true;
					break;
				case 4: // disable who
					is_showing_clients = false;
					break;
				case 5: // enable broadcast
					get_broadcast();
					is_showing_broadcast = true;
					break;
				case 6: // enable info
					is_showing_info = true;
					break;
				case 7: // disable info
					is_showing_info = false;
					break;
				default:
					is_showing_broadcast = false;
					break;
			}
		}
	}

}