<?php
$snowflake = new Snowflake(0, 2, 1);

$result = [];
$time = microtime(true) * 1000;
for ($i = 0; $i < 5000; $i++) {
    $result[] = $snowflake->generateId();
}
$time2 = microtime(true) * 1000;
echo $time2 - $time;
echo PHP_EOL;

echo count($result) . '-' . count(array_unique($result));

$new = [];
foreach ($result as $value) {
    isset($new[$value]) ? $new[$value]++ : $new[$value] = 1;
}
print_r(array_filter($new, function ($v) {
    return $v > 1;
}));
