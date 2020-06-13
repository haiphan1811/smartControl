<?php 
	include '../sql/sql-function.php';
	
	$conn = ConnectDatabse();	// connect to database

	if( $_REQUEST['temphumid']) {
	// Fetch status of device
	FetchTemperatureandHumidity($conn);

	CloseDatabase($conn);
}

function FetchTemperatureandHumidity($conn) {
	$sql = "SELECT *  FROM temphumid";
	$result = $conn->query($sql);
//	if(mysqli_num_rows($result) > 0){
	$data = array(	'temp' =>'100',
			'humid' =>'0');
		while($row = $result->fetch_assoc()) {
			//echo $row["type"];
			if($row["id"] == 1){
					$data['temp'] = $row["value"];
			} 
			else if ($row["id"] == 2){
					$data['humid'] = $row["value"];
	    		}
	    	}
	echo (json_encode($data));
//	}
}
?>