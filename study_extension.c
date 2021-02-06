/*
 * study_extension extension for PHP
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#include "php_study_extension.h"
#include "zend_generators.h"
#include "zend_extensions.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

#define STUDY_COMMON (is_ref ? "&" : "")

PHPAPI void study_extension_var_dump(zval *struc, int level) /* {{{ */
{
	uint32_t count;
	HashTable *myht;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zend_string *class_name;
	int is_ref = 0;

	if (level > 1) {
		php_printf("%*c", level - 1, ' ');
	}

again:
	switch(Z_TYPE_P(struc))
	{
		case IS_NULL:
			php_printf("%sNULL: null\n", STUDY_COMMON);
			break;
		case IS_TRUE:
			php_printf("%sBOOL: true\n", STUDY_COMMON);
			break;
		case IS_FALSE:
			php_printf("%sBOOL: false\n", STUDY_COMMON);
			break;
		case IS_LONG:
			php_printf("%sLONG: " ZEND_LONG_FMT "\n", STUDY_COMMON, Z_LVAL_P(struc));
			break;
		case IS_DOUBLE:
			php_printf("%sDOUBLE: %.*G\n", STUDY_COMMON, (int) EG(precision), Z_DVAL_P(struc));
			break;
		case IS_STRING:
			php_printf("%sSTRING: value=\"", STUDY_COMMON);
			PHPWRITE(Z_STRVAL_P(struc), Z_STRLEN_P(struc));
			php_printf("\", length=%zd\n", Z_STRLEN_P(struc));
			break;
		case IS_RESOURCE: {
			const char *type_name = zend_rsrc_list_get_rsrc_type(Z_RES_P(struc));
			php_printf("%sRESOURCE: id=%d type=%s\n", STUDY_COMMON, Z_RES_P(struc)->handle, type_name ? type_name : "Unknown");
			break;
		}
		case IS_ARRAY:
			myht = Z_ARRVAL_P(struc);

			/* recursion protection */
			if (level > 1 && !(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				if (GC_IS_RECURSIVE(myht)) {
					PUTS("*RECURSION*\n");
					return;
				}
				GC_PROTECT_RECURSION(myht);
			}
			count = zend_array_count(myht);
			php_printf("%sARRAY(%d) {\n", STUDY_COMMON, count);

			ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
				if (key == NULL) { /* numeric key */
					php_printf("%*c[" ZEND_LONG_FMT "]=>\n", level + 1, ' ', num);
				} else { /* string key */
					php_printf("%*c[\"", level + 1, ' ');
					PHPWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
					php_printf("\"]=>\n");
				}
				study_extension_var_dump(val, level + 2);
			} ZEND_HASH_FOREACH_END();

			if (level > 1 && !(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				GC_UNPROTECT_RECURSION(myht);
			}

			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			break;
		case IS_OBJECT:
			/* recursion protection */
			if (Z_IS_RECURSIVE_P(struc)) {
				PUTS("*RECURSION*\n");
				return;
			}
			Z_PROTECT_RECURSION_P(struc);

			myht = zend_get_properties_for(struc, ZEND_PROP_PURPOSE_DEBUG);
			class_name = Z_OBJ_HANDLER_P(struc, get_class_name)(Z_OBJ_P(struc));

			php_printf("%sOBJECT(%s)#%d (%d) {\n", STUDY_COMMON, ZSTR_VAL(class_name), Z_OBJ_HANDLE_P(struc), myht ? zend_array_count(myht) : 0);

			if (myht) {
				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
					zend_property_info *prop_info = NULL;

					if (Z_TYPE_P(val) == IS_INDIRECT) {
						val = Z_INDIRECT_P(val);
						if (key) {
							prop_info = zend_get_typed_property_info_for_slot(Z_OBJ_P(struc), val);
						}
					}

					if (!Z_ISUNDEF_P(val) || prop_info) { /* TODO: Is need Z_ISUNDEF_P? */
						const char *object_prop_name, *object_class_name;

						if (key == NULL) { /* numeric key */
							php_printf("%*c[" ZEND_LONG_FMT "]=>\n", level + 1, ' ', num);
						} else { /* string key */
							int unmangle = zend_unmangle_property_name(key, &object_class_name, &object_prop_name);
							php_printf("%*c[", level + 1, ' ');

							if (object_class_name && unmangle == SUCCESS) {
								if (object_class_name[0] == '*') {
									php_printf("\"%s\":protected", object_prop_name);
								} else {
									php_printf("\"%s\":\"%s\":private", object_prop_name, object_class_name);
								}
							} else {
								php_printf("\"");
								PHPWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
								php_printf("\"");
							}
							PUTS("]=>\n");
						}

						study_extension_var_dump(val, level + 2);
					}
				} ZEND_HASH_FOREACH_END();
				zend_release_properties(myht); /* must: Release myht */
			}

			zend_string_release_ex(class_name, 0);
			if (level > 1) {
				php_printf("%*c", level - 1, ' ');
			}
			PUTS("}\n");
			Z_UNPROTECT_RECURSION_P(struc); /* unprotect recursion */
			break;
		case IS_REFERENCE:
			is_ref = 1;
			struc = Z_REFVAL_P(struc);
			goto again;
			break;
		default:
			php_printf("UNKNOWN\n");
			break;
	}
}
/* }}} */

