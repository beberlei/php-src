--TEST--
Bug #55005 (imagepolygon num_points requirement)
--SKIPIF--
<?php
if (!extension_loaded('gd')) die('skip gd extension not available');
?>
--FILE--
<?php
require_once __DIR__ . '/func.inc';

$g = imagecreate(300, 300);
$bgnd = imagecolorallocate($g, 255, 255, 255);
$fgnd = imagecolorallocate($g, 0, 0, 0);
trycatch_dump(
    fn () => imagefilledpolygon($g, array(100,10, 100,100, 180,100), 2, $fgnd),
    fn () => imagepolygon($g, array(200,10, 200,100, 280,100), 2, $fgnd)
);
?>
--EXPECT--
!! [ValueError] Polygon must have at least 3 points
!! [ValueError] Polygon must have at least 3 points
