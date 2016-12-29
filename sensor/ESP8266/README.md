=====Readme for using the sketch and others

[german]

1. Kopiere das File 'privats_dummy.h' nach 'privats.h'
2. Passe das File 'privats.h' an, (Zugriffsdaten befüllen).
3. Speichern und compilieren

## deep sleep ##
Die deep-sleep Funktionalität kann nur angewendet werden, wenn am ESP8266
eine Brücke (Drahtverbindung) zwischen dem Pin 16 (GPIO16) und dem Reset-Pin
(RST) angebracht ist.

Ohne größere Problem kann das beim ESP8266-01 nicht gemacht werden (Der
GPIO16 wird nicht herausgeführt und muss mit einem feinen Draht direkt am
Chip kontaktiert werden)


[english]

1. Copy the file 'privats_dummy.h' to 'privats.h' in the same directory.
2. Change the content of the File 'privates.h' (fill access data).
3. Save and compile

## deep sleep ##
The deep sleep functionality can used only with the ESP8266 if a wire
between the GPIO16 and the reset pin (RST).

On the ESP8266-01 we have no direktly access to the GPIO16, in this case
the pin must be directly soldered on the pin on the chip.
