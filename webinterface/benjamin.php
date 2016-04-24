<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
   <title>ESP-1 Erdfeuchtefühler</title>
   <meta charset="utf-8">

<?php
   // --------------
   // ESP 1/2 -  Erdfeuchte / DHT Stick
   // --------------
   $dbhost              = "192.168.178.14";
   $dbuser              = "datalogger";
   $dbpw                = "datalogger";
   $dbname              = "ESPLogger";
   $tableName           = "ESP_Feuchte";

   $ESP_MAC  = "0";		// wird aus der db geholt und später gesetzt
   $ESP_ID   = "1";		// default die 1			
   $ESP_GPIO = "0";		// default GPIO 0

   define("LastDSAnzahl_default",   "60");
   define("dia_hoehe_default",      "600");
   define("dia_breite_default",     "800");
   define("autorefreshOnDefault",   "1");
   define("autoRefreshTimeDefault", "30");

   $autorefrsh = autorefreshOnDefault;
   
   if (isset($_GET["checkboxes"])) {
	  if ($_GET["checkboxes"] == "autorefreshOn") {
		 $autorefrsh = 1;
		 echo "<meta http-equiv='refresh' content='";
			if ( isset($_GET['autoRefreshTime'])) {
			   echo $_GET['autoRefreshTime']; }
			else { echo autoRefreshTimeDefault;}
		 echo "' >";
	  }
   }
   else {
      $autorefrsh = "0";
   }

   if (isset($_GET["LastDSAnzahl"])) {
	   $anzahlDatenpunkte = $_GET["LastDSAnzahl"];
   }
   else $anzahlDatenpunkte = LastDSAnzahl_default;
   
   $ESP_GPIO="0"; // default
   if (isset($_GET["gpio"])) {
		if (($_GET["gpio"] == "0") or ($_GET["gpio"] == "2")) { 
			$ESP_GPIO = $_GET["gpio"];
		}
	}

   // Verbindung aufbauen, auswählen einer Datenbank
   $link = mysql_connect($dbhost, $dbuser, $dbpw) or die("Keine Verbindung möglich: " . mysql_error());
   mysql_select_db($dbname) or die("Auswahl der Datenbank $dbname fehlgeschlagen");

   // Mac aus den Messdaten holen
   $query = "select id, espmac from $tableName where $tableName.espno = $ESP_ID order by id desc limit 1";
   $result = mysql_query($query) or die("1. Anfrage fehlgeschlagen: " . mysql_error());   
   $line = mysql_fetch_array($result);
   $ESP_MAC = $line[1];	
   
   // Datenabfrage 1. Set
   $query = "select SQL_CACHE id, date, time, feuchte from 
		(select * from $tableName where $tableName.espno = $ESP_ID and $tableName.gpio = '0' order by id desc limit " . $anzahlDatenpunkte . ")
		 as sub order by date, time asc";
   $result0 = mysql_query($query) or die("1. Anfrage fehlgeschlagen: " . mysql_error());

   // Datenabfrage 2. Set
   $query = "select SQL_CACHE id, date, time, feuchte from 
		(select * from $tableName where $tableName.espno = $ESP_ID and $tableName.gpio = '2' order by id desc limit " . $anzahlDatenpunkte . ")
		 as sub order by date, time asc";
   $result2 = mysql_query($query) or die("1. Anfrage fehlgeschlagen: " . mysql_error());

   // Ausgabe der Ergebnisse
   echo "<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>\n";
   echo "<script type='text/javascript'>\n";
   echo "   google.charts.load('44', {'packages':['corechart']});\n";
   echo "   google.charts.setOnLoadCallback(drawChart);\n";
   echo "   function drawChart() {\n";
   echo "      var data0 = new google.visualization.arrayToDataTable([\n";
   echo "         ['Zeit', 'ErdFeuchte']\n";
   $c0 = 0;
   while ($line0 = mysql_fetch_array($result0, MYSQL_ASSOC)) {
	  printf("        ,['%s %s', %s]\n", $line0["date"], $line0["time"], $line0["feuchte"]);
	  $c0 += 1;
   }
   echo "         ]);\n";
   echo "      var data2 = new google.visualization.arrayToDataTable([\n";
   echo "         ['Zeit', 'ErdFeuchte']\n";
   $c2 = 0;
   while ($line2 = mysql_fetch_array($result2, MYSQL_ASSOC)) {
	  printf("        ,['%s %s', %s]\n", $line2["date"], $line2["time"], $line2["feuchte"]);
	  $c2 += 1;
   }
   echo "         ]);\n";
   echo "      var options0 = {\n";
   echo "         title: 'Sensor Benjamin (GPIO 2 - " . $c0 . " Datenpunkte)',\n";
