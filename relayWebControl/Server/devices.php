<?php

//use the code below to embed device control to other external pages
//<iframe src="http://iotsv.cafe24.com/devices.php?dev_num=1" frameborder="0" width="300" height="163" scrolling="no"></iframe>

ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);

include_once('dbconnect.php');

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

echo '<link rel="stylesheet" href="styles/layout.css" type="text/css">';

// SHOW DEVICES
$article = '<iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe" style="display: none;"></iframe><article><figure><img src="images/con###IMG###.png" width="32" height="32" alt="">
</figure><strong>Device ###DEVNUM### - ###STATUS###</strong><br><form action="/index.php" method="post" target="dummyframe" onSubmit="setTimeout(function(){window.location.reload();}, 100)">
<input type="hidden" name="devnum" value="###DEVNUM###"><input type="hidden" name="onoff" value="###ONOFF###">
<!---###reserve###---><!---###---><br><input type="submit" id="onoff" value="###ONOFFTEXT###"></form><br>
<p>last online: <a onclick="return false">###SEC###</a>ago</p></article>';
$reserveField ='<input type="text" placeholder=year-month-day&nbsp;hour:min:sec name="reservation" onFocus="this.value=(this.value==\'\' ? \''.$curtime.'\' : this.value);"><br>';
$reserveText = '<p>예약: ###RESTIME###</p>';
$repeatText = '<p>반복: ###REP1### ~ ###REP2###</p>';

$SQL = "SELECT * FROM devices ORDER BY dev_num";
$result = mysqli_query($conn, $SQL);

$num_rows = mysqli_num_rows($result);
$devices[$num_rows][] = "0";
for($i=1; $i<=$num_rows; $i++){
    $row = mysqli_fetch_assoc($result);
    $devices[$i][0] = $row['dev_num'];
    $devices[$i][1] = $row['update_time'];
    $devices[$i][2] = $row['cur_status'];
    $devices[$i][3] = $row['set_status'];
    $devices[$i][4] = $row['reserve'];
    $devices[$i][5] = $row['repeat_start'];
    $devices[$i][6] = $row['repeat_stop'];
}

function show_devices($dev_num){
    global $devices;
    global $article, $reserveText, $repeatText, $reserveField;

    $downtime = elapsed_time(strtotime($devices[$dev_num][1]));
    $online = strpos($downtime, "min")==false ? 1 : 0;
    $cur_status = $devices[$dev_num][2]==1 ? "on" : "off";
    $set_status_text = $devices[$dev_num][3]=="1" ? "TURN OFF" : "TURN ON";
    $set_status = $devices[$dev_num][3]=="1" ? "0" : "1";
    $set_reserve = $devices[$dev_num][4]==NULL ? 0 : 1;
    $ser_repeat = $devices[$dev_num][5]==NULL ? 0 : 1;

    if(strpos($downtime, "min") == true)    {    $online = 0;    }
    else if(strpos($downtime, "hour") == true)    {    $online = 0;    }
    else if(strpos($downtime, "day") == true)    {    $online = 0;    }
    else if(strpos($downtime, "week") == true)    {    $online = 0;    }
    else if(strpos($downtime, "month") == true)    {    $online = 0;    }
    else if(strpos($downtime, "month") == true)    {    $online = 0;    }
    else     {    $online = 1;    }


    $html = $article;
    $html = str_replace("###DEVNUM###", $devices[$dev_num][0], $html);
    $html = str_replace("###IMG###", $online, $html);
    $html = str_replace("###SEC###", $downtime, $html);
    $html = str_replace("###STATUS###", $cur_status, $html);
    $html = str_replace("###ONOFF###", $set_status , $html);
    if($set_reserve){  // DB에 reserve가 존재하므로 예약 취소
        $html = str_replace("###ONOFFTEXT###", "예약 취소" , $html);
        $html = str_replace("<!---###reserve###--->", $reserveText, $html);

        $reserveNotifyText = $devices[$dev_num][4];
        $reserveNotifyText .= " 에 ".($devices[$dev_num][3]==1 ? "켜기" : "끄기");
        $reserveNotifyText .=
        $html = str_replace("###RESTIME###", $reserveNotifyText, $html);
    }elseif($ser_repeat){  // 반복 취소
        $html = str_replace("###ONOFFTEXT###", "반복 취소" , $html);
        $html = str_replace("<!---###reserve###--->", $repeatText, $html);
        $html = str_replace("###REP1###", $devices[$dev_num][5], $html);
        $html = str_replace("###REP2###", $devices[$dev_num][6], $html);

    }else{
        $html = str_replace("<!---###reserve###--->", $reserveField, $html);
        $html = str_replace("###ONOFFTEXT###", $set_status_text , $html);
    }

    if(($dev_num+1) % 3 == 0){
        $html = str_replace("<article>", '<article class="last">', $html);
    }

    return $html;
}

if(isset($_GET['dev_num'])){
    $dev_num = $_GET['dev_num'];
    echo '<html><body><div class="wrapper row2"><div id="container" class="clear" style="padding:0px"><div id="homepage"><section id="services" class="clear" style="margin: 0px">
	<iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"></iframe>';
    $html = show_devices($dev_num);
    echo $html;
    echo '</section></div></div></div></body></html>';
}
