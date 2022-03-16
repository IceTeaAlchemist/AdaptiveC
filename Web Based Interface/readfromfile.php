<?php
			$mode =$_POST['mode'];
			$output=null;
			$retval = null;
			exec('/var/www/html/readtographcore',$output,$retval);
		  echo json_encode($output);
?>
