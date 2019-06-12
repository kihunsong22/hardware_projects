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
    return $result ? $result : '0 sec ';
}

function is_timestamp($timestamp) {
    if(strtotime(date('d-m-Y H:i:s',$timestamp)) === (int)$timestamp) {
        return 1;
    } else
        return false;
}

function is_time($time) {
    if(preg_match("/(?:[01]\d|2[0123]):(?:[012345]\d):(?:[012345]\d)/", $time)){
        return 1;
    }else{
        return 0;
    }

//    if(strtotime(date('H:i:s',$timestamp)) === (int)$timestamp) {
//        return 1;
//    } else
//        return 0;
}

$publicip = get_client_ip();
$curtime = (new DateTime())->format("Y-m-d H:i:s");

foreach($_POST as $key => $value){
    echo "<script>console.log('$key: $value')</script>";
}


if(isset($_POST['devnum']) && isset($_POST['onoff'])) {
    $set_devnum = $_POST['devnum'];
    $set_onoff = $_POST['onoff'];
    $reserve = $_POST['reservation'];
    $repeat = explode(" ", $reserve);

    $SQL = "SELECT * FROM devices WHERE dev_num='$set_devnum'";
    $result = mysqli_query($conn, $SQL);
    $row = mysqli_fetch_assoc($result);
    $is_reserve_set = $row['reserve']==NULL ? 0 : 1;
    $is_repeat_set = $row['repeat_start']==NULL ? 0 : 1;
    if($is_reserve_set){  // 예약 취소
        $SQL = "UPDATE devices SET set_status='$set_onoff', reserve=NULL WHERE dev_num='$set_devnum'";
        mysqli_query($conn, $SQL);

        echo "<script>console.log('예약 취소');</script>";
    }elseif($is_repeat_set){  // 반복 취소
        $SQL = "UPDATE devices SET set_status='$set_onoff', repeat_start=NULL, repeat_stop=NULL WHERE dev_num='$set_devnum'";
        mysqli_query($conn, $SQL);

        echo "<script>console.log('반복 취소');</script>";
    }elseif(is_time($repeat[0]) && is_time($repeat[1])){  // 반복 예약
        $a = $repeat[0];
        $b = $repeat[1];
        $SQL = "UPDATE devices SET set_status='$set_onoff', repeat_start='$a', repeat_stop='$b' WHERE dev_num='$set_devnum'";
        mysqli_query($conn, $SQL);

        echo "<script>console.log('반복 예약');</script>";
    }elseif(is_timestamp($reserve)){  // 예약
        $SQL = "UPDATE devices SET set_status='$set_onoff', reserve='$reserve' WHERE dev_num='$set_devnum'";
        mysqli_query($conn, $SQL);

        echo "<script>console.log('예약');</script>";
    }else{  // 단순 온오프
        $SQL = "UPDATE devices SET set_status='$set_onoff' WHERE dev_num='$set_devnum'";
        mysqli_query($conn, $SQL);

        echo "<script>console.log('온오프');</script>";
    }
}

if(isset($_POST['remove'])){
    $remove_post = $_POST['remove'];
    if($remove_post != ""){
        $remove_post = addslashes($remove_post);

        $SQL = "DELETE FROM devices WHERE dev_num='$remove_post'";
        mysqli_query($conn, $SQL);
    }
    if($reset_page == 1)
        echo "<script>window.location.href='./'</script>";
}

$SQL = "SELECT * FROM devices ORDER BY dev_num";
$result = mysqli_query($conn, $SQL);

$num_rows = mysqli_num_rows($result);

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
      </p>
    </section>
    <!-- main content -->
    <div id="homepage">
      <!-- services area -->
      <h1>Controls</h1>
      <section id="services" class="clear">

          <?php
          include_once ('devices.php');
          for($i=1; $i<=$num_rows; $i++){
              $html = show_devices($i);
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
      <br><br><br><br><br><br><br>
      <!-- / One Quarter -->
    </div>
    <!-- / content body -->
  </div>
</div>
<!-- Footer -->
<div class="wrapper row3">
  <footer id="footer" class="clear">
    <p class="fl_left">Copyright &copy; 2019 - All Rights Reserved - <a href="#">IOTSV</a></p>
    <p class="fl_right">Template by <a target="_blank" href="https://www.os-templates.com/" title="Free Website Templates">OS Templates</a></p>
  </footer>
</div>
</body>
</html>