/* {{{ void study_extension_dump( mixed $var )
 */
PHP_FUNCTION(study_extension_dump)
{
	zval *zv_ptr;
	int argc, i;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_VARIADIC('+', zv_ptr, argc)
	ZEND_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		study_extension_var_dump(&zv_ptr[i], 1);
	}
}

static inline zend_bool study_skip_internal_handler(zend_execute_data *skip)
{
	return !(skip->func && ZEND_USER_CODE(skip->func->common.type))
		&& skip->prev_execute_data
		&& skip->prev_execute_data->func
		&& ZEND_USER_CODE(skip->prev_execute_data->func->common.type)
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_FCALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_ICALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_UCALL
		&& skip->prev_execute_data->opline->opcode != ZEND_DO_FCALL_BY_NAME
		&& skip->prev_execute_data->opline->opcode != ZEND_INCLUDE_OR_EVAL;
}

/* {{{ void study_extension_print_backtrace( [ int options ] )
 */
PHP_FUNCTION(study_extension_print_backtrace)
{
	zend_execute_data *ptr, *skip, *call;
	zend_function *func;
	zend_long options = 0;
	zend_long limit = 0;
	zval arg_array;

	zend_object *object;

	int lineno;
	int frameno = 0;

	char *call_type;
	const char *filename;
	const char *function_name;
	zend_string *class_name = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &options, &limit) == FAILURE) {
		return;
	}

	ZVAL_UNDEF(&arg_array);
	ptr = EX(prev_execute_data);

	call = ptr;
	ptr = ptr->prev_execute_data;

	while (ptr && (limit == 0 || frameno < limit)) {
		frameno++;
		class_name = NULL;
		call_type = NULL;

		ZVAL_UNDEF(&arg_array);
		ptr = zend_generator_check_placeholder_frame(ptr);
		skip = ptr;

		if (study_skip_internal_handler(skip)) {
			skip = skip->prev_execute_data;
		}

		if (skip->func && ZEND_USER_CODE(skip->func->common.type)) {
			filename = ZSTR_VAL(skip->func->op_array.filename);
			if (skip->opline->opcode == ZEND_HANDLE_EXCEPTION) {
				if (EG(opline_before_exception)) {
					lineno = EG(opline_before_exception)->lineno;
				} else {
					lineno = skip->func->op_array.line_end;
				}
			} else {
				lineno = skip->opline->lineno;
			}
		} else {
			filename = NULL;
			lineno = 0;
		}

		object = (Z_TYPE(call->This) == IS_OBJECT) ? Z_OBJ(call->This) : NULL;

		if (call->func) {
			zend_string *zend_function_name;
			func = call->func;

			if (func->common.scope && func->common.scope->trait_aliases) {
				zend_function_name = zend_resolve_method_name(object ? object->ce : func->common.scope, func);
			} else {
				zend_function_name = func->common.function_name;
			}

			if (zend_function_name != NULL) {
				function_name = ZSTR_VAL(zend_function_name);
			} else {
				function_name = NULL;
			}
		} else {
			func = NULL;
			function_name = NULL;
		}

		if (function_name) {
			if (object) {
				if (func->common.scope) {
					class_name = func->common.scope->name;
				} else if (object->handlers->get_class_name == zend_std_get_class_name) {
					class_name = object->ce->name;
				} else {
					class_name = object->handlers->get_class_name(object);
				}

				call_type = "->";
			} else if (func->common.scope) {
				class_name = func->common.scope->name;
				call_type = "::";
			} else {
				class_name = NULL;
				call_type = NULL;
			}
		} else {
			if (!ptr->func || !ZEND_USER_CODE(ptr->func->common.type) || ptr->opline->opcode != ZEND_INCLUDE_OR_EVAL) {
				function_name = "unknown";
			} else {
				switch (ptr->opline->extended_value) {
					case ZEND_EVAL:
						function_name = "eval";
						break;
					case ZEND_INCLUDE:
						function_name = "include";
						break;
					case ZEND_REQUIRE:
						function_name = "require";
						break;
					default:
						function_name = "unknown";
						break;
				}
			}
			call_type = NULL;
		}

		php_printf("function: ");
		if (class_name) {
			PUTS(ZSTR_VAL(class_name));
			PUTS(call_type);
		}
		php_printf("%s\t", function_name);
		if (filename) {
			php_printf("filename: %s:%d\n", filename, lineno);
		} else {
			// ex: array_walk, preg_replace etc
			PUTS("\n");
		}

		call = skip;
		ptr = skip->prev_execute_data;
	}

}

