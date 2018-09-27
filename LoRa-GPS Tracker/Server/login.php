<?php
if(!isset($_POST['ID'])||!isset($_POST['PW'])){  // check SUBMIT values and set cookies accordingly
    $hashresult = "ac0d7042c3195537fc2fc092f19656747994d596b42c02df3a315f491fb1e9ec8d468a1c004b72a9191f626e3773e7f76fed33bc2f0ab6a1175464a4add2f641";
    if(isset($_COOKIE['LOGINVAL'])){
        $loginval = $_COOKIE['LOGINVAL'];
    }else{
        $loginval = "";
    }
    $userhash = hash("sha512", hash("sha512", $loginval));

    if(!isset($_COOKIE['LOGINVAL']) || $userhash != $hashresult){
        echo"<script>alert('Login Please!');location.replace('/');</script>";
        die;
    }
}else{  // validate login status
    $hashID = "857b47cfadee1b62e6057c23d3cb880e7d5c5c19edcd95c71d3a0b4a0164c21445d3afa8acecc86099d54c9696db0e5a953634b44b1652fbdf5838bff97f3d4b";
    $hashPW = "e91fff92100ab81af765adc0e62f286b313629dc146b659f5db67d208edab8c49b6be2a23bbf884e9ebf44c733368797649b3305482059f8dbd7fb85f622ad5d";
    $hashcookie = "68d609bd44a83f4756bc99a35ecdf4c29d6472a9244b10b4443c621abc2741913b58e511aef9bfb854df3cf40203938b0026ff78ba9caecc0294742dbc48e75c";
    // ID and PW are hashed twice, cookie value will be checked after being hashed twice again.

    $ID=$_POST['ID'];
    $PW=$_POST['PW'];

    $userhashID = hash("sha512", hash("sha512", $ID));
    $userhashPW = hash("sha512", hash("sha512", $PW));

    if($hashID==$userhashID && $hashPW==$userhashPW){
        setcookie("LOGINVAL", $hashcookie);
        echo"<script>location.replace('/main.php');</script>";
    }
    else{
        echo"<script>alert('Incorrect Login!');location.replace('/');</script>";
        die;
    }
}
?>
