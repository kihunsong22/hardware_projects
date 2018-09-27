<?php
$NOLOGIN = 4660;
include("./head.php");
include_once('dist/Medoo.php');
include_once('SocketPing.php');

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
$curtime = (new DateTime())->format("Y-m-d H:i:s");


if(isset($_POST['serverno'])){
    $serverno = $_POST['serverno'];
}
else if(isset($_GET['serverno'])){
    $serverno = $_GET['serverno'];
}
if(!isset($serverno) || ($serverno < 0 || $serverno > 3)){
    $serverno = 404;
}

if(isset($_POST['localip'])){
    $localip = $_POST['localip'];
}
else{
    $localip = "0.0.0.0";
}

if(isset($_POST['thermal'])){
    $thermal = $_POST['thermal'];
}
else{
    $thermal = 0;
}

if(isset($_POST['freq'])){
    $freq = $_POST['freq'];
}
else{
    $freq = 0;
}

// $server = serverinfo($serverno);    //$server[0] == $serverip, $server[1] == $serverport

$SQL = "INSERT INTO pf_servers VALUES('', '$curtime',  '$serverno', '$localip', '$publicip', '$thermal', '$freq')";
mysqli_query($conn, $SQL);

echo"<scrip>history.back();</script>";
die;

?>
<!-- Footer -->
<?php include("./footer.php"); ?>
