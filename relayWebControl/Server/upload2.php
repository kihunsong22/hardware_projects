<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);
date_default_timezone_set('Asia/Seoul');

include_once('dbconnect.php');

function logCon($logString){
    $logString = htmlspecialchars($logString);
    echo "<script>console.log('$logString')</script>";
}

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

$SQL = "UPDATE devices SET `update_time`=CURRENT_TIMESTAMP, `cur_status`='$cur_status' WHERE `dev_num`='$devnum'";
mysqli_query($conn, $SQL);


// check time

$SQL = "SELECT * FROM reserve WHERE `dev_num`='$devnum'";
$result = mysqli_query($conn, $SQL);
$reserve[][] = "";

$dayw = date('w');
logCon("current day of week: ".$dayw);
for($i=0; $i<mysqli_num_rows($result); $i++){
    $row = mysqli_fetch_assoc($result);
    $reserve[$i]['idx'] = $row['idx'];
    $reserve[$i]['control'] = $row['control'];
    $reserve[$i]['timestamp'] = $row['timestamp'];
    $reserve[$i]['time'] = $row['time'];
    $reserve[$i]['day'] = $row['day'];

    if($row['day'] != NULL){
        logCon("반복: {");
    }else{
        logCon("예약: {");
    }

    $udtime = strtotime($row['timestamp']);
    $utime = strtotime($row['time']);

    logCon("    IDX: ".$row['idx']);
    logCon("    control: ".$row['control']);
    if($row['timestamp'] != NULL)   logCon("    timestamp: ".$row['timestamp']);
    if($row['timestamp'] != NULL)   logCon("    UNIX datetime: ".$udtime);
    if($row['time'] != NULL)   logCon("    time: ".$row['time']);
    if($row['time'] != NULL)   logCon("    UNIX time: ".$utime);
    if($row['day'] != NULL)   logCon("    day: ".$row['day']);
    logCon("}");

    if($row['timestamp'] == NULL){  // 반복
        $reserve[$i]['Utimestamp'] = strtotime($row['time']);
    }else{  // 예약
        $reserve[$i]['Utimestamp'] = strtotime($row['timestamp']);
    }

    if($row['timestamp']==NULL){  // 반복일경우 요일 계산
        $lastRepDayPassed = 8;
        $lastRepIndex = 0;
        $repDayPassed[] = 0;
        $day = str_split($row['day']);

        foreach($day as $key=>$value){
            $repDayPassed[$key] = $dayw-intval($value);
            $repDayPassed[$key] = $repDayPassed[$key]<0 ? $repDayPassed[$key]+7 : $repDayPassed[$key];
            if($lastRepDayPassed > $repDayPassed[$key]){
                $lastRepDayPassed = $repDayPassed[$key];
                $lastRepIndex = $i;
            }
        }

        if($lastRepDayPassed > 0){
            $dayToAdd = "+" . $lastRepDayPassed . " day";
            $reserve[$i]['Utimestamp'] = strtotime($dayToAdd, $reserve[$i]['Utimestamp']);
        }
        
        // UNIX timestamp에 lastRepDayPassed만큼 더하기
        // strtotime('+1 day', $timestamp);
    }

    logCon("Utimestamp: ".$reserve[$i]['Utimestamp']);
}

$curTime = time();
logCon("curTime: ".$curTime);

$control = 0;
// $lastRes = 9999999999;
$lastRes = 0;
for($i=0; $i<mysqli_num_rows($result); $i++){
    if( ($lastRes<$reserve[$i]['Utimestamp']) && ($reserve[$i]['Utimestamp']<$curTime) ){
        $lastRes = $reserve[$i]['Utimestamp'];
    }
}
logCon("lastRes: ".$lastRes);

// 하나의 변수에 전체 예약의 가장 최근상태를 저장
// 예약:
//     timestamp 그대로 UNIX timestamp로 변경
// 반복:
//     요일을 계산해서 time을 UNIX timestamp로 변경한 후 날짜 더하기
// 이후 배열변수를 foreach 돌려서 현재 UNIX timestamp와 크지 않은 최댓값을 구하고 그때의 control 값 리턴