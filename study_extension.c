/*
 * study_extension extension for PHP
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/standard/credits.h"
#include "ext/standard/info.h"
#include "php_study_extension.h"
#include "zend_generators.h"
#include "zend_extensions.h"
#include "zend_constants.h"
#include "ext/standard/html.h"
#include "zend_highlight.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

#define STUDY_COMMON (is_ref ? "&" : "")

PHPAPI void study_extension_var_dump(zval *struc, int level, int escape) /* {{{ */
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
			if (escape) {
				study_php_info_print_html_esc(Z_STRVAL_P(struc), Z_STRLEN_P(struc));
			} else {
				PHPWRITE(Z_STRVAL_P(struc), Z_STRLEN_P(struc));
			}
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
					if (escape) {
						study_php_info_print_html_esc(ZSTR_VAL(key), ZSTR_LEN(key));
					} else {
						PHPWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
					}
					php_printf("\"]=>\n");
				}
				study_extension_var_dump(val, level + 2, escape);
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

						study_extension_var_dump(val, level + 2, escape);
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
		study_extension_var_dump(&zv_ptr[i], 1, 0);
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

			/*
			 * ee97ffd887441557db8aa3cb8d78c02c07274755
			 */
			zend_execute_data *prev_call = skip;
			zend_execute_data *prev = skip->prev_execute_data;

			while (prev) {
				if (prev_call &&
					prev_call->func &&
					!ZEND_USER_CODE(prev_call->func->common.type)) {
					prev = NULL;
					break;
				}
				if (prev->func && ZEND_USER_CODE(prev->func->common.type)) {
					php_printf(" called: %s:%d\n", ZSTR_VAL(prev->func->op_array.filename), prev->opline->lineno);
					break;
				}
				prev_call = prev;
				prev = prev->prev_execute_data;
			}
			if (!prev) {
				PUTS("\n");
			}
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

static ZEND_COLD void study_php_print_gpcse_array(char *name, uint32_t name_length)
{
	zval *data, *tmp;
	zend_ulong num_key;
	zend_string *key, *string_key;

	key = zend_string_init(name, name_length, 0);
	zend_is_auto_global(key);

	if ((data = zend_hash_find(&EG(symbol_table), key)) != NULL && (Z_TYPE_P(data) == IS_ARRAY)) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), num_key, string_key, tmp) {
			if (!sapi_module.phpinfo_as_text) {
				php_printf("<tr><td class=\"e\">$");
				study_php_info_print_html_esc(name, strlen(name));
				php_printf("['");
			} else {
				php_printf("$%s['", name);
			}

			if (string_key != NULL) {
				php_printf("%s", ZSTR_VAL(string_key));
			} else {
				php_printf(ZEND_ULONG_FMT, num_key);
			}

			if (!sapi_module.phpinfo_as_text) {
				php_printf("']</td><td class=\"v\">");
			} else {
				php_printf("'] => ");
			}
			// 面倒くさかったから、var_dump写経のときのやつをdumpする
			study_extension_var_dump(tmp, 1, !sapi_module.phpinfo_as_text);

		} ZEND_HASH_FOREACH_END();
	}

	zend_string_efree(key);
}

static ZEND_COLD int study_php_info_print_html_esc(const char *str, size_t len)
{
	size_t written;
	zend_string *new_str;

	new_str = php_escape_html_entities((unsigned char *) str, len, 0, ENT_QUOTES, "utf-8");
	written = php_output_write(ZSTR_VAL(new_str), ZSTR_LEN(new_str));
	zend_string_free(new_str);
	return written;
}

