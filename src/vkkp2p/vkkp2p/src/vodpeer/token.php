<?php
	$t = ceil( time() / 300 );
	$ip = $_SERVER["REMOTE_ADDR"]; 

	$token = md5( $ip . $t );

	echo $token;
?>

