<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);
date_default_timezone_set('Asia/Seoul');

include_once('./dbconnect.php');

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
    return $result ? $result : '0 secs ';
}

$publicip = get_client_ip();
$curtime = (new DateTime())->format("Y-m-d H:i:s");

foreach($_POST as $key => $value){
    echo "<script>console.log('$key: $value')</script>";
}

if(isset($_POST['devnum'])){
    $set_devnum = $_POST['devnum'];
    $set_onoff = $_POST['onoff'];
    echo "<script>console.log('Set $set_devnum to $set_onoff')</script>";

    $SQL = "UPDATE devices SET set_status='$set_onoff' WHERE dev_num='$set_devnum'";
    mysqli_query($conn, $SQL);

    echo "<script>window.location.href='./'</script>";  // remove POST data after processing
}

if(isset($_POST['remove'])){
    $remove = $_POST['remove'];
    $remove = addslashes($remove);

    $SQL = "DELETE FROM devices WHERE dev_num='$remove'";
    mysqli_query($conn, $SQL);
    echo "<script>window.location.href='./'</script>";  // remove POST data after processing
}

$SQL = "SELECT * FROM devices ORDER BY dev_num";
$result = mysqli_query($conn, $SQL);

$num_rows = mysqli_num_rows($result);
$devices[$num_rows][3] = "0";
for($i=0; $i<$num_rows; $i++){
    $row = mysqli_fetch_assoc($result);
    $devices[$i][0] = $row['dev_num'];
    $devices[$i][1] = $row['update_time'];
    $devices[$i][2] = $row['cur_status'];
    $devices[$i][3] = $row['set_status'];
}
?>

<!DOCTYPE html>
<html lang="en" dir="ltr">
<head>
<title>IOTSV</title>
<meta charset="UTF-8">
<link rel="stylesheet" href="styles/layout.css" type="text/css">
<!--[if lt IE 9]><script src="scripts/html5shiv.js"></script><![endif]-->
</head>
<body>
<div class="wrapper row1">
  <header id="header" class="clear">
    <div id="hgroup">
      <h1><a href="/">IOTSV</a></h1>
      <h2>Control Interface</h2>
    </div>
    <form action="#" method="post" onsubmit="return confirm('정말로 삭제하시겠습니까?')">
      <fieldset>
        <legend>Search:</legend>
        <input type="text" placeholder="Remove Device" name="remove">
        <input type="submit" id="sf_submit" value="remove">
      </fieldset>
    </form>
<!--    <nav>-->
<!--      <ul>-->
<!--        <li><a href="#">Text Link</a></li>-->
<!--        <li><a href="#">Text Link</a></li>-->
<!--        <li><a href="#">Text Link</a></li>-->
<!--        <li><a href="#">Text Link</a></li>-->
<!--        <li class="last"><a href="#">Text Link</a></li>-->
<!--      </ul>-->
<!--    </nav>-->
  </header>
</div>
<!-- content -->
<div class="wrapper row2">
  <div id="container" class="clear">
<!--     content body -->
<!--    <section id="slider"><a href="#"><img src="images/demo/960x360.gif" alt=""></a></section>-->
    <section id="shout">
      <p>
          <?php echo "number of devices: $num_rows"; ?>
          <br>예약 기능 만들기, 아두이노 코드
          <br>password, devnum, status
      </p>
    </section>
    <!-- main content -->
    <div id="homepage">
      <!-- services area -->
      <h1>Controls</h1>
      <section id="services" class="clear">

          <?php
          $article = '<article><figure><img src="images/con###IMG###.png" width="32" height="32" alt="">
</figure><strong>Device ###DEVNUM### - ###STATUS###</strong><br><form action="/" method="post">
<input type="hidden" name="devnum" value="###DEVNUM###"><input type="hidden" name="onoff" value="###ONOFF###">
예약: <br><input type="text" placeholder=year-month-day&nbsp;hour:min:sec name="reservation"><br><br>
<input type="submit" id="sf_submit" value="###ONOFFTEXT###"></form><br>
<p>last online: <a onclick="return false">###SEC###</a>ago</p></article>';

          for($i=0; $i<$num_rows; $i++){
              $downtime = elapsed_time(strtotime($devices[$i][1]));
              $online = strpos($downtime, "min")==false ? 1 : 0;
              $cur_status = $devices[$i][2]==1 ? "on" : "off";
              $set_status_text = $devices[$i][3]=="1" ? "TURN OFF" : "TURN ON";
              $set_status = $devices[$i][3]=="1" ? "0" : "1";

              if(strpos($downtime, "min") == true)    {    $online = 0;    }
              else if(strpos($downtime, "hour") == true)    {    $online = 0;    }
              else if(strpos($downtime, "day") == true)    {    $online = 0;    }
              else if(strpos($downtime, "week") == true)    {    $online = 0;    }
              else if(strpos($downtime, "month") == true)    {    $online = 0;    }
              else if(strpos($downtime, "month") == true)    {    $online = 0;    }
              else     {    $online = 1;    }


              $html = $article;
              $html = str_replace("###DEVNUM###", $devices[$i][0], $html);
              $html = str_replace("###IMG###", $online, $html);
              $html = str_replace("###SEC###", $downtime, $html);
              $html = str_replace("###STATUS###", $cur_status, $html);
              $html = str_replace("###ONOFFTEXT###", $set_status_text , $html);
              $html = str_replace("###ONOFF###", $set_status , $html);

              if(($i+1) % 3 == 0){
                  $html = str_replace("<article>", '<article class="last">', $html);
              }

              echo $html;
          }
          ?>

<!--        <article>-->
<!--          <figure><img src="images/con0.png" width="32" height="32" alt=""></figure>-->
<!--          <strong>Lorum ipsum dolor</strong>-->
<!--          <p>You can use and modify the template for both personal and commercial use. You must keep all copyright information and credit links in the template and associated files.</p>-->
<!--          <p class="more"><a href="#">Read More &raquo;</a></p>-->
<!--        </article>-->
<!---->
<!--        <article class="last">-->
<!--          <figure><img src="images/con0.png" width="32" height="32" alt=""></figure>-->
<!--          <strong>Lorum ipsum dolor</strong>-->
<!--          <p>For more HTML5 templates visit <a href="https://www.os-templates.com/">free website templates</a>. Orciinterdum condimenterdum nullamcorper elit nam curabitur laoreet met praesenean et iaculum.</p>-->
<!--          <p class="more"><a href="#">Read More &raquo;</a></p>-->
<!--        </article>-->
      </section>
      <!-- / services area -->
      <!-- ########################################################################################## -->
      <!-- ########################################################################################## -->
      <!-- ########################################################################################## -->
      <!-- ########################################################################################## -->
      <!-- One Quarter -->
      <br><br><br><br><br><br><br><br>
      <!-- / One Quarter -->
    </div>
    <!-- / content body -->
  </div>
</div>
<!-- Footer -->
<div class="wrapper row3">
  <footer id="footer" class="clear">
    <p class="fl_left">Copyright &copy; 2018 - All Rights Reserved - <a href="#">Domain Name</a></p>
    <p class="fl_right">Template by <a target="_blank" href="https://www.os-templates.com/" title="Free Website Templates">OS Templates</a></p>
  </footer>
</div>
</body>
</html>
