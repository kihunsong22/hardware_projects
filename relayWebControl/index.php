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

$SQL = "SELECT * FROM devices ORDER BY dev_num";
$result = mysqli_query($conn, $SQL);

$num_rows = mysqli_num_rows($result);
$devices[$num_rows][3] = "0";
for($i=0; $i<$num_rows; $i++){
    $row = mysqli_fetch_assoc($result);
    $devices[$i][0] = $row['dev_num'];
    $devices[$i][1] = $row['update_time'];
    $devices[$i][2] = $row['cur_status'];
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
      <h1><a href="#">IOTSV</a></h1>
<!--      <h2>Free HTML5 Website Template</h2>-->
    </div>
    <form action="#" method="post">
      <fieldset>
        <legend>Search:</legend>
        <input type="text" value="Search Our Website&hellip;" onFocus="this.value=(this.value=='Search Our Website&hellip;')? '' : this.value ;">
        <input type="submit" id="sf_submit" value="submit">
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
<!--        SAMPLES-->
<!--        <article>-->
<!--          <figure><img src="images/demo/32x32.gif" width="32" height="32" alt=""></figure>-->
<!--          <strong>Lorum ipsum dolor</strong>-->
<!--          <p>This is a W3C compliant free website template from <a href="https://www.os-templates.com/" title="Free Website Templates">OS Templates</a>. For full terms of use of this template please read our <a href="https://www.os-templates.com/template-terms">website template licence</a>.</p>-->
<!--          <p class="more"><a href="#">Read More &raquo;</a></p>-->
<!--        </article>-->

<!--        <article>-->
<!--          <figure><img src="images/demo/32x32.gif" width="32" height="32" alt=""></figure>-->
<!--          <strong>Lorum ipsum dolor</strong>-->
<!--          <p>You can use and modify the template for both personal and commercial use. You must keep all copyright information and credit links in the template and associated files.</p>-->
<!--          <p class="more"><a href="#">Read More &raquo;</a></p>-->
<!--        </article>-->

<!--        <article class="last">-->
<!--          <figure><img src="images/demo/32x32.gif" width="32" height="32" alt=""></figure>-->
<!--          <strong>Lorum ipsum dolor</strong>-->
<!--          <p>For more HTML5 templates visit <a href="https://www.os-templates.com/">free website templates</a>. Orciinterdum condimenterdum nullamcorper elit nam curabitur laoreet met praesenean et iaculum.</p>-->
<!--          <p class="more"><a href="#">Read More &raquo;</a></p>-->
<!--        </article>-->
          <?php
          $article = '<article><figure><img src="images/con###IMG###.png" width="32" height="32" alt=""></figure><strong>Device ###DEVNUM### - ###STATUS###</strong><br><br><form action="#" method="post"><br><!--<input type="text" value="Search Our Website&hellip;">--><input type="submit" id="" value="submit"></form><br><p>last online: <a onclick="return false">###SEC###</a>ago</p></article>';

          for($i=0; $i<$num_rows; $i++){
              $downtime = elapsed_time(strtotime($devices[$i][1]));
              $online = strpos($downtime, "mins")==false ? 1 : 0;
              $cur_status = $devices[$i][2]==1 ? "on" : "off";

              $html = $article;
              $html = str_replace("###DEVNUM###", $devices[$i][0], $html);
              $html = str_replace("###IMG###", $online, $html);
              $html = str_replace("###SEC###", $downtime, $html);
              $html = str_replace("###STATUS###", $cur_status, $html);

              if($i % 3 == 0){
                  str_replace("<article>", "<article class=\"last\">", $html);
              }

              echo $html;
          }
          ?>


<!--        <article>-->
<!--          <figure><img src="images/con###IMG###.png" width="32" height="32" alt=""></figure>-->
<!--          <strong>Device ##DEVNUM##</strong><br><br>-->
<!--          <form action="#" method="post">-->
<!--            <input type="text" value="Search Our Website&hellip;">-->
<!--            <input type="submit" id="" value="submit">-->
<!--          </form>-->
<!--          <p>last online: <a onclick="return false">###SEC##</a>ago</p>-->
<!--        </article>-->

        <article>
          <figure><img src="images/con0.png" width="32" height="32" alt=""></figure>
          <strong>Lorum ipsum dolor</strong>
          <p>You can use and modify the template for both personal and commercial use. You must keep all copyright information and credit links in the template and associated files.</p>
          <p class="more"><a href="#">Read More &raquo;</a></p>
        </article>

        <article class="last">
          <figure><img src="images/con0.png" width="32" height="32" alt=""></figure>
          <strong>Lorum ipsum dolor</strong>
          <p>For more HTML5 templates visit <a href="https://www.os-templates.com/">free website templates</a>. Orciinterdum condimenterdum nullamcorper elit nam curabitur laoreet met praesenean et iaculum.</p>
          <p class="more"><a href="#">Read More &raquo;</a></p>
        </article>
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
