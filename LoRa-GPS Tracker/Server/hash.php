<?php
include_once("./head.php");
$input = "admin";
$hashtye = "md5";
if(isset($_GET['input'])){
    $input = $_GET['input'];
}
if(isset($_GET['hashtype'])){
    $hashtype = strtolower($_GET['hashtype']);
}
else{
    $hashtype = "md5";
}

for($i=1;$i<46;$i++){
    foreach (hash_algos() as $hashlist) {
        $com = strcmp($hashlist, $hashtype);
        if($com==0){
            $hashexists = 1;
            break;
        }
        else{
            $hashexits = 0;
        }
    }
}
if(!$hashexits){
    $hashtype = "md5";
}

$result = 0;

$hash1 = hash($hashtype, $input);
$hash2 = hash($hashtype, $hash1);
$hash3 = hash($hashtype, $hash2);
$hash4 = hash($hashtype, $hash3);
?>
<!-- content -->
<div class="wrapper row2">
    <div id="container" class="clear">
        <!-- content body -->
        <div id="homepage">
            <!-- One Quarter -->
            <h1><?php echo"$hashtype"; ?> HASH</h1>
            <section id="code">
                <form class="" action="/hash.php" method="GET">
                    <input type="text" name="input" placeholder="string to hash"><br>
                    <input type="text" name="hashtype" value="sha512" required><br>
                    <input type="submit" value="HASH">
                </form>
            </section>
            <section id="code">
                <?php
                echo"HASH1: $hash1";
                echo"<br><br>HASH2: $hash2";
                echo"<br><br>HASH3: $hash3";
                echo"<br><br>HASH4: $hash4<br>";
                ?>
            </section>
        </div>
    </div>
</div>
<!-- Footer -->
<?php include("./footer.php"); ?>
