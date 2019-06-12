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
    $row = mysqli_fetch_assoc($result);
    $status = $row['set_status'];
    $reserve = $row['reserve'];
    $repeat_start = $row['repeat_start'];
    $repeat_stop = $row['repeat_stop'];

    if($reserve!=NULL && (strtotime($reserve) - time())<=0){  // 예약된 시간
        $SQL = "UPDATE devices SET reserve=NULL WHERE dev_num='$devnum'";
        mysqli_query($conn, $SQL);

        echo "@".$status."#";
    }elseif ($reserve!=NULL && (strtotime($reserve) - time())>0){  // 예약이 설정되었으나 시간이 안됨
        $status = $status==0 ? 1 : 0;
        echo "@".$status."#";
    }elseif ($repeat_start!=NULL && $repeat_stop!=NULL &&
        (strtotime(date("H:i:s"))-strtotime($repeat_start)>=0) && (strtotime($repeat_stop)-strtotime(date("H:i:s"))>=0)){  // 반복할 시간
        echo "<script>console.log('반복');</script>";
        echo "@".$status."#";
    }elseif ($repeat_start!=NULL && $repeat_stop!=NULL &&
        ((strtotime(date("H:i:s"))-strtotime($repeat_start))<0 || (strtotime($repeat_stop)-strtotime(date("H:i:s"))<0))){  // 반복이 설정되었으나 시간이 안됨
        echo "<script>console.log('반복시간이안됨');</script>";
        echo "<script>console.log('$repeat_start, $repeat_stop');</script>";
        $a = strtotime($repeat_start);
        $b = strtotime($repeat_stop);
        $c = strtotime(date("H:i:s"));
        echo "<script>console.log('$a, $b, $c');</script>";
        $status = $status==0 ? 1 : 0;
        echo "@".$status."#";
    }else{  // 예약 없음
        echo "@".$status."#";
    }

    $SQL = "UPDATE devices SET update_time=CURRENT_TIMESTAMP, cur_status='$cur_status' WHERE dev_num='$devnum'";
    mysqli_query($conn, $SQL);
}else{
    $SQL = "INSERT INTO devices (dev_num, set_status) VALUES('$devnum', '0')";
    mysqli_query($conn, $SQL);
//    echo "insert";
}
