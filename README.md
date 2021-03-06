# E3DC-Control-Display-Prognose

Prinzipiell gelten alle Hinweise von Eberhard, s.u..

## Probleme mit Displayansteuerung
* Lösung, da sich fbtft_device für Grafikausgabe in Raspi Buster Kernel geändert hat: https://www.raspberrypi.org/forums/viewtopic.php?f=107&t=281204&p=1843929#p1843929

* Wenn keine Diplay vorhanden, oder Probleme mit Grafik das Programm mit Startparameter -nodisplay starten.


## Displayansteuerung
+ Installation der Treiber für Displayansteuerung
* sudo modprobe fbtft_device name=sainsmart18
*Create /etc/modprobe.d/fbtft.conf”:
   And add line
   options fbtft_device name=sainsmart18 gpios=reset:25,dc:24,cs:8 rotate=90 fps=30 speed=48000000

+ Installieren: git clone https://github.com/Micha70/E3DC-Control-Display.git
+ nach E3DC-Control-Display wechseln und mit make compilieren
```
cd E3DC-Control-Display
make
```
--> weitere Konfiguration siehe unten

--> wenn fatal error: bcm2835.h: No such file or directory
download the latest version of the library, say bcm2835-1.xx.tar.gz, then:
```
cd /home/pi/
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.60.tar.gz     --> you can find the last version on http://www.airspayce.com/mikem/bcm2835/
tar zxvf bcm2835-1.60.tar.gz
cd bcm2835-1.60
./configure
make
sudo make check
sudo make install
```


+ verwendetes Display: https://www.amazon.de/gp/product/B078J5TS2G/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1
+ Anschluss Display:
```
TFT     PIN     RASPI Pin
VCC     1       1 (3,3V)
GND     2       6 (GND)
SCL     7       23 (SCLK)
SDA     6       19 (MOSI)
RS/DC   5       18 (GPIO24)
RES     4       22 (GPIO25)
CS      3       24 (GPIO8)
LED     8       Option ohne Helligkeitsregelung: Pin 17 3v3 Stromversorgung
                Option mit Helligkeitsregelung (PWM): --> PIN12 (BCM18 PWM0)
```
Um das Display am Raspi betreiben zu können, muss die SPI aktiviert werden mit raspi-config! SPI ist standardmäßig deaktiviert.

Option ohne Helligkeitregelung: Display kann direkt 1:1 angeschlossen werden

Option mit Helligkeitsregelung(PWM): Treiberstufe erforderlich:

```
                                         5V
                                         |
                            2Ohm       __|
                           _____     ||  |
PIN12 (BCM18 PWM0)   -----|_____|--- ||->|
                                     ||__
                                         |
                                         |
                                         -
                                        | |
                                        | | 27Ohm
                                         -
                                         |
                                         |------> Display (Pin 8)

```

PWM funktioniert nur wenn E3DC-Control-Display mit Sudo Rechten gestartet wird. Ansonsten wird nur zwischen 100% und 0% umgeschaltet.
In io_BCM2835.h stehen die Schaltzeitpunkte und die Helligkeitsstufen:
```
    #define TIME_FOR_BRIGHTNESS "06:00"
    #define TIME_FOR_BRIGHTNESS "06:00"
    #define TIME_FOR_DARKNESS "22:00"
    #define BRIGHTNESS_LEVEL  80
    #define DARKNESS_LEVEL  10
```


Mein E3DC.sh mit sudo Rechten:

```#!/bin/bash
cd E3DC-Control-Display
while true; do
sudo ./E3DC-Control-Display
sleep  300
echo "Neustart durchgefÃ¼hrt"
done
```
---
## Prognose

**Aktivierung** erfolgt in e3dc.confic.txt:
```
prognose = true
```
Folgende Parameter müssen zusätzlich gesetzt werden:
```
longitude = 12.100000
latitude = 49.100000
dach_richtung = 22
dach_neigung = 40
wirkungsgrad = 0.8
anlagen_leistung = 5.7
grundbedarf = 300
```

*latitude - latitude of location, -90 (south) … 90 (north)
*longitude - longitude of location, -180 (west) … 180 (east)
*dach_neigung - plane declination, 0 (horizontal) … 90 (vertical)
*dach_richtung - plane azimuth, -180 … 180 (-180 = north, -90 = east, 0 = south, 90 = west, 180 = north)
*anlagen_leistung - installed modules power in kilo watt
*wirkungsgrad - Korrekturfaktor zwischen Prognose und wirklicher Leistung der Anlage
*grundbedarf - Grundbedarf des Hauses, was wird ohne größere Verbraucher benötigt (duchschn. Verbrauch der Nacht)


**Erklärung:**
Als Prognose wird die Prognose von http://doc.forecast.solar/doku.php?id=api:weather verwendet. Da nur 10 Prognoseabfragen pro Stunde möglich sind, wird dies automatisch vom Programm berücksichtigt.