PHP_FUNCTION(study_extension_nop)
{
}

PHPAPI extern char *php_ini_opened_path;
PHPAPI extern char *php_ini_scanned_path;

static zend_always_inline int php_info_print(const char *str)
{
	return php_output_write(str, strlen(str));
}

PHPAPI ZEND_COLD void study_php_print_info(int flag)
{
	zend_string *php_uname;

	if (sapi_module.phpinfo_as_text) {
		php_info_print("text mode study_extension_phpinfo()\n");
	} else {
		php_info_print("html\n");
	}

	if (flag & PHP_INFO_GENERAL) {
		char *zend_version = get_zend_version();
		char temp_api[10];

		php_uname = php_get_uname('a');

		// PHP バージョン
		php_info_print_table_row(2, "PHP Version", PHP_VERSION);
		// uname -aの結果(Linux)
		// GetVersion関数などの結果(Windows)
		php_info_print_table_row(2, "System", ZSTR_VAL(php_uname));
		// コンパイルした日付
		php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__);
#ifdef COMPILER
		// Windowsで表示される(Windowsでしか設定していない)
		// Visual C++ 2017 など
		php_info_print_table_row(2, "Compiler", COMPILER);
#endif
#ifdef ARCHITECTURE
		// Windowsで表示される(Windowsでしか設定していない)
		// 32bit: x86
		// 64bit: x64
		php_info_print_table_row(2, "Architecture", ARCHITECTURE);
#endif
#ifdef CONFIGURE_COMMAND
		// configure時のコマンド
		php_info_print_table_row(2, "Configure Command", CONFIGURE_COMMAND);
#endif

		// sapi_module.name php_sapi_name()の戻り値と同じ
		if (sapi_module.name) {
			php_info_print_table_row(2, "Server API (sapi_module.name)", sapi_module.name);
		}

		// sapi_module.pretty_nameはsapi_module.nameよりもわかりやすい名前で表示する
		if (sapi_module.pretty_name) {
			php_info_print_table_row(2, "Server API", sapi_module.pretty_name);
		}

		// virtual_getcwdを使ってpathを取得するときなどに使う。
		// 何故ならばスレッドが実行するプログラムはそれ自身を起点としないと
		// 例えばinclude "file.php";などを見つけることができないため。
		// マルチスレッドを有効にするオプションである
		// --enable-maintainer-ztsをconfigureに加えるとここもenabledとなる。
#ifdef VIRTUAL_DIR
		php_info_print_table_row(2, "Virtual Directory Support", "enabled");
#else
		php_info_print_table_row(2, "Virtual Directory Support", "disabled");
#endif

		// php.iniのファイルパス
		// --with-config-file-path=PATH で変更できる
		php_info_print_table_row(2, "Configuration File (php.ini) Path", PHP_CONFIG_FILE_PATH);
		// 読み込んだphp.iniファイルのpath
		php_info_print_table_row(2, "Loaded Configuration File", php_ini_opened_path ? php_ini_opened_path : "(none)");

		// php.iniを追加で読み込める。これは環境変数 PHP_INI_SCAN_DIR を使う。
		php_info_print_table_row(2, "Scan this dir for additional .ini files", php_ini_scanned_path ? php_ini_scanned_path : "(none)");

		snprintf(temp_api, sizeof(temp_api), "%d", PHP_API_VERSION);
		// PHP APIのバージョン
		// phpizeで使う
		php_info_print_table_row(2, "PHP API", temp_api);

		// Zend ExtensionのZEND_MODULE_API_NO
		snprintf(temp_api, sizeof(temp_api), "%d", ZEND_MODULE_API_NO);
		php_info_print_table_row(2, "PHP Extension", temp_api);

		// Zend ExtensionのZEND_EXTENSION_API_NO
		snprintf(temp_api, sizeof(temp_api), "%d", ZEND_EXTENSION_API_NO);
		php_info_print_table_row(2, "Zend Extension", temp_api);

		php_info_print_table_row(2, "Zend Extension Build", ZEND_MODULE_BUILD_ID);
		php_info_print_table_row(2, "PHP Extension Build", ZEND_MODULE_BUILD_ID);

		zend_string_free(php_uname);
	}
}

