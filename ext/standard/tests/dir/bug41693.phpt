--TEST--
Bug #41693 (scandir() allows empty directory names)
--FILE--
<?php

try {
    var_dump(scandir(''));
} catch (\ValueError $e) {
    echo $e->getMessage() . "\n";
}

?>
--EXPECT--
Directory name cannot be empty
