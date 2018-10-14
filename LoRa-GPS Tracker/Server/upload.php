<?php
// include("./head.php");
include_once('dist/Medoo.php');

// ini_set("display_errors", 1);
// ini_set("display_startup_errors", 1);
// error_reporting(E_ALL);

function get_client_ip() {
    $ipaddress = '';
    if (getenv('HTTP_CLIENT_IP'))
    $ipaddress = getenv('HTTP_CLIENT_IP');
    else if(getenv('HTTP_X_FORWARDED_FOR'))
    $ipaddress = getenv('HTTP_X_FORWARDED_FOR');
    else if(getenv('HTTP_X_FORWARDED'))
    $ipaddress = getenv('HTTP_X_FORWARDED');
    else if(getenv('HTTP_FORWARDED_FOR'))
    $ipaddress = getenv('HTTP_FORWARDED_FOR');
    else if(getenv('HTTP_FORWARDED'))
    $ipaddress = getenv('HTTP_FORWARDED');
    else if(getenv('REMOTE_ADDR'))
    $ipaddress = getenv('REMOTE_ADDR');
    else
    $ipaddress = 'UNKNOWN';
    return $ipaddress;
}

$publicip = get_client_ip();

if(isset($_POST['passcode'])){
    $passcode = $POST['passcode'];  //passcode 설정해야함
}else if(isset($_GET['passcode'])){
    $passcode = $_GET['passcode'];
}else{
    $passcode = 0;
}

if(isset($_POST['gps_long'])){
    $gps_long = $_POST['gps_long'];
}
else if(isset($_GET['gps_long'])){
    $gps_long = $_GET['gps_long'];
}else{
    $gps_long = -1;
}

if(isset($_POST['gps_lang'])){
    $gps_lang = $_POST['gps_lang'];
}else if(isset($_GET['gps_lang'])){
    $gps_lang = $_GET['gps_lang'];
}else{
    $gps_lang = -1;
}

if(isset($_POST['rssi'])){
    $rssi = $_POST['rssi'];
}else if(isset($_GET['rssi'])){
    $rssi = $_GET['rssi'];
}else{
    $rssi = 404;
}

if($passcode != 4660){
    die("INVALID ACCESS");
}

$SQL = "INSERT INTO data (gps_lang, gps_long, rssi) VALUES('$gps_lang', '$gps_long', '$rssi');";
mysqli_query($conn, $SQL);

?>
