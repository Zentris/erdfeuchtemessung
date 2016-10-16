<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head><meta http-equiv="content-type" content="text/html; charset=ISO-8859-1" /><meta http-equiv="expires" content="0"></head>
<body>
<?php
$dbhost = "localhost";
$dbuser = "datalogger";
$dbpw   = "datalogger";
$dbname = "ESPLogger";
$tableName = "ESP_Feuchte";

#- parameter to table row map
#--->             parameter        table-row-name
$parm2rowMap = array (
                 "espno"       => "espno",
                 "date"        => "date",
                 "time"        => "time",
                 "feuchte"     => "feuchte",
                 "temperatur"  => "temperatur",
                 "luftfeuchte" => "luftfeuchte",
                 "nan"         => "nan",
                 "mac"         => "espmac",
                 "gpio"        => "gpio",
                 "sensorid"    => "sensorid"
               );

# -------------------------------------------
# --- process REST data   -------------------
# -------------------------------------------

## -- command line
# wget -q -O /dev/nul  'http://192.168.178.23/upd_feuchte.php?espno=1&date=2015-08-13_14-22-01&feuchte=123456789'
#
#

$rowMapData = array();
$debug = 0;

#-testausgabe (nur, wenn ein &debug=x mitkommt!)
if ( array_key_exists("debug", $rowMapData) && $rowMapData["debug"] != "" ) {
	$debug = 1;
	foreach ($rowMapData as $k => $v) echo "::: $k = $v <br />";
}

foreach ($_REQUEST as $k => $p) {
  if ( $debug == 1) echo "$k = $p <br />";
  if ( array_key_exists($k, $parm2rowMap)) {
    $rowMapData[$parm2rowMap[$k]] = (isset($_REQUEST[$k])) ? htmlspecialchars($_REQUEST[$k]) : "";
  }
}


## - parameter check, return html return value 404 and exit with -1
if ( array_key_exists("espmac", $rowMapData) && $rowMapData["espmac"] != "" &&
     array_key_exists("sensorid", $rowMapData) && $rowMapData["sensorid"] != "" &&
     array_key_exists("date", $rowMapData) && $rowMapData["date"] != "" ) {
  if ( $debug == 1) { echo "ok <br />"; }
}
else {
  http_response_code(404);
  exit (-2);
}

$values="values(";
$bInsertSep = false;

$sql="insert into $tableName (";
foreach ($rowMapData as $row => $data) {
  if ($bInsertSep) {
    $sql    .= ",";
    $values .= ",";
  }
  else $bInsertSep = true;
    $sql .= $row;
    $values .= "'" . $data . "'";
}
$sql .= ") " . $values . ")";

echo "sql='$sql'<br />";

echo "\n*[ACTION]* : DB will be conected" . "<br />";
$mysqli = new mysqli($dbhost, $dbuser, $dbpw, $dbname);
if ($mysqli->connect_errno) {
    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}

//echo "*[ACTION]* : DB is connected" . "<br />";
//echo "*[ACTION]* : prepare statement" . "<br />";
$stmt = $mysqli->prepare($sql);

//echo "*[ACTION]* : execute statement" . "<br />";
$stmt->execute();

//echo "*[ACTION]* : DB will be closed" . "<br />";
$mysqli->close();
//echo "*[ACTION]* : DB is closed" . "<br />";

echo "@@@messabstand=120000;";

?>
</body>
</html>
