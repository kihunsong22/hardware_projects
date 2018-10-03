<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Recaptcha test zone</title>
    <link rel="stylesheet" href="assets/css/captcha_style.css">
    <script src='https://www.google.com/recaptcha/api.js'></script>
</head>

<?php
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(E_ALL);

// Test Key
// $site_Key = "6LeIxAcTAAAAAJcZVRqyHh71UMIEGNQ_MXjiZKhI";
// $secret_Key = "6LeIxAcTAAAAAGG-vFI1TnRWxMZNFuojJ4WifJWe";
$site_Key = "6LfhymQUAAAAAOVG5FQtdIbEcw_ywvv_P841oYMb";
$secret_Key = "Tmt4bWFIbHRVVlZCUVVGQlFWQnFORWRGWm00MGJGRnZaMlkzTmxwRk4xWlRkVlZVT1dWYVZ3PT0=";
$ip = $_SERVER['REMOTE_ADDR'];

$success = "FAIL";

if(isset($_POST['message'])){
    $message = addslashes($_POST['message']);
}

if(isset($_POST['g-recaptcha-response'])){
    $g_recaptcha = addslashes($_POST['g-recaptcha-response']);

	$url = 'https://www.google.com/recaptcha/api/siteverify';
	$data = array(
		'secret' => base64_decode(base64_decode($secret_Key)),
		'response' => $g_recaptcha
	);
	$options = array(
		'http' => array (
            'header' => "Content-Type: application/x-www-form-urlencoded\r\n",
			'method' => 'POST',
			'content' => http_build_query($data)
		)
    );
    
	$context  = stream_context_create($options);
	$response_data = file_get_contents($url, false, $context);
	$response=json_decode($response_data);
	if ($response->success==TRUE) {
        $success = "SUCCESS";
    }
}

?>

<body>
<div class="container">
  <div class="info">
    <h1>Google Recaptcha</h1><span>Test Zone</span>
  </div>
</div>
<div class="form">
  <form class="login-form" action="captcha.php" method="post">

    Site Key
    <input type="text" name="Site Key" value="6LfhymQUAAAAAOVG5FQtdIbEcw_ywvv_P841oYMb" readonly/>
    <br>
    <input type="text" name="message" placeholder="text1"/>
    <input type="text" name="g-recaptcha-response" placeholder="g-recaptcha-response"/>
    <div class="g-recaptcha" data-sitekey="6LfhymQUAAAAAOVG5FQtdIbEcw_ywvv_P841oYMb"></div>
    <br>
    <button>Submit</button>
    </form>
    <form action="#">
        <?php
        echo"<br>RECAPTCHA = $success<br>";
        if(isset($message)){
            echo"message = $message<br>";
        }else{
            echo"<br>NULL<br>";
        }
        if(isset($g_recaptcha)){
            echo"g-recaptcha-response = $g_recaptcha<br>";
        }else{
            echo"<br>NULL<br>";
        }
        ?>
    </form>
</div>
</body>
</html>
