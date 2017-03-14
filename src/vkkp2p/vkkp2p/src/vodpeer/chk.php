<?php
	$t = ceil( time() / 300 );
	$ip = $_SERVER["REMOTE_ADDR"]; 

	$token = md5( $ip . $t );


	$t = $_REQUEST['t'];
	$r = $_REQUEST['r'];
	$c = $_REQUEST['c'];
	$m = $_REQUEST['m'];

	$c2 = md5( $token . "," . $r );

	//echo "$c\n$c2\n";

	$key = "59369668+nioFImUQLI3NTIwMjU3Mg==";
	if ( $t == "linux" ) {
	$key = "59369668+nioFImUQLI3NTIwMjU3Mg==";
	}
	else if ( $t == "windows" ) {
	$key = "184589045vaMDyAnCgsyNzY4MjAzMA==";
	}
	else if ( $t == "android" ) {
	$key = "542983943kv5qruLtkYyNTk1MTE2Nw==";
	}

	if ( $c == $c2 ) {
		$data = $t . "," . $r . "," . $key ;
		$m2 = md5( $data );

		echo $m2;
	}

?>