static ZEND_COLD void php_info_print_stream_hash(const char *name, HashTable *ht)
{
	zend_string *key;

	if (ht) {
		if (zend_hash_num_elements(ht)) {
			int first = 1;

			if (!sapi_module.phpinfo_as_text) {
				php_printf("<tr><td class=\"e\">Registered %s</td><td class=\"v\">", name);
			} else {
				php_printf("\nRegisterd %s => ", name);
			}

			ZEND_HASH_FOREACH_STR_KEY(ht, key) {
				if (key) {
					if (first) {
						first = 0;
					} else {
						php_printf(", ");
					}
					if (!sapi_module.phpinfo_as_text) {
						study_php_info_print_html_esc(ZSTR_VAL(key), ZSTR_LEN(key));
					} else {
						php_printf("%s", ZSTR_VAL(key));
					}
				}
			} ZEND_HASH_FOREACH_END();

			if (!sapi_module.phpinfo_as_text) {
				php_printf("</td></tr>\n");
			}
		} else {
			char reg_name[128];
			snprintf(reg_name, sizeof(reg_name), "Registered %s", name);
			php_info_print_table_row(2, reg_name, "none registered");
		}
	} else {
		php_info_print_table_row(2, name, "disabled");
	}
}

static int module_name_cmp(const void *a, const void *b)
{
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;

	return strcasecmp(((zend_module_entry *)Z_PTR(f->val))->name,
			((zend_module_entry *)Z_PTR(s->val))->name);
}

PHPAPI ZEND_COLD void study_info_print_css(void)
{
	PUTS("body { background-image: url(" TEKITOH_MEMDHOI_INFO_BACKGROUND_IMAGE "); }\n");
	PUTS("table { width: 100%; border: 1px black solid; margin-bottom: 1px; }\n");
	PUTS("td { word-wrap: break-word; }\n");
	PUTS("pre { white-space: pre-wrap; }\n");
	PUTS(".phplogo img { float: right; }\n");
	PUTS(".e { border: 1px black solid; width:300px; }\n");
	PUTS(".v { border: 1px black solid; max-width:300px; }\n");
	PUTS(".h { font-weight: bold; }\n");
	PUTS(".body {width:1122px;margin:0 auto;overflow: hidden;font-family: \"Helvetica Neue\",Helvetica,\"ヒラギノ角ゴ ProN W3\",\"Hiragino Kaku Gothic ProN\",\"メイリオ\",Meiryo,sans-serif;}.bodyside { background-color: #ffffff;}.bodyhead {margin-bottom: 5px;height:40px; background-color: #ffffff;}.bodyheadimg {max-width: 100%; max-height: 100%;margin: 0;}.leftelements > p > a {text-align: center;}\n");
	PUTS(".bodybody{border-image:url(" TEKITOH_MEMDHOI_INFO_BORDER_IMAGE ") 20 stretch; border-style: solid; border-width: 18px; border-color: white;margin-bottom: 10px;font-size: 0.9em; background-color: #ffffff;}}\n");
}

PHPAPI ZEND_COLD void ZEND_COLD study_php_info_print_style(void)
{
	php_printf("<style type=\"text/css\">\n");
	study_info_print_css();
	php_printf("</style>\n");
}

PHPAPI ZEND_COLD void study_php_print_info_htmlhead(void)
{
	php_printf("<!DOCTYPE html>\n");
	php_printf("<html>\n");
	php_printf("<head>\n");
	study_php_info_print_style();
	php_printf("<title>PHP %s - phpinfo()</title>", PHP_VERSION);
	php_printf("<meta name=\"ROBOTS\" content=\"NOINDEX,NOFOLLOW,NOARCHIVE\">");
	php_printf("</head>\n");
	php_printf("<body><div id=\"body\" class=\"body\">\n");
}

PHPAPI ZEND_COLD void study_php_info_print_box_start(int flag)
{
	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		if (flag) {
			php_printf("<tr class=\"h\"><td>\n");
		} else {
			php_printf("<tr class=\"v\"><td>\n");
		}
	} else if (!flag) {
		php_printf("\n");
	}
}