**Prognoseermittlung und Einfluss:**
Bei der Prognoseermittlung wird die zu erwartende verbleibende maximale Leistung und der verbleibende Ertrag bezogen auf die Uhrzeit ermittelt. Diese Prognosewerte werden auch im Logfile abgelegt:
PROGNOSE Uhrzeit max.Leistung verbleibenderErtrage Kriterium

*Kriterium = 0:* kein Eingriff wegen niedriger Prognose

*Kriterium = 1:* prognostizierte verbleibende max Leistung (korrigiert mit Wirkungsgrad der Anlage) < (Einspeiselimit + Grundbedarf) und SOC < 70

--> fLadeende wird auf ladeende2 gesetzt

*Kriterium = 2:* prognostizierter Ertrag (korrigiert mit Wirkungsgrad der Anlage) < ( notwendige Energie um Akku auf 100% aufzuladen ) * 2 und SOC < 90

--> fLadeende wird auf 100% gesetzt

--> RegelEnde und LadeEnde werden um 2h vorgezogen

--> Unload = 100 Entladen der Batterie wird nicht durchgeführt

*Kriterium = 3:* prognostizierter Ertrag für den Nachmittag (nach 12:00) (korrigiert mit Wirkungsgrad der Anlage) < ( 1.5* Akku Kapazität )
--> fLadeende/Ladeschwelle wird auf 100% gesetzt

--> RegelEnde und LadeEnde werden um 4h vorgezogen

--> Unload = 100 Entladen der Batterie wird nicht durchgeführt

--> Kriterium 3 wird eingelogged für kompletten Tag, solange SOC<90%


---
# E3DC-Control

Viele haben bemängelt, das der Speicher von E3DC über keine ausreichende und funktionsfähige Steuerung zur prognosebasierende Laden verfügt.
Ich habe nun schon seit mehr als ein Jahr, meine Software auf einem headless Raspberry Pi Zero laufen.
nachdem ich nun mehrfach um meine Software gebeten wurde, habe beschlossen, diese über Github zu veröffentlichen.

Ich kann natürlich keinerlei Haftung für Funktion der Software, Wartung etc. übernehmen. Jeder ist natürlich eingeladen, die Software zu erweitern und zu verbessern und diese auf eigene Gefahr und Risiko einzusetzen.

Hier findet Ihr meine erste Version.

git clone https://github.com/Eba-M/E3DC-Control.git

Ich bin auch gerade erst dabei mich in Github einzuarbeiten, erwartet bitte keine perfekte Dokumentation und Anleitung.
Diese wird sicherlich im Lauf des Projektes noch verbessert und erweitern werden müssen.



viel Spass beim Ausprobieren

Eberhard

Diese Programm ist gedacht, auf einem Raspberry Pi ständig laufen zu lassen.
Bei mir läuft das Programm auf einen headless Raspberry Pi Zero W.
Das bedeutet, ich habe keinen Monitor oder Tastatur angeschlossen.
Die gesamte Steuerung/Überwachung und Bedienung erfolgt remote per ssh von meinen MAC aus.
Ich lasse das Programm in einer Session innerhalb von Screen laufen.

Als Basis dient das von E3DC veröffentliche RSCP-Beispielprogramm. Der Speicher wird morgens mit maximaler Ladeleistung auf ein mittleres SoC geladen. Dann wird die Batterie innerhalb eines Ladekorridor bis zum geplanten Ladeende auf 90% geladen. Das Ladeende wird für Winterminimum und Sommermaximum definiert und das Programm ermittelt für jeden Tag dazwischen das entsprechende Ende der Überwachungsladung. Daneben wird auch die Überschussleistung ermittelt und nötigenfalls die Ladeleistung hochgeregelt um unter Einspeisegrenze zu bleiben.

HT = Hochtarifoptimierung. Mit den Parametern z.B. "hton = 5" und "htoff = 21" wird der Beginn und die Endzeit des Hochtarif von den Wochentagen Mo.-Fr. in GMT festgelegt, hier im Beispiel von 6Uhr bis 22 Uhr. Über "htmin = 50" wird festgelegt, dass diese Regelung erst bei einem Soc des Speichers von < 50% wirksam wird.
So wird sichergestellt, das bei HT/NT Tarifen der Speicher möglichst im Hochtarif ausspeichert. Sonst wird der Speicher über Nacht bei NT entleert und morgens bei Beginn des HT ist dann keine Ladung mehr im Speicher verfügbar.
Die Speichergröße fortlaufend wird zwischen dem kürzesten Tag (ht=50) und Tag-/Nachtgleiche 0% mittels einer Cosinusfunktion verändert.  

Nun noch einige Hinweise um das Programm auf den Raspberry Pi zu instllieren und dort zu nutzen

Wenn man einen Raspberry Pi Zero W headless nutzen möchte, findet man hier hinweise zu ssh over usb

https://desertbot.io/blog/ssh-into-pi-zero-over-usb

Den Raspberry PI einrichten und Betrieb


# WLAN SCHON VOR DER INBETRIEBNAHME KONFIGURIEREN

