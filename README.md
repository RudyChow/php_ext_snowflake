a php extension based on snowflake

PHP VERSION >= 7.3.0

##  How to install

```bash
$ phpize
$ ./configure
$ make
$ make install
```

php.ini
```ini
extension=snowflake.so
```

##  How to use

the extension class such as:
```php
class Snowflake
{
    private $epoch;
    private $workerId;
    private $dataCenterId;
    private $seqNum;
    private $lastTime;

    /**
     * @param integer $epoch
     * @param integer $workerId
     * @param integer $dataCenterId
     * 
     * @exception params out of range
     */
    public function __construct(int $epoch, int $workerId = 0, int $dataCenterId = 0)
    {
    }

    /**
     * @return integer
     * 
     * @exception clock moved backwards
     */
    public function generateId()
    {
    }
}
```

example:

```php
$epoch = 0;
$snowflake = new Snowflake($epoch);
$snowflakeId = $snowflake->generateId();

```