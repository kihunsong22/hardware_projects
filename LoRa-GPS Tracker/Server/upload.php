<?php
include("./head.php");
include_once('dist/Medoo.php');

ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);

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

if(isset($_GET['passcode'])){
    $passcode = $_GET['passcode'];  //passcode 설정해야함
}
else{
    exit();
}

if(isset($_POST['gps_long'])){
    $gps_long = $_POST['gps_long'];
}
else{
    $gps_long = -1;
}

if(isset($_POST['gps_lang'])){
    $gps_lang = $_POST['gps_lang'];
}
else{
    $gps_lang = -1;
}

if(isset($_POST['rssi'])){
    $rssi = $_POST['rssi'];
}
else{
    $rssi = 1234;
}

$SQL = "INSERT INTO data (gps_lang, gps_long, rssi) VALUES('$gps_lang', '$gps_long', '$rssi');";
mysqli_query($conn, $SQL);

// exit();
?>
