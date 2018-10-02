<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);

include_once("./head.php");
include_once('dist/Medoo.php');
include_once('SocketPing.php');

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

    if($menu>10 || $menu<0){
        die("INVALID REQUEST FORMANT");
    }
}else{
    $menu = 404;
    die
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
            <!-- One Quarter -->
            <h1>Server List</h1>
            <section id="services" class="clear">
                <article>
                    <figure><img src="assets/images/ethernet_green.png" width="32" height="32" alt=""></figure>
                    <strong>Server1 - Odroid HC1</strong>
                    <p>This is a W3C compliant free website template from <a href="http://www.os-templates.com/" title="Free Website Templates">OS Templates</a>. For full terms of use of this template please read our <a href="http://www.os-templates.com/template-terms">website template licence</a>.</p>
                    <p class="more"><a href="/server.php?serverno=1">Detailed Info &raquo;</a></p>
                </article>

                <article>
                    <figure><img src="assets/images/ethernet_orange.png" width="32" height="32" alt=""></figure>
                    <strong>Server2 - RasPi 2B</strong>
                    <p>You can use and modify the template for both personal and commercial use. You must keep all copyright information and credit links in the template and associated files.</p>
                    <p class="more"><a href="/server.php?serverno=2">Detailed Info &raquo;</a></p>
                </article>

                <article class="last">
                    <figure><img src="assets/images/ethernet_orange.png" width="32" height="32" alt=""></figure>
                    <strong>Server3 - IWINV waterplus</strong>
                    <p>For more HTML5 templates visit <a href="http://www.os-templates.com/">free website templates</a>. Orciinterdum condimenterdum nullamcorper elit nam curabitur laoreet met praesenean et iaculum.</p>
                    <p class="more"><a href="/server.php?serverno=3">Detailed Info &raquo;</a></p>
                </article>
            </section>

            <section id="code">
                <p>code to view PHP errors:<br><br>
                    ini_set('display_errors', 1);<br>
                    ini_set('display_startup_errors', 1);<br>
                    error_reporting(E_ALL);</p>
            </section>

            <!-- / One Quarter -->
            <section id="shout">
            <br><br>
            <hr>
                <p>random text</p>
            </section>

            <!-- 4pics -->
            <!-- <section id="latest" class="clear">
                <article class="one_quarter">
                    <figure><img src="assets/images/demo/215x315.gif" width="215" height="315" alt="">
                        <figcaption>Image Caption Here</figcaption>
                    </figure>
                </article>
                <article class="one_quarter">
                    <figure><img src="assets/images/demo/215x315.gif" width="215" height="315" alt="">
                        <figcaption>Image Caption Here</figcaption>
                    </figure>
                </article>
                <article class="one_quarter">
                    <figure><img src="assets/images/demo/215x315.gif" width="215" height="315" alt="">
                        <figcaption>Image Caption Here</figcaption>
                    </figure>
                </article>
                <article class="one_quarter lastbox">
                    <figure><img src="assets/images/demo/215x315.gif" width="215" height="315" alt="">
                        <figcaption>Image Caption Here</figcaption>
                    </figure>
                </article>
            </section> -->

            <!-- coode snippets -->
            <!-- <section id="code">
                <p>I can place code sniffets in here</p>
            </section> -->

        </div>

        <!-- left column -->
        <p id='container'>
            <aside id="left_column">
                <h2 class="title">Links</h2>
                <nav>
                    <ul>
                        <li><a href="/">Paperframe</a></li>
                        <li><a href="www.dothome.co.kr">Dothome Hosting</a></li>
                        <li><a href="/myadmin">paperframe - PHPMyAdmin</a></li>
                        <li><a href="http://www.os-templates.com/free-basic-html5-templates/basic-87">Basic 87 template</a></li>
                        <li class="last"><a href="https://dribbble.com/shots/4081186-Type-widget-for-web">Dribble-theme</a></li>
                    </ul>
                </nav>
                <!-- /nav -->
            </aside>
        <p id='container'>


        <!-- main content -->
        <div id="content">
            <section id="services" class="last clear">
                <ul>
                    <li>
                        <article class="clear">
                            <figure><img src="assets/images/demo/180x150.gif" alt="">
                                <figcaption>
                                    <h2>ㅁㄴㅇㄻㄴㅇㄻㄴㅇㄹ</h2>
                                    <p>ㅁㄴㅇㄻㄴㅇㄻㄴㅇㄹ<a href="https://www.google.com">GOOGLE</a></p>
                                    <footer class="more"><a href="#">Read More &raquo;</a></footer>
                                </figcaption>
                            </figure>
                        </article>
                    </li>
                    <li class="last">
                        <article class="clear">
                            <figure><img src="assets/images/demo/180x150.gif" alt="">
                                <figcaption>
                                    <h2>ㅁㄴㅇㄻㄴㅇㄻㄴㅇㄻㄴㅇㄻㄴㅇㄹ</h2>
                                    <p>ㅁㄴㅇㄻㄴㅇㄹ</p>
                                    <footer class="more"><a href="#">Read More &raquo;</a></footer>
                                </figcaption>
                            </figure>
                        </article>
                    </li>
                </ul>
            </section>
        </div>

    </div>
</div>
<!-- Footer -->
<?php include("./footer.php"); ?>
