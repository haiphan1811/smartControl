<?php

$user = $_POST['u'];
$pass = $_POST['p'];
$Connection = 0;

$ip = $_SERVER['REMOTE_ADDR'];
if ( ($user == 'haiphan') && ($pass == '181193') || ($ip == '192.168.1.1') || (($user == 'FbCrest') && ($pass == '123456')))
{
    $Connection = 1;
    include 'action_page.php';
}
else
{
//echo "YOUR ROUTER IP ADDRESS .$ip.";
     header("Location: relogin.html");
}

?>