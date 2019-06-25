<html style="overflow:hidden; border: 0px solid grey;">

<head>
    <link rel="stylesheet" href="styles/layout.css" type="text/css">
    <script type="text/javascript" src="/scripts/moment.js"></script>
    <!--    <script type="text/javascript" src="https://code.jquery.com/jquery-1.12.4.min.js"></script>-->
</head>

<body>
    <div class="wrapper row2">
        <div id="container" class="clear" style="padding:0px">
            <div id="homepage">
                <section id="services" class="clear" style="margin: 0px">
                    <iframe width="0" height="0" style="border:0" name="dummyframe" id="dummyframe" style="display: none;"></iframe>
                    <article style="border: 0px solid black;">
                        <figure><img src="images/con0.png" width="32" height="32" alt="" id="status_img"></figure>
                        <strong>Device <span id="devnum">X</span> - <span id="cur_status">X</span></strong>

                        <form action="/index2.php" method="post" target="dummyframe" style="margin: 0;">
                            ON <input type="radio" name="control" id="radio_on" checked value="1" onclick="setSubmitButton()">
                            / OFF <input type="radio" name="control" id="radio_off" value="0" onclick="setSubmitButton()"><br>
                            예약: <input type="text" id="res_input" placeholder="년-월-일&nbsp;시:분:초" name="reservation" onFocus="setRes(this)" onclick="setSubmitButton()" onblur="setSubmitButton()"><br>
                            반복: <input type="text" id="rep_input" placeholder="시:분:초" name="repeat" onFocus="setRep(this)" onclick="setSubmitButton()" onblur="setSubmitButton()">
                            <br>
                            월<input type="checkbox" name="day[]" value="1">&nbsp; 화<input type="checkbox" name="day[]" value="2">&nbsp;
                            수<input type="checkbox" name="day[]" value="3">&nbsp; 목<input type="checkbox" name="day[]" value="4">&nbsp;
                            금<input type="checkbox" name="day[]" value="5">&nbsp; 토<input type="checkbox" name="day[]" value="6">&nbsp;
                            일<input type="checkbox" name="day[]" value="7">
                            <input type="hidden" id="hid_devnum" name="devnum" value="X">
                            <!-- <input type="hidden" id="hid_onoff" name="onoff" value="X"><br> -->
                            <input type="submit" id="onoff" value="Turn On" class="orange">
                        </form>

                        <br>
                        <p style="padding: 0 0 0 10px;">last online: <a onclick="return false" id="last_online"> X </a> ago</p>
                        <p id="showReserve" style="border: 1px black solid; padding: 0 0 0 10px;">
                        </p>
                    </article>
                </section>
            </div>
        </div>
    </div>
    <!--    <script>-->
    <!--    </script>-->
</body>

</html>

<script type="text/javascript" src="/scripts/devices.js"></script>