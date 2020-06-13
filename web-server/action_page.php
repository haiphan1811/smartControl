<!-- 	Nguyen Hai Duong, September 2016 
 			GNU LESSER GENERAL PUBLIC LICENSE Version 2.1, February 1999
-->

<?php 

include 'function/print-HTML.php';
include 'sql/sql-function.php';

$IDConnection = 0;
$IDConnection = $Connection;

if ($IDConnection == 0)
{
	echo 'DONT GO TO MY WEB LIKE THIS WAY';
}
if ($IDConnection != 1) return;

$conn = ConnectDatabse();

?>

<!DOCTYPE html>
<html lang="en">
  <head>

    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->
    <title>Smart Control - Phan Thanh Hai</title>
    
    <!-- CSS -->
    <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Open+Sans">
    <link rel="stylesheet" type="text/css" href="fonts/font-awesome/css/font-awesome.min.css">
    <link rel="stylesheet" type="text/css" href="css/index.css">
    <!-- Bootstrap -->
    <link rel="stylesheet" type="text/css" href="css/bootstrap.min.css">
    <link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css">
 
    
    <!-- JS -->
    <script type="text/javascript" src="js/jquery/jquery.js"></script>
    <script type="text/javascript" src="js/jquery/jquery-ui.min.js"></script>
    <script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
    <script type="text/javascript" src="js/query.js"></script>
    
    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/html5shiv/3.7.3/html5shiv.min.js"></script>
      <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]-->
  </head>
  <body class="smartcontrol">
    <!-- Fixed navbar -->
    <div class="container"></div>
    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container">
        <div class="navbar-header">
	   <h3 style="color:white;text-align:center;padding-top: 10px;">Smart Control - Phan Thanh Hai</h3>
        </div>
      </div>
    </nav>
    <!-- Conainer -->
    <div class="container">

      <div class="row">
        <div class="land-1">

	<div class="col-lg-12 col-md-12 col-sm-12 col-xs-112">
        <div class="object-Temp">
            <div class="ojb-temperature">Temperature</div> 
            <div class="temperature-icon-box"><img src="image/temperature-icon-c.png" class="temperature-icon" alt="Temperature"></div>
            <div class="temperature-unit"></div>       
            <div id= "temp-output" class="temperature-showvalue"></div>
            <div class="ojb-humidity">Humidity</div> 
            <div class="humidity-icon-box"><img src="image/humidity-icon.png" class="humidity-icon" alt="Humidity"></div>
	    <div id= "humid-output" class="humidity-showvalue"></div>      
	    <div class="temperature-unit"></div> 
</div>
      </div>
</br>
<?php

	PrintObjectDatabase($conn);
	//PrintObjectSend();

?>
          <div class="clearfix"></div>  
        </div>
      </div>
    </div>
	<div class="log-box alert alert-danger" role="alert">
			<strong>Woop !</strong>
			<p class="log-text">test demo alert log</p>
	</div>
	<div id="result">
	
	</div>
  </body>

</html>

<?php 
	CloseDatabase($conn);
?>
