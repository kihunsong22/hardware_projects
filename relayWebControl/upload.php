<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);
date_default_timezone_set('Asia/Seoul');

include_once('dbconnect.php');


if(isset($_GET['pass'])){
    $upload_pass = $_GET['pass'];
    if(strcmp($upload_pass, "pass1406002454") != 0){
        die("wrong password");
    }
}else{
    die("no password");
}

if(isset($_GET['devnum'])){
    $devnum = $_GET['devnum'];
}else{
    die("no devnum");
}

if(isset($_GET['status'])){
    $cur_status = $_GET['status'];
}else{
    die("no status");
}

$SQL = "SELECT * FROM devices WHERE dev_num='$devnum'";
$result = mysqli_query($conn, $SQL);

if(mysqli_num_rows($result)==1){
    $SQL = "UPDATE devices SET update_time=CURRENT_TIMESTAMP, cur_status='$cur_status' WHERE dev_num='$devnum'";
    mysqli_query($conn, $SQL);
//    echo "update";

    $SQL = "SELECT set_status FROM devices WHERE dev_num='$devnum'";
    $result = mysqli_query($conn, $SQL);
    while($row = mysqli_fetch_array($result)){
        $status = $row['0'];
        echo $status;
    }
}else{
    $SQL = "INSERT INTO devices (dev_num, set_status) VALUES('$devnum', '0')";
    mysqli_query($conn, $SQL);
//    echo "insert";
}