//   echo "         curveType: 'function',\n";
   echo "         legend: { position: 'top' }\n";
   echo "      };\n";

   echo "      var options2 = {\n";
   echo "         title: 'Sensor Wasserglas (GPIO 0 - " . $c2 . " Datenpunkte)',\n";
//   echo "         curveType: 'function',\n";
   echo "         legend: { position: 'none'}\n";
   echo "      };\n";

   echo "      var chart = new google.visualization.LineChart(document.getElementById('curve_chart0'));\n";
   echo "      chart.draw(data0, options0);\n";
   echo "      var chart2 = new google.visualization.LineChart(document.getElementById('curve_chart2'));\n";
   echo "      chart2.draw(data2, options2);\n";
   echo "   }\n";
   echo "</script>\n";

   mysql_free_result($result);   // Freigeben des Resultsets
   mysql_close($link);           // Schließen der Verbinung
?>

</head>
<body>
   <h3>ESP-1 Erdfeuchtefühler (id=<?php echo $ESP_ID ?> , MAC=<?php echo $ESP_MAC ?>, Anzahl Messpunkte = <?php echo $anzahlDatenpunkte ?>) </h3>
   <div id="curve_chart0" style="width: 110%; height: 400px; position: relative; top: 0px; left: 0px;"></div>
   <div id="curve_chart2" style="width: 110%; height: 400px; position: relative; top: 0px; left: 0px;"></div>

<form action="<?php echo $_SERVER["PHP_SELF"];?>" method="get">
<table border="0">
  <tr>
	<th style="text-align:right">Autorefresh an:</th>
	<th><input type="text" name="autoRefreshTime" size="5" maxlength="5"
		value="<?php if(isset($_GET['autoRefreshTime'])){echo $_GET['autoRefreshTime'];}else{echo autoRefreshTimeDefault;}?>"/> (sec)</th>
	<th><input type="checkbox" name="checkboxes" size="10" maxlength="10" value="autorefreshOn" <?php if ($autorefrsh) echo "checked"; ?>/> </th>
  </tr>
<!--  
  <tr>
	<th style="text-align:right">GPIO-Selektion:</th>
	<th><select name="gpio" size="1">
			<option value="0" <?php if ($ESP_GPIO=="0") echo "selected" ?> >GPIO-0 (steckt im Benny)</option>
			<option value="2" <?php if ($ESP_GPIO=="2") echo "selected" ?> >GPIO-2 (Silikonvergossen)</option>
		</select>
	</th>
  </tr>
-->
  <tr>
	<th style="text-align:right">Anzahl der letzten Datensätze:</th>
	<th><select name="LastDSAnzahl" size="1" width=200>
			<option value="15"  <?php if ($anzahlDatenpunkte=="15")     echo "selected" ?> >15 Werte (1/2h)</option>
			<option value="30"  <?php if ($anzahlDatenpunkte=="30")     echo "selected" ?> >30 Werte (1h)</option>
			<option value="60"  <?php if ($anzahlDatenpunkte=="60")     echo "selected" ?> >60 Werte (2h)</option>
			<option value="90"  <?php if ($anzahlDatenpunkte=="90")     echo "selected" ?> >90 Werte (3h)</option>
			<option value="180" <?php if ($anzahlDatenpunkte=="180")    echo "selected" ?> >180 Werte (6h)</option>
			<option value="360" <?php if ($anzahlDatenpunkte=="360")    echo "selected" ?> >360 Werte (12h)</option>
			<option value="720" <?php if ($anzahlDatenpunkte=="720")    echo "selected" ?> >720 Werte (24h)</option>
                        <option value="1440"<?php if ($anzahlDatenpunkte=="1440")   echo "selected" ?> >1440 Werte (48h)</option>
			<option value="2160"<?php if ($anzahlDatenpunkte=="2160")   echo "selected" ?> >2160 Werte (3 Tage)</option>
			<option value="3600"<?php if ($anzahlDatenpunkte=="3600")   echo "selected" ?> >3600 Werte (5 Tage)</option>
			<option value="5040"<?php if ($anzahlDatenpunkte=="5040")   echo "selected" ?> >5040 Werte (7 Tage)</option>
			<option value="10080"<?php if ($anzahlDatenpunkte=="10080") echo "selected" ?> >10080 Werte (14 Tage)</option>
		</select>
  </th>
</table>

<input type="submit" value="Aktualisieren" />
</form>


</body>
</html>


