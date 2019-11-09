/* This is a generated file, edit the .stub.php file instead. */

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_language, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, language, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_internal_encoding, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_http_input, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_http_output arginfo_mb_internal_encoding

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_detect_order, 0, 0, 0)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_substitute_character, 0, 0, 0)
	ZEND_ARG_INFO(0, substchar)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_preferred_mime_name, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_parse_str, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, encoded_string, IS_STRING, 0)
	ZEND_ARG_INFO(1, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_output_handler, 0, 2, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, status, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_str_split, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, split_length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strlen, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strpos, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strrpos, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_stripos arginfo_mb_strpos

#define arginfo_mb_strripos arginfo_mb_strpos

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strstr, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, part, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_strrchr arginfo_mb_strstr

#define arginfo_mb_stristr arginfo_mb_strstr

#define arginfo_mb_strrichr arginfo_mb_strstr

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_substr_count, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_substr, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_strcut arginfo_mb_substr

#define arginfo_mb_strwidth arginfo_mb_strlen

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strimwidth, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, trimmarker, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_convert_encoding, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	ZEND_ARG_INFO(0, from)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_convert_case, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, sourcestring, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_strtoupper, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sourcestring, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_mb_strtolower arginfo_mb_strtoupper

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_detect_encoding, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_INFO(0, encoding_list)
	ZEND_ARG_TYPE_INFO(0, strict, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_list_encodings, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_encoding_aliases arginfo_mb_preferred_mime_name

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_encode_mimeheader, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, charset, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, transfer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, linefeed, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, indent, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_decode_mimeheader, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_convert_kana, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_convert_variables, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(1, var)
	ZEND_ARG_VARIADIC_INFO(1, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_encode_numericentity, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, convmap, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, is_hex, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_decode_numericentity, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, convmap, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_send_mail, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	ZEND_ARG_INFO(0, additional_headers)
	ZEND_ARG_TYPE_INFO(0, additional_parameters, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_get_info arginfo_mb_http_input

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_check_encoding, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_mb_scrub arginfo_mb_strlen

#define arginfo_mb_ord arginfo_mb_strlen

ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_chr, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, cp, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_regex_encoding, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_ereg, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_INFO(1, registers)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
#define arginfo_mb_eregi arginfo_mb_ereg
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_ereg_replace, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, replacement, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
#define arginfo_mb_eregi_replace arginfo_mb_ereg_replace
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_ereg_replace_callback, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_split, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_match, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_ereg_search, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
#define arginfo_mb_ereg_search_pos arginfo_mb_ereg_search
#endif

#if HAVE_MBREGEX
#define arginfo_mb_ereg_search_regs arginfo_mb_ereg_search
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_init, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_INFO_EX(arginfo_mb_ereg_search_getregs, 0, 0, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_getpos, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_setpos, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

#if HAVE_MBREGEX
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_regex_set_options, 0, 0, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_STRING, 0)
ZEND_END_ARG_INFO()
#endif
