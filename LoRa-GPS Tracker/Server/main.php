<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);

include_once("./head.php");
include_once('dist/Medoo.php');

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
if(isset($_GET['menu'])){
    $menu = $_GET['menu'];

    if($menu>5 || $menu<0){
        die("INVALID REQUEST FORMANT");
    }
}else{
    $menu = 1;
}
?>

<!-- content -->
<div class="wrapper row2">
    <div id="container" class="clear">
        <!-- content body -->
        <div id="map">
            <!-- Google Maps Div -->
            <script src="https://maps.googleapis.com/maps/api/js?key=AIzaSyCf9yz2xyNW7nPYCjxMIzO29348OSZa1wk&callback=initMap"
            async defer></script>
            <script>
                var map;
                function initMap() {
                    map = new google.maps.Map(document.getElementById('map'), {
                    center: {lat: 37.341894, lng: 126.831500},
                    zoom: 17
                    });

                    var marker = new google.maps.Marker({
                    position: {lat: 37.3419, lng: 126.83152},
                    map: map,
                    title: 'KDMHS'
                    });
                }
            </script>
        </div>

        <div id="homepage">
            <section id="code">
                <p>code to view PHP errors:<br><br>
                    ini_set('display_errors', 1);<br>
                    ini_set('display_startup_errors', 1);<br>
                    error_reporting(E_ALL);
                </p>
            </section>
        </div>
        
    </div>
</div>
<!-- Footer -->
<?php include("./footer.php"); ?>
