<?php
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

?>
<!-- content -->
<div class="wrapper row2">
    <div id="container" class="clear">
        <!-- content body -->
        <div id="homepage">
            <!-- One Quarter -->
            <h1><?php echo"SERVER $serverno"; ?></h1>
            <section id="code">
                <form class="" action="upload.php" method="post">
                    <input type="text" name="localip" value="256.256.256.256" placeholder="localip" readonly> - ip<br>
                    <input type="text" name="thermal" placeholder="thermal" readonly> - th<br>
                    <input type="text" name="freq" placeholder="freq" readonly> - fr<br>
                    <input type="submit" value="Submit"><br>
                </form>
            </section>
            <section id="code">
                <?php
                echo("serverno : $serverno<br>");
                echo("publicIP : $publicip<br>");
                echo("localip : $localip<br>");
                echo("thermal : $thermal<br>");
                echo("curtime : $curtime<br><br><br>");
                checkalive_echo($server[0], $server[1]);
                ?>
            </section>
        </div>
    </div>
</div>
<!-- Footer -->
<?php include("./footer.php"); ?>
