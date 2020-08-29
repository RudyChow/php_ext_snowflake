/* snowflake extension for PHP */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include <sys/time.h>

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()  \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

zend_class_entry *snowflake_ce;

/* 获取ms */
zend_long getMs()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
}

/* {{{ 类的方法
 */
// 构造方法
PHP_METHOD(Snowflake, __construct)
{
	zend_long epoch = 0, workerId = 0, dataCenterId = 0;

	// 7.0后新提供的方式 FAST-ZPP
	// 第一个参数是开始时间 第二和第三个参数是workerid和datacenterid
	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_LONG(epoch)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(workerId)
		Z_PARAM_LONG(dataCenterId)
	ZEND_PARSE_PARAMETERS_END();

	// 参数判断
	if (epoch < 0 || epoch > 2199023255551)
	{
		zend_throw_exception(NULL, "epoch out of range [0, 2199023255551]", 10086);
	}
	if (workerId < 0 || workerId > 31)
	{
		zend_throw_exception(NULL, "workerId out of range [0, 31]", 10086);
	}
	if (dataCenterId < 0 || dataCenterId > 31)
	{
		zend_throw_exception(NULL, "dataCenterId out of range [0, 31]", 10086);
	}

	// 获取到当前对象
	zval *obj = getThis();

	// 设置对象的值
	zend_update_property_long(snowflake_ce, obj, "epoch", sizeof("epoch") - 1, epoch);
	zend_update_property_long(snowflake_ce, obj, "workerId", sizeof("workerId") - 1, workerId);
	zend_update_property_long(snowflake_ce, obj, "dataCenterId", sizeof("dataCenterId") - 1, dataCenterId);
}

// 生成snowflake的id
PHP_METHOD(Snowflake, generateId)
{
	zval *obj, *epoch, *workerId, *dataCenterId, *seqNum, *lastTime;
	zend_long snowflakeId, seqNumL, lastTimeL, millisecond = 0;
	obj = getThis();
	epoch = zend_read_property(snowflake_ce, obj, "epoch", sizeof("epoch") - 1, 1, NULL);
	workerId = zend_read_property(snowflake_ce, obj, "workerId", sizeof("workerId") - 1, 1, NULL);
	dataCenterId = zend_read_property(snowflake_ce, obj, "dataCenterId", sizeof("dataCenterId") - 1, 1, NULL);
	seqNum = zend_read_property(snowflake_ce, obj, "seqNum", sizeof("seqNum") - 1, 1, NULL);
	lastTime = zend_read_property(snowflake_ce, obj, "lastTime", sizeof("lastTime") - 1, 1, NULL);
	seqNumL = Z_LVAL_P(seqNum);
	lastTimeL = Z_LVAL_P(lastTime);

START_GENERATE:
	// 获取当前ms
	millisecond = getMs();

	// 判断时间回拨
	if (millisecond < lastTimeL)
	{
		zend_throw_exception(NULL, "clock moved backwards", 10087);
	}
	// 时间一致时 判断序列号重复
	if (millisecond == lastTimeL)
	{
		// 如果序列号已经达到最大值 则重新开始获取时间
		if (seqNumL == 0xfff) /* bits: 1111 1111 1111 */
		{
			goto START_GENERATE;
		}
		else
		{
			seqNumL++;
		}
	}
	else
	{
		seqNumL = 0;
	}

	// 生成id
	snowflakeId = snowflakeId | (millisecond - Z_LVAL_P(epoch)) << 22 | Z_LVAL_P(workerId) << 17 | Z_LVAL_P(dataCenterId) << 12 | seqNumL;

	// 记录序列号 和上次获取的时间
	zend_update_property_long(snowflake_ce, obj, "seqNum", sizeof("seqNum") - 1, seqNumL);
	zend_update_property_long(snowflake_ce, obj, "lastTime", sizeof("lastTime") - 1, millisecond);

	RETURN_LONG(snowflakeId)
}
/* }}} */

// 注册方法
const zend_function_entry snowflake_functions[] = {
	PHP_ME(Snowflake, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(Snowflake, generateId, NULL, ZEND_ACC_PUBLIC)
PHP_FE_END};

// 注册类
PHP_MINIT_FUNCTION(snowflake)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Snowflake", snowflake_functions);

	snowflake_ce = zend_register_internal_class(&ce);

	// 类的变量
	zend_declare_property_long(snowflake_ce, "epoch", sizeof("epoch") - 1, 0, ZEND_ACC_PRIVATE);
	zend_declare_property_long(snowflake_ce, "workerId", sizeof("workerId") - 1, 0, ZEND_ACC_PRIVATE);
	zend_declare_property_long(snowflake_ce, "dataCenterId", sizeof("dataCenterId") - 1, 0, ZEND_ACC_PRIVATE);
	zend_declare_property_long(snowflake_ce, "seqNum", sizeof("seqNum") - 1, 0, ZEND_ACC_PRIVATE);
	zend_declare_property_long(snowflake_ce, "lastTime", sizeof("lastTime") - 1, 0, ZEND_ACC_PRIVATE);

	return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(snowflake)
{
#if defined(ZTS) && defined(COMPILE_DL_SNOWFLAKE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(snowflake)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "snowflake support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ snowflake_module_entry
 */
zend_module_entry snowflake_module_entry = {
	STANDARD_MODULE_HEADER,
	"snowflake",		   /* Extension name */
	NULL,				   /* zend_function_entry */
	PHP_MINIT(snowflake),  /* PHP_MINIT - Module initialization */
	NULL,				   /* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(snowflake),  /* PHP_RINIT - Request initialization */
	NULL,				   /* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(snowflake),  /* PHP_MINFO - Module info */
	PHP_SNOWFLAKE_VERSION, /* Version */
	STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_SNOWFLAKE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(snowflake)
#endif
