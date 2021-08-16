#include <Arduino.h>
#include <SPI.h>
#include "VentiData.h"
#include "Beat2020WifiCom.h"

using namespace beat2020;

WifiCom<5> wifi;

void setup() {

}

void loop() {
	unsigned long time = millis();
	Data data(11, time + 1, time + 2, time + 3, time + 4, time + 5, time);
	wifi.addData(data);
	wifi.update();
}