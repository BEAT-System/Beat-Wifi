#ifndef Beat2020WifiCom_h
#define Beat2020WifiCom_h
#include <SPI.h>
#include <WiFiNINA.h>
#include "VentiData.h"
#include "secrets.h"

namespace beat2020{
	template < int BUFFER_SIZE >
	class WifiCom {
	private:
		Data _buff[BUFFER_SIZE];
		uint8_t _buff_counter = 0;
		char* ssid = SECRET_SSID;
		char* pass = SECRET_PASS;
		int status = WL_IDLE_STATUS;

		unsigned long lastConnectionTime = 0;	// last time you connected to the server, in milliseconds
		const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

		WiFiClient client;
		IPAddress server;

		String get_buff_as_json();

	public:
		WifiCom();
		bool connect();
		void send();
		void update();
		void addData(const Data& data);
	};

	template < int BUFFER_SIZE >
	WifiCom<BUFFER_SIZE>::WifiCom():
		server(SERVER_IP)
	{

	}

	template < int BUFFER_SIZE >
	String WifiCom<BUFFER_SIZE>::get_buff_as_json() {
		String msg = u8R"({"data": [)";
		for (size_t i = 0; i < _buff_counter; i++)
		{
			char json[100 * _buff_counter];
			sprintf(json,
				u8R"({"time": %lu, "bpm": %d, "p_set": %d, "p_c": %d, "flow_c": %d, "vol_c": %d})",
				_buff[i].getTime(),
				_buff[i].bpm,
				_buff[i].p_set,
				_buff[i].p_c,
				_buff[i].flow_c,
				_buff[i].vol_c);
			if (i < BUFFER_SIZE - 1)
			{
				msg = msg + json + ", ";
			}
			else {
				msg = msg + json;
			}

		}
		msg = msg + u8R"(]})";
		return msg;
	}

	template < int BUFFER_SIZE >
	bool WifiCom<BUFFER_SIZE>::connect() {
		//Initialize serial and wait for port to open:
		//Serial.begin(9600);
		//Serial.println("connect");
		/*while (!Serial) {
			; // wait for serial port to connect. Needed for native USB port only
		}*/
		// check for the WiFi module:
		if (WiFi.status() == WL_NO_MODULE) {
			//Serial.println("Communication with WiFi module failed!");
			// don't continue
			while (true);
		}

		String fv = WiFi.firmwareVersion();
		if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
			//Serial.println("Please upgrade the firmware");
		}

		// attempt to connect to Wifi network:
		while (status != WL_CONNECTED) {
			//Serial.print("Attempting to connect to SSID: ");
			//Serial.println(ssid);
			// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
			status = WiFi.begin(ssid, pass);

			// wait 1 seconds for connection:
			delay(500);
		}
		// you're connected now, so print out the status:
		return true;
	}

	template < int BUFFER_SIZE >
	void WifiCom<BUFFER_SIZE>::send() {
		//Serial.println("send");
		connect();
		String body = get_buff_as_json();
		// close any connection before send a new request.
		// This will free the socket on the Nina module
		client.stop();

		// if there's a successful connection:
		if (client.connect(server, SERVER_PORT)) {
			Serial.println("connecting...");
			// send the HTTP PUT request:
			client.println("POST /device/" + String(_buff[0].id) + " HTTP/1.1");
			client.println("Host: example.org");
			client.println("Content-Type: application/json");
			client.print("Content-Length: "); client.println(body.length());
			client.println("User-Agent: ArduinoWiFi/1.1");
			client.println("Connection: close");
			client.println();
			client.println(body);
			//Serial.println(body);
			//client.println();
			// note the time that the connection was made:
			lastConnectionTime = millis();
		}
		else {
			// if you couldn't make a connection:
			//Serial.println("connection failed");
		}
		_buff_counter = 0;
	}

	template < int BUFFER_SIZE >
	void WifiCom<BUFFER_SIZE>::update() {
		if (BUFFER_SIZE <= _buff_counter || millis() - lastConnectionTime > postingInterval)
		{
			send();
		}
	}

	template < int BUFFER_SIZE >
	void WifiCom<BUFFER_SIZE>::addData(const Data& data) {
		if (BUFFER_SIZE <= _buff_counter)
		{
			send();
		}
		_buff[_buff_counter] = data;
		++_buff_counter;
	}
}

#endif // !Beat2020WifiCom_h
