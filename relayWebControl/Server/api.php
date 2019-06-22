<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);
date_default_timezone_set('Asia/Seoul');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST');
header("Access-Control-Allow-Headers: X-Requested-With");

include_once('./dbconnect.php');

if(!function_exists("elapsed_time")){
    function elapsed_time($timestamp, $precision = 2) {
        $time = time() - $timestamp;
        $a = array('decade' => 315576000, 'year' => 31557600, 'month' => 2629800, 'week' => 604800, 'day' => 86400, 'hour' => 3600, 'min' => 60, 'sec' => 1);
        $i = 0;
        $result = "";
        foreach($a as $k => $v) {
            $$k = floor($time/$v);
            if ($$k) $i++;
            $time = $i >= $precision ? 0 : $time - $$k * $v;
            $s = $$k > 1 ? 's' : '';
            $$k = $$k ? $$k.' '.$k.$s.' ' : '';
            $result .= $$k;
        }
        return $result ? $result : '0 sec ';
    }
}

$curtime = (new DateTime())->format("Y-m-d H:i:s");

//foreach($_POST as $key => $value){
//    echo "<script>console.log('POST $key: $value')</script>";
//}
//
//foreach($_GET as $key => $value){
//    echo "<script>console.log('GET $key: $value')</script>";
//}


if(!isset($_GET['idx'])){
    die("no idx");
}

$idx = $_GET['idx'];
$idx = intval($idx);

if(is_int($idx)==false || $idx<=0){
    die("idx error");
}

//echo "<script>console.log('idx: $idx')</script>";

$SQL = "SELECT * FROM `devices` ORDER BY `dev_num`";
$result = mysqli_query($conn, $SQL);

$num_rows = mysqli_num_rows($result);

$devices[] = "";
for($i=1; $i<=$num_rows; $i++){
    $row = mysqli_fetch_assoc($result);
    if($row['dev_num'] == $idx){
        $devices["idx"] = $row['idx'];
        $devices["update_time"] = $row['update_time'];
        $devices["dev_num"] = $row['dev_num'];
        $devices["set_status"] = $row['set_status'];
        $devices["cur_status"] = $row['cur_status'];
        $devices["reserve"] = $row['reserve'];
        $devices["repeat_start"] = $row['repeat_start'];
        $devices["repeat_stop"] = $row['repeat_stop'];
    }
}
$timepassed = elapsed_time(strtotime($devices["update_time"]));
$devices["timepassed"] = $timepassed;
$arr = array('devices' => $devices);

$json = json_encode($arr, JSON_PRETTY_PRINT);
echo $json;