Mitunter ist es praktisch, wenn Sie einen Raspberry Pi auf Anhieb über das WLAN via SSH bedienen können. Das gibt Ihnen die Möglichkeit, ohne angeschlossene Maus und Tastatur mit der Konfiguration zu beginnen — zumindest soweit, wie Sie dies via SSH im Textmodus durchführen können.
Die leere Datei ssh bewirkt, dass der SSH-Dienst sofort aktiviert wird. (Bei aktuellen Raspbian-Versionen ist dies ja nicht mehr der Fall.) 
Und die Datei wpa_supplicant.conf enthält die WLAN-Konfiguration. Sie wird beim ersten Start des Raspberry Pi in das Verzeichnis /etc/wpa_supplicant kopiert. Die Datei muss die Bezeichnung des WLANs (SSID) und dessen Passwort enthalten. Dabei gilt dieser Aufbau. 

Datei wpa_supplicant.conf in der Boot-Partition (Raspbian Stretch)

country=DE
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
network={
       ssid="wlan-bezeichnung"
       psk="passwort"
       key_mgmt=WPA-PSK
}

Damit kann der Raspberry auf das Hausnetz zugreifen

Sobald der Raspberry Pi hochgefahren ist, können Sie sich mit ssh pi@raspberrypi und dem Default-Passwort raspberry einloggen. Anschließend müssen Sie sofort mit sudo passwd pi ein neues Passwort für den Benutzer pi einrichten! Ein aktiver SSH-Server in Kombination mit dem Default-Passwort ist ein großes Sicherheitsrisiko!
// Installieren git

`sudo apt install git`

`git clone  https://github.com/Eba-M/E3DC-Control.git`

// updates runterladen (nur erforderlich, wenn ein neues Updat installiert werden soll

cd E3DC-Control  // Nach E3DC wechseln und mit pull runterladen

git pull  https://github.com/Eba-M/E3DC-Control.git`

// nach clone oder pull  mit make kompilieren

cd E3DC-Control

make

// mit nano die Datei

nano e3dc.config.txt.template

// editieren und die Parameter

server_ip = xxx.xxx.xxx.xxx
server_port = 5033
e3dc_user = xxxxxxxxxx
e3dc_password = xxxxxxxx

// und die möglichen Parameter

wallbox = false         // true, wenn man die E3DC-Wallbox nutzen möchte
ext1 = false				// true, wenn ein externer Zähler genutzt wird
ext2 = false
wurzelzaehler = 0		// 6 = externer Wurzelzähler
einspeiselimit = 7.0 // 70% Einspeisegrenze
untererLadekorridor = 500  
obererLadekorridor = 1500  // bei der PRO wird 4500 empfohlen
minimumLadeleistung = 300
maximumLadeleistung = 3000  // 1500 bei mini, 3000 E12 und 9000/1200 PRO
wrleistung = 12000          // AC-Leistung des WR, 4600 bei mini
ladeschwelle = 15           // Unter 15% SoC wird immer geladen
ladeende = 85               // Ziel SoC 85% zwischen
winterminimum = 11		   // winterminimum wintersonnenwende
sommermaximum = 14           // sommermaximum sommersonnenwende
sommerladeende = 18.5     // im Sommer wird das Laden auf 100% verzögert
// Im Winterhalbjahr wird versucht den Speicher zum Hochtarif zu nutzen
htmin = 30                // Speicherreserve 30% bei winterminimum
htsockel = 10             // sockelwert bei Tag-Nachtgleiche
hton = 5                  // Begin Hochtarif
htoff = 14                // Ende Hochtarif
htsat = true              // Hochtarif Samstag
htsun = true              // Hochtourig Sonntag
debug = false             // zusätzliche debug ausgaben
```

// anpassen und als
//`e3dc.config.txt`
// abspeichern.


// Installieren von Screen

`sudo apt-get install screen`


// Skriptdatei erstellen

nano E3DC.sh

// folgende kopieren

#!/bin/bash
while true;
 do
./E3DC-Control
sleep 30
done
```
// Skriptdatei ausführbar machen

`chmod +x E3DC.sh`

// Ausführen

`./E3DC.sh`

// Autostartdatei erstellen

`sudo nano /etc/init.d/e3dcstart`

``` #!/bin/bash
 ### BEGIN INIT INFO
 # Provides:          scriptname
 # Required-Start:    $remote_fs $syslog
 # Required-Stop:     $remote_fs $syslog
 # Default-Start:     2 3 4 5
 # Default-Stop:      0 1 6
 # Short-Description: Start daemon at boot time
 # Description:       Enable service provided by daemon.
 ### END INIT INFO
echo "E3DC-Control  wird gestartet"
su  pi -c "screen -dmS E3DC /home/pi/E3DC-Control/E3DC.sh"

exit 0
```



// Skriptdatei ausführbar machen


`sudo chmod 755 /etc/init.d/e3dcstart`

// Skriptdatei in die autostart einfügen

`sudo update-rc.d e3dcstart defaults`


Richtig herunterfahren

`sudo shutdown -h 0`
