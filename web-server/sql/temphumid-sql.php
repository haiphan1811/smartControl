<?php

include 'sql-function.php';

// Create connection
$conn = ConnectToSql();

// Create a new database
$sql = 'CREATE DATABASE IF NOT EXISTS ' . dbName .'';
if (mysqli_query($conn, $sql))
    echo "Database 'home' created successfully" . PHP_EOL;
else
    echo "Error creating database: " . mysqli_error($conn) . PHP_EOL;

// Select database
$sql = 'USE ' . dbName . '';
if ( !mysqli_query($conn, $sql))
    echo "Error use database: " . mysqli_error($conn) . PHP_EOL;

// sql to create table for store date temp humid
CreateTempHumidTable($conn, "temphumid");

// sql to insert table for store date temp humid
InsertTempHumid($conn,"Temperature","25.3");
InsertTempHumid($conn,"Humidity","68.9");
//DeleteOTempHumid($conn, "Temperature");
//DeleteOTempHumid($conn, "Humidity");

// function create a table if not exist

function CreateTempHumidTable($conn, $table) {

	$tableName = $table;

	$sql = "CREATE TABLE IF NOT EXISTS $tableName (
		id INT(32) UNSIGNED AUTO_INCREMENT PRIMARY KEY, 
		type VARCHAR(11) NOT NULL,
		value VARCHAR(10) NOT NULL DEFAULT 0

	)";
	if (mysqli_query($conn, $sql)) {
	    echo "Table 'temphumid' created successfully\n";
	} else {
	    echo "Error creating table: " . mysqli_error($conn);
	}
}

// function insert object to table
function InsertTempHumid($conn, $Type, $Value) {
	
	$sql = "INSERT INTO temphumid (type, value)
	VALUES ('$Type', '$Value')";
	// echo $sql . PHP_EOL;
	if ($conn->query($sql) === TRUE)
	    echo "Create a new object successfully" . PHP_EOL;
	else
	    echo "Error: " . $sql . " : " . $conn->error . PHP_EOL;
}

function DeleteOTempHumid($conn, $objName) {

	$sql = "DELETE FROM temphumid WHERE type='$objName' ";
	// echo $sql . PHP_EOL;
	if ($conn->query($sql) === TRUE)
	    echo "Record deleted successfully" . PHP_EOL;
	else
	    echo "Error deleting record: " . $conn->error . PHP_EOL;
}
?>