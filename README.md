# Erdfeuchtemessung

### Hinweis - Wichtig!
Dieses Repository ist im Aufbau und stellt derzeit noch keine fertige Lösung bereit!
Für mögliche Fehler und daraus ggf. entstandene Schäden beim Anwender wird keinerlei Haftung übernommen.


### Warum?
Das Repo entstand, um die Ideen und Entwicklungen im immer länger werdenden Thread unter http://www.forum-raspberrypi.de/Thread-erdfeuchte-bewaesserung zu diesem Thema besser zu strukturieren.
Ausschlaggebend war die Idee des [[Giess-o-mat|https://www.mikrocontroller.net/articles/Giess-o-mat]], welchen ich anfangs verwendete und bei meinen Messungen diverse Effekte bzgl. der Langszeitstabilität feststellte (was dem Giess-o-mat nicht schlecht macht: Die Effekte begannen mit der Schutzbeschichtung des Sensors...). 

### Um was geht es?
Dieses Repository sammelt Ideen und mögliche Realisierungsbeispiele über die Art und Weise, wie man reproduzierbar und dauerhaft die Erdfeuchte bestimmen kann.
Der Fokus liegt dabei auf der Bestimmung des konkreten Bewässerungszeitpunktes und nicht auf eine prozentuale Ermittlung der Erdfeuchte.

Die ermittelte Erdfeuchte kann dann für weitergehende Aufgaben (z.B. Bewässerung) verwendet werden. 

### Wie soll das gehen? (Konzept)
* Ein oder mehrere Sensoren stecken direkt in der Erde und ermitteln die umgebende Feuchte der Erde.
* Die Erfassung und Vorverarbeitung der Feuchtedaten erfolgt mit einem ESP8266 ebenfalls vor Ort.
* Die Daten werden per lokalem WLAN per REST-Interface zu einem Datenerfassungsrechner gesendet, welcher die Daten in einer MySQL-DB speichert.
* Die Daten können nun aus der Datenbank beliebig ausgelesen und weiterverarbeitet werden.
* Die Steuerung der Messintervalle im ESP8266 erfolgt über die REST-Schnittstelle. 

### Wo kann ich mehr erfahren?
* Im Wiki zu diesem Projekt sind weitere Informationen abrufbar.
* http://www.forum-raspberrypi.de/Thread-erdfeuchte-bewaesserung
* https://www.mikrocontroller.net/articles/Giess-o-mat

