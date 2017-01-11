/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file px4_lead_me.cpp
 *
 *
 * @author Falk Lueke <falk.lueke@hs-owl.de>
 */

#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_global_position.h>		// uORB-Thema, beinhaltet die Globale position in WGS84 Koordinaten

/** Funktions deklaration
 *  --------------------- */
// Hauptfunktion
__EXPORT int px4_lead_me_main(int argc, char *argv[]);

/** Modulvariablen
 *  --------------
 *  Variablen die im Gesamten Modul sichtbar sind.
 *  Hier definieren damit die Uebersicht gesteigert wird. */
/** Variablen */
int error_counter = 0;									// Zaehler fuer Fehlerhafte Poll-Anfragen (Gesamtzahl)
int vehicle_global_position_interval = 200;				// Abfragegeschwindigkeit der Globalen Position (uorb_set_interval), in ms
int poll_answ_time = 1000;								// Periodendauer in der auf eine Aktualisierung der uORB-Themen gewartet wird, in ms
/** Funktions definition
 *  -------------------- */
/* Hauptfunktion
 *  */
int px4_lead_me_main(int argc, char *argv[]){
// Ausgabe des Textes ueber die Log-Funktion in der PX4 Firmware
PX4_INFO("Lead me Application Startet");

/** uORB
 *   ----
 * Informationen zu den uORB-Funktionen sind in
 * uORBManager.hpp
 * zu finden. */
/* Themen abonieren (subscribe)
  * Angabe des Themas ueber UORB_ID() Makro */
int uorb_sub_global_position = orb_subscribe(ORB_ID(vehicle_global_position));

/* Abfragegeschwindigkeit einstellen
  * Angabe der Geschwindigkeit in milli Sekunden
  * MUSS fuer jedes Thema sepeart eingestellt werden */
//int poll_ret = orb_set_interval(uorb_sub_global_position, vehicle_global_position_interval);

/* Sammlung der abonierten Themen einstellen
 *  */
 px4_pollfd_struct_t fds[] = {							// deklariert in px4_posix.h
   		{.fd = uorb_sub_global_position, .events = POLLIN},			// Initialisierung unvollstaendig
 };



/** Abfrage der uORB-Nachrichten
 *  */
for(int i = 0; i < 5; i++){
	/* Warten das eine neue uORB-Nachricht eintrifft
	 * Alle uORB-Themen in einer Sammlung werden beruecksichtigt */
	int poll_ret = px4_poll(fds, 1, poll_answ_time);				// 1000 = 1000 ms = 1 s

	// Keine Aktualisierung eines uOEB-Themas
	if(poll_ret == 0){												// Kein uORB-Thema hat in der angegebenen Zeit geantwortet
		PX4_ERR("Got no data whithin %d ms",poll_answ_time);		// Fehlermeldung in das Log
	}
	// Schwerer Fehler in der Abfrage - das ist ein Notfall
	else if (poll_ret < 0){
		if(error_counter < 10 || error_counter % 50 == 0){
			PX4_ERR("ERROR return value from px4_poll(): %d", poll_ret);
		}
		error_counter ++;											// Fehlerzaehler um eins erhoehen

	}
	// Aktualisierung eines uORB-Themas
	else{
		// Abfrage welches Thema aktualisiert wurde
		if(fds[0].revents & POLLIN){								// erstes Thema (vehicle_gloabal_position)
			// Variablen fuer Auswertung
			struct vehicle_global_position_s raw;						// Zwischenspeicher der uORB Rohdaten
			// uORB Rohdaten lesen
			orb_copy(ORB_ID(vehicle_global_position), uorb_sub_global_position, &raw);

			// Daten in die Konsole schreiben
			PX4_INFO("Vehicle Global Position:\nLatitude (Geo. Breite):\t%8.4\nLongitude (Geo. Laenge):\t%8.4\nAltitude (Hoehe):\t%8.4",
																	    (double)raw.lat,
																										(double)raw.lon,
																																(double)raw.alt);
		}
	}
}

/** ENDE
 *  ---- */
return OK;												 // "OK" ist als Praeprozessor Makro definiert
}

