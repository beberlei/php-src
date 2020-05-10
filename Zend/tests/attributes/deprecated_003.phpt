--TEST--
<<Deprecated>> attribute compile errors
--FILE--
<?php

<<Deprecated(1, 2, 3)>>
function test() {
}
--EXPECTF--
Fatal error: <<Deprecated>> requires at most one argument, 3 arguments given in %s
