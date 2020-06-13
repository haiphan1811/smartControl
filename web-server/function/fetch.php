<?php 
	include '../sql/sql-function.php';
	
	$conn = ConnectDatabse();	// connect to database

	if( $_REQUEST['fetch']) {
	// Fetch status of device
	FetchObjectStatus($conn);

	CloseDatabase($conn);
}

function FetchObjectStatus($conn) {
	$sql = "SELECT *  FROM device";
	$result = $conn->query($sql);
//	if(mysqli_num_rows($result) > 0){
	$data = array(	'lightstate' =>'progress-bar-off',
			'fanstate' =>'progress-bar-off');
		while($row = $result->fetch_assoc()) {
			//first device for light
			if($row["name"] == 'Room Light'){
				if($row["state"] == 1){
					$data['lightstate'] = 'progress-bar-on';
				}
				else{
					$data['lightstate'] = 'progress-bar-off';
				}
			} 
			else if ($row["name"] == 'Fan'){
				if($row["state"] == 1){
					$data['fanstate'] = 'progress-bar-on';
				}
				else{
					$data['fanstate'] = 'progress-bar-off';
			 	}
			
	    		}
	    	}
	echo (json_encode($data));
//	}
}
?>