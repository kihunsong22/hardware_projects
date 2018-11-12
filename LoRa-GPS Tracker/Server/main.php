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

if(isset($_GET['limit'])){
    $GPS_limit = $_GET['limit'];

    if($GPS_limit<0){
        die("INVALID REQUEST FORMANT");
    }
}else{
    $GPS_limit = 600;
}

$SQL = "SELECT * FROM data ORDER BY idx DESC LIMIT ".$GPS_limit;
$gps_result = mysqli_query($conn, $SQL);

$info = "";  // filtered DB printout
$filter_num = 0;  // number of filtered DB printouts
$info_all = ""; // DB printout
$gps_pos[][] = 0;  // list of proper gps positions
$i=0;
while($row = mysqli_fetch_assoc($gps_result)){
    $white = "&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp|&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp";
    $msg = "IDX:".$row['idx'].$white."LATITUDE:".$row['gps_lati'].$white."LONGTITUDE:"
    .$row['gps_long'].$white."debug:".$row['debug'].$white."TIME:".$row['timestamp']."<br><br>";
    $info_all .= $msg;

    if($row['gps_lati']>0 && $row['gps_long']>0 && $row['debug']!=404){
        $white = "&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp|&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp";
        $msg = "IDX:".$row['idx'].$white."LATITUDE:".$row['gps_lati'].$white."LONGTITUDE:"
        .$row['gps_long'].$white."debug:".$row['debug'].$white."TIME:".$row['timestamp']."<br><br>";
        $info .= $msg;
        
        $gps_pos[$filter_num][0] = $row['gps_lati'];
        $gps_pos[$filter_num][1] = $row['gps_long'];
        $filter_num++;
    }
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
            <!-- <script>
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
                    // 37.3419, 126.83152
                    });
                }
            </script> -->
            <script>
                var map;
                function initMap() {
                    map = new google.maps.Map(document.getElementById('map'), {
                    center: {lat: 37.341894, lng: 126.831500},
                    zoom: 17
                    });

                    // var marker = new google.maps.Marker({
                    // position: {lat: 37.3419, lng: 126.83152},
                    // map: map,
                    // title: 'KDMHS'
                    // // 37.3419, 126.83152
                    // });


                    <?php
                    // for($i=0;$i<$filter_num;$i++){  // via marker
                    //     $gps_lati = $gps_pos[$i][0];
                    //     $gps_long = $gps_pos[$i][1];

                    //     $gmapLine = "\nvar marker = new google.maps.Marker({position: {lat: ###, lng: @@@},map: map,title: 'GPS'});\n";
                    //     $gmapLine = str_replace("###", "$gps_lati", $gmapLine);
                    //     $gmapLine = str_replace("@@@", "$gps_long", $gmapLine);

                    //     echo $gmapLine;
                    // }
                    ?>


                    <?php
                    echo "var GPSpath = [";
                    for($i=0;$i<$filter_num;$i++){  // via polyline
                        $gps_lati = $gps_pos[$i][0];
                        $gps_long = $gps_pos[$i][1];

                        $gmapLine = "\n{lat: ###, lng: @@@},";
                        $gmapLine = str_replace("###", "$gps_lati", $gmapLine);
                        $gmapLine = str_replace("@@@", "$gps_long", $gmapLine);

                        echo $gmapLine;
                        // echo $gmapLine;
                    }
                    echo "\n];";

                    // marker for latest location
                    $gps_lati = $gps_pos[0][0];
                    $gps_long = $gps_pos[0][1];

                    $gmapLine = "\nvar marker = new google.maps.Marker({position: {lat: ###, lng: @@@},map: map,title: 'GPS'});\n";
                    $gmapLine = str_replace("###", "$gps_lati", $gmapLine);
                    $gmapLine = str_replace("@@@", "$gps_long", $gmapLine);

                    echo $gmapLine;
                    ?>

                    var GPSpath = new google.maps.Polyline({
                    path: GPSpath,
                    geodesic: true,
                    strokeColor: '#f36e27',
                    strokeOpacity: 1.0,
                    strokeWeight: 2
                    });

                    GPSpath.setMap(map);
                }
            </script>
        </div>

        <div id="homepage">
            <section>
            <!-- blank space -->
            </section>
            <section id="code">
            <p>
                <h5>Filtered DB printout</h5><br><br>
                <?php
                echo($info);
                ?>
            </p>
            </section>

            <section id="code">
            <p>
                <h5>All DB printout</h5><br><br>
                <?php
                echo($info_all);
                ?>
            </p>
            </section>
        </div>
        
    </div>
</div>
<!-- Footer -->
<?php include("./footer.php"); ?>
