<?php
include_once("./head.php");

ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);
?>
<!-- content -->
<div class="wrapper row2">
    <div id="container" class="clear">
        <!-- content body -->
        <div id="homepage">
            <!-- One Quarter -->
            <h1>Server List</h1>
            <section id="services" class="clear">
                <article>
                    <figure><img src="assets/images/ethernet_<?php if($status==0){echo"green";}else{echo"orange";} ?>.png" width="32" height="32" alt=""></figure>
                    <strong>Server1 - Odroid HC1</strong>
                    <p>This is a W3C compliant free website template from <a href="http://www.os-templates.com/" title="Free Website Templates">OS Templates</a>. For full terms of use of this template please read our <a href="http://www.os-templates.com/template-terms">website template licence</a>.</p>
                    <p class="more"><a href="/server.php?serverno=1">Detailed Info &raquo;</a></p>
                </article>

                <article>
                    <figure><img src="assets/images/ethernet_<?php if($status==0){echo"green";}else{echo"orange";} ?>.png" width="32" height="32" alt=""></figure>
                    <strong>Server2 - RasPi 2B</strong>
                    <p>You can use and modify the template for both personal and commercial use. You must keep all copyright information and credit links in the template and associated files.</p>
                    <p class="more"><a href="/server.php?serverno=2">Detailed Info &raquo;</a></p>
                </article>

                <article class="last">
                    <figure><img src="assets/images/ethernet_<?php if($status==0){echo"green";}else{echo"orange";} ?>.png" width="32" height="32" alt=""></figure>
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
                <p>Below may not be very useful.</p>
            </section>

            <!-- 4pics -->
            <section id="latest" class="clear">
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
            </section>

            <!-- coode snippets -->
            <section id="code">
                <p>I can place code sniffets in here</p>
            </section>

        </div>

        <!-- left column -->
        <container>
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
        </container>


        <!-- main content -->
        <div id="content">
            <section id="services" class="last clear">
                <ul>
                    <li>
                        <article class="clear">
                            <figure><img src="assets/images/demo/180x150.gif" alt="">
                                <figcaption>
                                    <h2>우리학교 너무춥다</h2>
                                    <p>구글이 최고지 <a href="https://www.google.com">GOOGLE</a></p>
                                    <footer class="more"><a href="#">Read More &raquo;</a></footer>
                                </figcaption>
                            </figure>
                        </article>
                    </li>
                    <li class="last">
                        <article class="clear">
                            <figure><img src="assets/images/demo/180x150.gif" alt="">
                                <figcaption>
                                    <h2>방과후실 와이파이는 FTP도 막아놨어</h2>
                                    <p>그래도 노트북할 수 있는게 어디야</p>
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
