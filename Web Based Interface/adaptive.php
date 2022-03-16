<?php
			$mode =$_POST['mode'];
			$output=null;
			$retval = null;
			exec('/var/www/html/adaptivephptry',$output,$retval);
                  file_put_contents('file.txt', $output);
		  echo $retval;
?>
