/* study_php_extension extension for PHP */

#ifndef PHP_STUDY_PHP_EXTENSION_H
# define PHP_STUDY_PHP_EXTENSION_H

extern zend_module_entry study_php_extension_module_entry;
# define phpext_study_php_extension_ptr &study_php_extension_module_entry

# define PHP_STUDY_PHP_EXTENSION_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_STUDY_PHP_EXTENSION)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_STUDY_PHP_EXTENSION_H */
