<?php
$snowflake = new Snowflake(0);

$result = [];
$time = microtime(true) * 1000;
for ($i = 0; $i < 1000000; $i++) {
    $result[] = $snowflake->generateId();
}
$time2 = microtime(true) * 1000;
echo $time2 - $time;
echo PHP_EOL;

echo count($result) . '-' . count(array_unique($result));
