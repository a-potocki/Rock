#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


SoftwareSerial BLTSerial(2, 3);   //arduino's rx, tx
Adafruit_BMP280 bme;
unsigned long time1, time2; //time1 - wymuszenie regularnych pomiarow
int cycle1 = 50; //odstep miedzy pomiarami wysokosci
int cycle2 = 500; //odstep miedzy wysylaniem przez blt
bool st, activated, depl; //do testow
float slPressure = 996.00; //do ustawiania
float alt_raw, vel, alt_fil, k, alt_max; //vel - predkosc, alt_fil wysokosc filtrowana, 
float a[10];
float c = 5; //czulosc filtra
float alt_min = 1000;
float vel_activation = 0.5; //predkosc do wlaczenia systemu spadochronu
float vel_deploy = -0.5; //predkosc do otwarcia spadochronu

void setup()
{
	pinMode(13, OUTPUT);
	BLTSerial.begin(115200);
	BLTSerial.println("Bluetooth On");
	if (!bme.begin()) {
		BLTSerial.println("Could not find a valid BMP280 sensor");
		while (1);
	}
	alt_fil = bme.readAltitude(slPressure);
  for (int i = 9; i >= 0; i--)
	{
		a[i] = alt_fil;
	}
	time1 = millis();
	time2 = time1 + 50;
	time1 = time1 + 50;
}

void loop()
{
	if (millis() >= time1)
	{
		GetAlt();
		time1 = time1 + cycle1;
	}
	if (millis() >= time2)
	{
		Dump();
		time2 = time2 + cycle2;

	}
	if (BLTSerial.available() > 0) BLTReceived();
}

void GetAlt() 
{
	alt_raw = bme.readAltitude(slPressure);
	//if (alt_raw > 50)
	alt_fil = (alt_fil*(c - 1) + alt_raw) / c;
	
	for (int i = 9; i > 0; i--)
	{
		a[i] = a[i - 1];
	}
	a[0] = alt_fil;

	vel = (a[0] - a[9]) * 2;

	if (alt_fil > alt_max) alt_max = alt_fil;
	if (alt_fil < alt_min) alt_min = alt_fil;
	if (vel > vel_activation) activated = 1;
	if (activated == 1 && vel < vel_deploy) depl = 1;
	if (depl == 1) Deploy();
}

void Dump() 
{
	String dataString = "";
	dataString += String(alt_raw);
	dataString += ",";
	dataString += String(alt_fil);
	dataString += ",";
	dataString += String(vel);
	dataString += ",";
	dataString += String(activated);
	dataString += ",";
	dataString += String(depl);
	dataString += ",";
	dataString += String(alt_max);
	dataString += ",";
	dataString += String(alt_min);
	dataString += ",";

  //dataString += String(time1/50);
  //dataString += ",";

  BLTSerial.println(dataString);
}

void Deploy()
{
	digitalWrite(13, HIGH);
}

void BLTReceived() //Wiem, ze istnieje komenda switch, ale tu nie dziala ze stringami
{
	String inc = BLTSerial.readString();
	if (inc == "readAlt")
	{
		Dump();
	}
	else if (inc == "max")
	{
		String dataString = "";
		dataString += String(alt_max);
		dataString += ",";
		dataString += String(alt_min);
		dataString += ",";

		BLTSerial.println(dataString);
	}
	else if (inc == "reset")
	{
		alt_max = 0;
		alt_min = 1000;
		depl = 0;
		BLTSerial.println("ok");
	}
	else if (inc == "state")
	{
		if (st == 1)
		{
			BLTSerial.println("on");
		}
		else
		{
			BLTSerial.println("off");
		}
	}
	else
	{
		BLTSerial.println("Unknown command");
	}
}
