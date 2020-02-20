/* study_php_extension extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_study_php_extension.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void study_php_extension_dump( [ mixed $var ] )
 */
PHP_FUNCTION(study_php_extension_dump)
{
	zval *zv_ptr;
	int argc;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_VARIADIC('+', zv_ptr, argc)
	ZEND_PARSE_PARAMETERS_END();

	switch(Z_TYPE_P(zv_ptr))
	{
		case IS_NULL:
			php_printf("NULL: null\n");
			break;
		case IS_TRUE:
			php_printf("BOOL: true\n");
			break;
		case IS_FALSE:
			php_printf("BOOL: false\n");
			break;
		default:
			php_printf("UNKNOWN\n");
			break;
	}
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(study_php_extension)
{
#if defined(ZTS) && defined(COMPILE_DL_STUDY_PHP_EXTENSION)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(study_php_extension)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "study_php_extension support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ study_php_extension_functions[]
 */
static const zend_function_entry study_php_extension_functions[] = {
	PHP_FE(study_php_extension_dump,		NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ study_php_extension_module_entry
 */
zend_module_entry study_php_extension_module_entry = {
	STANDARD_MODULE_HEADER,
	"study_php_extension",					/* Extension name */
	study_php_extension_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(study_php_extension),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(study_php_extension),			/* PHP_MINFO - Module info */
	PHP_STUDY_PHP_EXTENSION_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STUDY_PHP_EXTENSION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(study_php_extension)
#endif