PHPAPI ZEND_COLD void study_php_print_info(int flag)
{
	zend_string *php_uname;
	char **env, *tmp1, *tmp2;

	if (sapi_module.phpinfo_as_text) {
		php_printf("text mode study_extension_phpinfo()\n");
	} else {
		study_php_print_info_htmlhead();
		php_printf("<article class=\"bodybody\">");
	}

	if (flag & PHP_INFO_GENERAL) {
		char *zend_version = get_zend_version();
		char temp_api[10];

		php_uname = php_get_uname('a');

		if (!sapi_module.phpinfo_as_text) {
			study_php_info_print_box_start(1);
		}

		// PHP バージョン
		if (!sapi_module.phpinfo_as_text) {
			time_t the_time;
			struct tm *ta, tmbuf;

			the_time = time(NULL);
			ta = php_localtime_r(&the_time, &tmbuf);

			php_printf("<a class=\"phplogo\" href=\"http://www.php.net/\"><img border=\"0\" src=\"");
			if (ta && (ta->tm_mon == 3) && (ta->tm_mday == 1)) {
				php_printf("%s \" alt=\"PHP logo\"></a>", PHP_EGG_LOGO_DATA_URI);
			} else {
				php_printf("%s \" alt=\"PHP logo\"></a>", PHP_LOGO_DATA_URI);
			}

			php_printf("<header id=\"bodyhead\" class=\"bodyhead\"><h1 class=\"p\">PHP Version %s</h1></header>\n", PHP_VERSION);
		} else {
			php_info_print_table_row(2, "PHP Version", PHP_VERSION);
		}
		php_info_print_box_end();

		php_info_print_table_start();
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

		// Debug buildの有無
		// --enable-debug加えるとyesになる
#ifdef ZEND_DEBUG
		php_info_print_table_row(2, "Debug Build", "yes" );
#else
		php_info_print_table_row(2, "Debug Build", "no" );
#endif

		// スレッドセーフが有効かどうか
		// --enable-maintainer-zts (<=PHP7)
		// --enable-zts (PHP8)
		// もし有効ならば使用しているthreadのライブラリの名前
#ifdef ZTS
		php_info_print_table_row(2, "Thread Safety", "enabled");
		php_info_print_table_row(2, "Thread API", tsrm_api_name());
#else
		php_info_print_table_row(2, "Thread Safety", "disabled");
#endif

		// シグナルハンドラが有効かどうか
#ifdef ZEND_SIGNALS
		php_info_print_table_row(2, "Zend Signal Handling", "enabled");
#else
		php_info_print_table_row(2, "Zend Signal Handling", "disabled");
#endif

		// Zend Memory Manager(ZMM とか ZendMMとか呼ばれる)が有効かどうか
		php_info_print_table_row(2, "Zend Memory Manager", is_zend_mm() ? "enabled" : "disabled");

		// Zend Multibyte Support
		// mbstringのモジュールが出てくる
		{
			const zend_multibyte_functions *functions = zend_multibyte_get_functions();
			char *descr;
			if (functions) {
				spprintf(&descr, 0, "provided by %s", functions->provider_name);
			} else {
				descr = estrdup("disabled");
			}
			php_info_print_table_row(2, "Zend Multibyte Support", descr);
			efree(descr);
		}

		// IPv6をサポートしているか
#if HAVE_IPV6
		php_info_print_table_row(2, "IPv6 Support", "enabled");
#else
		php_info_print_table_row(2, "IPv6 Support", "disabled");
#endif

		// DTrace(ダイナミックトレース)が有効かどうか。
		// 動的にプログラムの実行を追跡したりする
#if HAVE_DTRACE
		php_info_print_table_row(2, "DTrace Support", (zend_dtrace_enabled ? "enabled" : "available, disabled"));
#else
		php_info_print_table_row(2, "DTrace Support", "disabled");
#endif

		// サポートされているPHPのストリーム
		php_info_print_stream_hash("PHP Streams", php_stream_get_url_stream_wrappers_hash());
		// ストリームソケット
		php_info_print_stream_hash("Stream Socket Transports", php_stream_xport_get_hash());
		// ストリームフィルター
		php_info_print_stream_hash("Stream Filters", php_get_stream_filters_hash());

		php_info_print_table_end();

		// Engineの表示
		php_info_print_box_start(0);
		if (sapi_module.phpinfo_as_text) {
			php_printf("Engine:");
			php_printf("\n");
			php_printf("%s", zend_version);
			php_info_print_box_end();
		} else {
			zend_html_puts(zend_version, strlen(zend_version));
		}

		zend_string_free(php_uname);
	}

	// Configurationセクション
	zend_ini_sort_entries();
	if (flag & PHP_INFO_CONFIGURATION) {
		if (sapi_module.phpinfo_as_text) {
			php_printf("\n");
			php_info_print_table_header(1, "Configuration");
		} else {
			php_printf("<hr>\n");
			php_printf("<h2> %s </h2>\n", "Configuration");
		}
		if (!(flag & PHP_INFO_MODULES)) {
			php_printf("\n");
			php_info_print_table_header(1, "PHP Core");
			display_ini_entries(NULL);
		}
	}

	// Modules
	if (flag & PHP_INFO_MODULES) {
		HashTable sorted_registry;
		zend_module_entry *module;

		zend_hash_init(&sorted_registry, zend_hash_num_elements(&module_registry), NULL, NULL, 1);
		zend_hash_copy(&sorted_registry, &module_registry, NULL);
		zend_hash_sort(&sorted_registry, module_name_cmp, 0);

		ZEND_HASH_FOREACH_PTR(&sorted_registry, module) {
			if (module->info_func || module->version) {
				php_info_print_module(module);
			}
		} ZEND_HASH_FOREACH_END();

		php_printf("\n");
		if (!sapi_module.phpinfo_as_text) {
			php_printf("<h2>%s</h2>", "Additional Modules");
		} else {
			php_info_print_table_header(1, "Additional Modules");
		}
		php_info_print_table_start();
		php_info_print_table_header(1, "Module Name");
		ZEND_HASH_FOREACH_PTR(&sorted_registry, module) {
			if (!module->info_func && !module->version) {
				php_info_print_module(module);
			}
		} ZEND_HASH_FOREACH_END();
		php_info_print_table_end();

		zend_hash_destroy(&sorted_registry);
	}

	// 環境変数
	if (flag & PHP_INFO_ENVIRONMENT) {
		php_printf("\n");
		php_printf("Environment");

		php_info_print_table_start();
		php_info_print_table_header(2, "Variable", "Value");

		// ミューテックスで排他ロックする
		tsrm_env_lock();

		// environはmain/php.hにある
		for (env = environ; env != NULL && *env != NULL; env++) {
			tmp1 = estrdup(*env);
			if (!(tmp2 = strchr(tmp1, '='))) {
				efree(tmp1);
				continue;
			}
			*tmp2 = 0;
			tmp2++;
			php_info_print_table_row(2, tmp1, tmp2);
			efree(tmp1);
		}

		tsrm_env_unlock();
		php_info_print_table_end();
	}

	// EGPCS
	if (flag & PHP_INFO_VARIABLES) {
		zval *data;

		if (sapi_module.phpinfo_as_text) {
			php_printf("\n");
			php_info_print_table_header(1, "PHP Variables");
		} else {
			php_printf("<hr>\n");
			php_printf("<h2> %s </h2>\n", "PHP Variables");
		}

		php_info_print_table_start();
		php_info_print_table_header(2, "Variable", "Value");

		if ((data = zend_hash_str_find(&EG(symbol_table), "PHP_SELF", sizeof("PHP_SELF")-1)) != NULL && Z_TYPE_P(data) == IS_STRING) {
			php_info_print_table_row(2, "PHP_SELF", Z_STRVAL_P(data));
		}
		if ((data = zend_hash_str_find(&EG(symbol_table), "PHP_AUTH_TYPE", sizeof("PHP_AUTH_TYPE")-1)) != NULL && Z_TYPE_P(data) == IS_STRING) {
			php_info_print_table_row(2, "PHP_AUTH_TYPE", Z_STRVAL_P(data));
		}
		if ((data = zend_hash_str_find(&EG(symbol_table), "PHP_AUTH_USER", sizeof("PHP_AUTH_USER")-1)) != NULL && Z_TYPE_P(data) == IS_STRING) {
			php_info_print_table_row(2, "PHP_AUTH_USER", Z_STRVAL_P(data));
		}
		if ((data = zend_hash_str_find(&EG(symbol_table), "PHP_AUTH_PW", sizeof("PHP_AUTH_PW")-1)) != NULL && Z_TYPE_P(data) == IS_STRING) {
			php_info_print_table_row(2, "PHP_AUTH_PW", Z_STRVAL_P(data));
		}

		study_php_print_gpcse_array(ZEND_STRL("_REQUEST"));
		study_php_print_gpcse_array(ZEND_STRL("_GET"));
		study_php_print_gpcse_array(ZEND_STRL("_POST"));
		study_php_print_gpcse_array(ZEND_STRL("_FILES"));
		study_php_print_gpcse_array(ZEND_STRL("_COOKIE"));
		study_php_print_gpcse_array(ZEND_STRL("_SERVER"));
		study_php_print_gpcse_array(ZEND_STRL("_ENV"));
		php_info_print_table_end();
	}

	/*
	 * クレジット(本家phpinfo(INFO_CREDITS)
	 * cliだと表示されないんだけどなんで？)
	 *
	 * https://bugs.php.net/bug.php?id=80771
	 */
	if ((flag & PHP_INFO_CREDITS)) {
		php_info_print_hr();
		php_print_credits(PHP_CREDITS_ALL & ~PHP_CREDITS_FULLPAGE);
	}

	// ライセンス
	if (flag & PHP_INFO_LICENSE) {
		if (!sapi_module.phpinfo_as_text) {
			php_printf("<pre>");
		}
		php_printf("\nPHP License\n");
		php_printf("This program is free software; you can redistribute it and/or modify\n");
		php_printf("it under the terms of the PHP License as published by the PHP Group\n");
		php_printf("and included in the distribution in the file:  LICENSE\n");
		php_printf("\n");
		php_printf("This program is distributed in the hope that it will be useful,\n");
		php_printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
		php_printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
		php_printf("\n");
		php_printf("If you did not receive a copy of the PHP license, or have any\n");
		php_printf("questions about PHP licensing, please contact license@php.net.\n");
		if (!sapi_module.phpinfo_as_text) {
			php_printf("</pre>");
		}
	}

	if (!sapi_module.phpinfo_as_text) {
		php_printf("</article></body></html>");
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

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(study_extension)
{
#ifdef CONFIGURE_COMMAND
	REGISTER_STRING_CONSTANT("STUDY_EXTENSION_CONFIGURE_COMMAND", CONFIGURE_COMMAND, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_STRING_CONSTANT("STUDY_EXTENSION_CONFIGURE_COMMAND", "", CONST_CS | CONST_PERSISTENT);
#endif

	return SUCCESS;
}
/* }}} */

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
	// phpinfo関数で表示される関数部分
	php_info_print_table_start();
	php_info_print_table_header(2, "study_extension support", "enabled");
	php_info_print_table_row(2, "Version", PHP_STUDY_EXTENSION_VERSION);
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
	PHP_MINIT(study_extension),			/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(study_extension),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(study_extension),			/* PHP_MINFO - Module info ここをNULLにすると、phpinfoではPHP_STUDY_EXTENSION_VERSIONが表示される */
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
