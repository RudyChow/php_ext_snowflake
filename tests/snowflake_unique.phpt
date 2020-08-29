--TEST--
Check if snowflake id is unique
--FILE--
<?php
$snowflake = new Snowflake(0);

$result = [];
for ($i = 0; $i < 500000; $i++) {
    $result[] = $snowflake->generateId();
}
echo count(array_unique($result));

?>
--EXPECT--
500001