PHP_FUNCTION(study_extension_phpinfo)
{
	zend_long flag = PHP_INFO_ALL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flag)
	ZEND_PARSE_PARAMETERS_END();

	/* 2260e1742d671a2b57b0d086faf87393a0460bef */
	/* Andale! Andale! Yee-Hah! (早く！早く！やっほー）*/
	php_output_start_default();
	study_php_print_info(flag);
	php_output_end();
	RETURN_TRUE;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(study_extension)
{
#if defined(ZTS) && defined(COMPILE_DL_STUDY_EXTENSION)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(study_extension)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "study_extension support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_study_extension_dump, 0)
	ZEND_ARG_VARIADIC_INFO(0, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_extension_print_backtrace, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_extension_phpinfo, 0, 0, 0)
	ZEND_ARG_VARIADIC_INFO(0, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_study_extension_nop, 0)
ZEND_END_ARG_INFO()

/* }}} */


/* {{{ study_extension_functions[]
 */
static const zend_function_entry study_extension_functions[] = {
	PHP_FE(study_extension_dump,		arginfo_study_extension_dump)
	PHP_FE(study_extension_print_backtrace,		arginfo_study_extension_print_backtrace)
	PHP_FE(study_extension_phpinfo,		arginfo_study_extension_phpinfo)
	PHP_FE(study_extension_nop,		arginfo_study_extension_nop)
	PHP_FE_END
};
/* }}} */

/* {{{ study_extension_module_entry
 */
zend_module_entry study_extension_module_entry = {
	STANDARD_MODULE_HEADER,
	"study_extension",					/* Extension name */
	study_extension_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(study_extension),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(study_extension),			/* PHP_MINFO - Module info */
	PHP_STUDY_EXTENSION_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STUDY_EXTENSION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(study_extension)
#endif

/*
 * vim: noexpandtab tabstop=4 shiftwidth=4
 */
