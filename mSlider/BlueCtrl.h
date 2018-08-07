/*

@@@@@@    @@@                     @@@@     @              @@@   
 @@  @@    @@                    @@  @@   @@               @@   
 @@  @@    @@                   @@    @   @@               @@   
 @@  @@    @@   @@  @@   @@@@@  @@      @@@@@@  @@ @@@     @@   
 @@@@@     @@   @@  @@  @@   @@ @@        @@     @@  @@    @@   
 @@  @@    @@   @@  @@  @@@@@@@ @@        @@     @@  @@    @@   
 @@  @@    @@   @@  @@  @@      @@    @   @@     @@        @@   
 @@  @@    @@   @@  @@  @@   @@  @@  @@   @@ @@  @@        @@   
@@@@@@    @@@@   @@@ @@  @@@@@    @@@@     @@@  @@@@      @@@@  

*/

#ifndef BlueCtrl_h
#define BlueCtrl_h

#include "mSlider.h"
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include <Metronome.h>
#include <Applet.h>

#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused

#define STATUS_PERIOD	100

class BlueCtrl : public Applet
{
public:
	BlueCtrl() : ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST), Metro(STATUS_PERIOD), Buffer(""), BufferIndex(0) {}
	void		Setup();
	void		Run();
	bool		Command(String s);
	bool		Write(String s);

private:
	void		error(char* err);

	Metronome	Metro;
	Adafruit_BluefruitLE_SPI ble;
	bool		Connected;
	String		Buffer;
	uint8_t		BufferIndex;	// current buffer index
};

#endif
