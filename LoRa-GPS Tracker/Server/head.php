<!DOCTYPE html>
<html lang="en" dir="ltr">
<head>
    <title>GPSTracker</title>
    <meta charset="UTF-8">
    <link rel="stylesheet" href="assets/css/main-layout.css" type="text/css">
    <?php if(!isset($NOLOGIN))include "login.php"; ?>
</head>
<body>
    <div class="wrapper row1">
        <header id="header" class="clear">
            <div id="hgroup">
                <h1><a href="/main.php">GPS<span>Tracker</span></a></h1>
                <h2>LoRa based GPS tracker</h2>
            </div>
            <nav>
                <ul>
                    <li><a href="/main.php?menu=1">Map</a></li>
                    <li><a href="/main.php?menu=2">Status</a></li>
                    <li><a href="/main.php?menu=3">Operations</a></li>
                    <li><a href="/hash.php">HASH</a></li>
                    <li class="last"><a href="/">LOGOUT</a></li>
                </ul>
            </nav>
        </header>
    </div>
