# Wichtiger Hinweis 
Das Projekt liegt seit ca. 2017 "auf Eis", d.h. es wird von mir nicht weiter entwickelt.
Grund ist, dass ich inzwischen zu einem anderen Sensortyp gewechselt bin.
Den Code lass ich jedoch hier stehen, als Anschaung. Ich bitte jedoch das Alter bzgl. der Aktualität zu beachten

---


## Erdfeuchtemessung

#### Hinweis - Wichtig!
Dieses Repository ist in Entwicklung und stellt derzeit noch keine fertige Lösung bereit!
Für mögliche Fehler und daraus ggf. entstandene Schäden beim Anwender wird keinerlei Haftung übernommen.

Für tiefergreifende Infos ==> **WIKI dieses Repositorys** ansehen.

#### Warum?
Das Repo entstand, um die Ideen und Entwicklungen im immer länger werdenden Thread unter http://www.forum-raspberrypi.de/Thread-erdfeuchte-bewaesserung zu diesem Thema besser zu strukturieren.

Ausschlaggebend war die Idee des Giess-o-mat (https://www.mikrocontroller.net/articles/Giess-o-mat), welchen ich anfangs verwendete und bei meinen Messungen diverse Effekte bzgl. der Langzeitstabilität feststellte (was den Giess-o-mat nicht schlecht macht: Die Effekte begannen bei mir mit der Schutzbeschichtung des Sensors...). 

#### Um was geht es?
Dieses Repository sammelt Ideen und mögliche Realisierungsbeispiele über die Art und Weise, wie man reproduzierbar und dauerhaft die Erdfeuchte bestimmen kann.
Der Fokus liegt dabei auf der Bestimmung des konkreten Bewässerungszeitpunktes und nicht auf eine prozentuale Ermittlung der Erdfeuchte.

Die ermittelte Erdfeuchte kann dann für weitergehende Aufgaben (z.B. Bewässerung) verwendet werden. 

#### Wie soll das gehen? (Konzept)
* Ein oder mehrere Sensoren stecken direkt in der Erde und ermitteln die umgebende Feuchte der Erde.
* Die Erfassung und Vorverarbeitung der Feuchtedaten erfolgt mit einem ESP8266 ebenfalls vor Ort.
* Die Daten werden per lokalem WLAN per REST-Interface zu einem Datenerfassungsrechner gesendet, welcher die Daten in einer MySQL-DB speichert.
* Die Daten können nun aus der Datenbank beliebig ausgelesen und weiterverarbeitet werden (z.B. Giessautomatik).
* Die Steuerung der Messintervalle im ESP8266 erfolgt über die REST-Schnittstelle. 
* Das Konzept mit einem ESP8266 erlaubt weitere Sensoren in Pflanzennähe (z.B. Luft/Bodentemperatur, Sonnenscheindauer und -intensität usw.).

#### Wo kann ich mehr erfahren?
* Im Wiki (https://github.com/Zentris/erdfeuchtemessung/wiki) zu diesem Projekt sind weitere Informationen abrufbar.
* http://www.forum-raspberrypi.de/Thread-erdfeuchte-bewaesserung
* https://www.mikrocontroller.net/articles/Giess-o-mat
* https://www.mikrocontroller.net/topic/169824#1623456
* https://wwwvs.cs.hs-rm.de/vs-wiki/index.php/Internet_der_Dinge_WS2015/SmartPlant#Messmethode_2:_Kapazitiv
 
#### Online Shop(s):
* http://www.ramser-elektro.at/shop/bausaetze-und-platinen/giess-o-mat-sensor-bausatz/
