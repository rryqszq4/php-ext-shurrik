/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_shurrik.h"
#include "shurrik_client.h"
#include <ctype.h>


ShurrikData shurrik_data;

/* If you declare any globals in php_shurrik.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(shurrik)
*/

/* True global resources - no need for thread safety here */
static int le_shurrik;


/* extension redirection functions  */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* shurrik_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*shurrik_old_execute)(zend_op_array *op_array TSRMLS_DC);
void shurrik_execute(zend_op_array *op_array TSRMLS_DC);

void (*shurrik_old_execute_internal)(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC);
void shurrik_execute_internal(zend_execute_data *execute_data TSRMLS_DC);

/* {{{ shurrik_functions[]
 *
 * Every user visible function must have an entry in shurrik_functions[].
 */
zend_function_entry shurrik_functions[] = {
	PHP_FE(confirm_shurrik_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(say,NULL)
	PHP_FE(shurrik_work,NULL)
	PHP_FE(sample_array,NULL)
	PHP_FE(shurrik_get,NULL)
	{NULL, NULL, NULL}	/* Must be the last line in shurrik_functions[] */
};
/* }}} */

/* {{{ shurrik_module_entry
 */
zend_module_entry shurrik_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"shurrik",
	shurrik_functions,
	PHP_MINIT(shurrik),
	PHP_MSHUTDOWN(shurrik),
	PHP_RINIT(shurrik),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(shurrik),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(shurrik),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SHURRIK
ZEND_GET_MODULE(shurrik)
#endif

int shurrik_init(){
	sprintf(shurrik_data.some_data,"==== start ====\n");
	return 1;
}

int shurrik_client(){
	int server_fifo_fd, client_fifo_fd;
	char client_fifo[256];

	server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
	if (server_fifo_fd == -1){
		//fprintf(stderr, "Sorry, no shurrik server\n");
		php_printf("Sorry, no shurrik server\n");
		return 1;
	}
	
	//sprintf(shurrik_data.some_data, "Hello from ");
	strcat(shurrik_data.some_data,"\n==== end ====\n");
	write(server_fifo_fd, &shurrik_data, sizeof(shurrik_data));
	close(server_fifo_fd);
	return 1;
}

char *shurrik_get_opname(zend_uchar opcode){
	switch(opcode) {
		case ZEND_NOP: return "ZEND_NOP"; break;
		case ZEND_ADD: return "ZEND_ADD"; break;
		case ZEND_SUB: return "ZEND_SUB"; break;
		case ZEND_MUL: return "ZEND_MUL"; break;
		case ZEND_DIV: return "ZEND_DIV"; break;
		case ZEND_MOD: return "ZEND_MOD"; break;
		case ZEND_SL: return "ZEND_SL"; break;
		case ZEND_SR: return "ZEND_SR"; break;
		case ZEND_CONCAT: return "ZEND_CONCAT"; break;
		case ZEND_BW_OR: return "ZEND_BW_OR"; break;
		case ZEND_BW_AND: return "ZEND_BW_AND"; break;
		case ZEND_BW_XOR: return "ZEND_BW_XOR"; break;
		case ZEND_BW_NOT: return "ZEND_BW_NOT"; break;
		case ZEND_BOOL_NOT: return "ZEND_BOOL_NOT"; break;
		case ZEND_BOOL_XOR: return "ZEND_BOOL_XOR"; break;
		case ZEND_IS_IDENTICAL: return "ZEND_IS_IDENTICAL"; break;
		case ZEND_IS_NOT_IDENTICAL: return "ZEND_IS_NOT_IDENTICAL"; break;
		case ZEND_IS_EQUAL: return "ZEND_IS_EQUAL"; break;
		case ZEND_IS_NOT_EQUAL: return "ZEND_IS_NOT_EQUAL"; break;
		case ZEND_IS_SMALLER: return "ZEND_IS_SMALLER"; break;
		case ZEND_IS_SMALLER_OR_EQUAL: return "ZEND_IS_SMALLER_OR_EQUAL"; break;
		case ZEND_CAST: return "ZEND_CAST"; break;
		case ZEND_QM_ASSIGN: return "ZEND_QM_ASSIGN"; break;
		case ZEND_ASSIGN_ADD: return "ZEND_ASSIGN_ADD"; break;
		case ZEND_ASSIGN_SUB: return "ZEND_ASSIGN_SUB"; break;
		case ZEND_ASSIGN_MUL: return "ZEND_ASSIGN_MUL"; break;
		case ZEND_ASSIGN_DIV: return "ZEND_ASSIGN_DIV"; break;
		case ZEND_ASSIGN_MOD: return "ZEND_ASSIGN_MOD"; break;
		case ZEND_ASSIGN_SL: return "ZEND_ASSIGN_SL"; break;
		case ZEND_ASSIGN_SR: return "ZEND_ASSIGN_SR"; break;
		case ZEND_ASSIGN_CONCAT: return "ZEND_ASSIGN_CONCAT"; break;
		case ZEND_ASSIGN_BW_OR: return "ZEND_ASSIGN_BW_OR"; break;
		case ZEND_ASSIGN_BW_AND: return "ZEND_ASSIGN_BW_AND"; break;
		case ZEND_ASSIGN_BW_XOR: return "ZEND_ASSIGN_BW_XOR"; break;
		case ZEND_PRE_INC: return "ZEND_PRE_INC"; break;
		case ZEND_PRE_DEC: return "ZEND_PRE_DEC"; break;
		case ZEND_POST_INC: return "ZEND_POST_INC"; break;
		case ZEND_POST_DEC: return "ZEND_POST_DEC"; break;
		case ZEND_ASSIGN: return "ZEND_ASSIGN"; break;
		case ZEND_ASSIGN_REF: return "ZEND_ASSIGN_REF"; break;
		case ZEND_ECHO: return "ZEND_ECHO"; break;
		case ZEND_PRINT: return "ZEND_PRINT"; break;
		case ZEND_JMP: return "ZEND_JMP"; break;
		case ZEND_JMPZ: return "ZEND_JMPZ"; break;
		case ZEND_JMPNZ: return "ZEND_JMPNZ"; break;
		case ZEND_JMPZNZ: return "ZEND_JMPZNZ"; break;
		case ZEND_JMPZ_EX: return "ZEND_JMPZ_EX"; break;
		case ZEND_JMPNZ_EX: return "ZEND_JMPNZ_EX"; break;
		case ZEND_CASE: return "ZEND_CASE"; break;
		case ZEND_SWITCH_FREE: return "ZEND_SWITCH_FREE"; break;
		case ZEND_BRK: return "ZEND_BRK"; break;
		case ZEND_CONT: return "ZEND_CONT"; break;
		case ZEND_BOOL: return "ZEND_BOOL"; break;
		case ZEND_INIT_STRING: return "ZEND_INIT_STRING"; break;
		case ZEND_ADD_CHAR: return "ZEND_ADD_CHAR"; break;
		case ZEND_ADD_STRING: return "ZEND_ADD_STRING"; break;
		case ZEND_ADD_VAR: return "ZEND_ADD_VAR"; break;
		case ZEND_BEGIN_SILENCE: return "ZEND_BEGIN_SILENCE"; break;
		case ZEND_END_SILENCE: return "ZEND_END_SILENCE"; break;
		case ZEND_INIT_FCALL_BY_NAME: return "ZEND_INIT_FCALL_BY_NAME"; break;
		case ZEND_DO_FCALL: return "ZEND_DO_FCALL"; break;
		case ZEND_DO_FCALL_BY_NAME: return "ZEND_DO_FCALL_BY_NAME"; break;
		case ZEND_RETURN: return "ZEND_RETURN"; break;
		case ZEND_RECV: return "ZEND_RECV"; break;
		case ZEND_RECV_INIT: return "ZEND_RECV_INIT"; break;
		case ZEND_SEND_VAL: return "ZEND_SEND_VAL"; break;
		case ZEND_SEND_VAR: return "ZEND_SEND_VAR"; break;
		case ZEND_SEND_REF: return "ZEND_SEND_REF"; break;
		case ZEND_NEW: return "ZEND_NEW"; break;
		case ZEND_FREE: return "ZEND_FREE"; break;
		case ZEND_INIT_ARRAY: return "ZEND_INIT_ARRAY"; break;
		case ZEND_ADD_ARRAY_ELEMENT: return "ZEND_ADD_ARRAY_ELEMENT"; break;
		case ZEND_INCLUDE_OR_EVAL: return "ZEND_INCLUDE_OR_EVAL"; break;
		case ZEND_UNSET_VAR: return "ZEND_UNSET_VAR"; break;
		case ZEND_UNSET_DIM: return "ZEND_UNSET_DIM"; break;
		case ZEND_UNSET_OBJ: return "ZEND_UNSET_OBJ"; break;
		case ZEND_FE_RESET: return "ZEND_FE_RESET"; break;
		case ZEND_FE_FETCH: return "ZEND_FE_FETCH"; break;
		case ZEND_EXIT: return "ZEND_EXIT"; break;
		case ZEND_FETCH_R: return "ZEND_FETCH_R"; break;
		case ZEND_FETCH_DIM_R: return "ZEND_FETCH_DIM_R"; break;
		case ZEND_FETCH_OBJ_R: return "ZEND_FETCH_OBJ_R"; break;
		case ZEND_FETCH_W: return "ZEND_FETCH_W"; break;
		case ZEND_FETCH_DIM_W: return "ZEND_FETCH_DIM_W"; break;
		case ZEND_FETCH_OBJ_W: return "ZEND_FETCH_OBJ_W"; break;
		case ZEND_FETCH_RW: return "ZEND_FETCH_RW"; break;
		case ZEND_FETCH_DIM_RW: return "ZEND_FETCH_DIM_RW"; break;
		case ZEND_FETCH_OBJ_RW: return "ZEND_FETCH_OBJ_RW"; break;
		case ZEND_FETCH_IS: return "ZEND_FETCH_IS"; break;
		case ZEND_FETCH_DIM_IS: return "ZEND_FETCH_DIM_IS"; break;
		case ZEND_FETCH_OBJ_IS: return "ZEND_FETCH_OBJ_IS"; break;
		case ZEND_FETCH_FUNC_ARG: return "ZEND_FETCH_FUNC_ARG"; break;
		case ZEND_FETCH_DIM_FUNC_ARG: return "ZEND_FETCH_DIM_FUNC_ARG"; break;
		case ZEND_FETCH_OBJ_FUNC_ARG: return "ZEND_FETCH_OBJ_FUNC_ARG"; break;
		case ZEND_FETCH_UNSET: return "ZEND_FETCH_UNSET"; break;
		case ZEND_FETCH_DIM_UNSET: return "ZEND_FETCH_DIM_UNSET"; break;
		case ZEND_FETCH_OBJ_UNSET: return "ZEND_FETCH_OBJ_UNSET"; break;
		case ZEND_FETCH_DIM_TMP_VAR: return "ZEND_FETCH_DIM_TMP_VAR"; break;
		case ZEND_FETCH_CONSTANT: return "ZEND_FETCH_CONSTANT"; break;
		case ZEND_EXT_STMT: return "ZEND_EXT_STMT"; break;
		case ZEND_EXT_FCALL_BEGIN: return "ZEND_EXT_FCALL_BEGIN"; break;
		case ZEND_EXT_FCALL_END: return "ZEND_EXT_FCALL_END"; break;
		case ZEND_EXT_NOP: return "ZEND_EXT_NOP"; break;
		case ZEND_TICKS: return "ZEND_TICKS"; break;
		case ZEND_SEND_VAR_NO_REF: return "ZEND_SEND_VAR_NO_REF"; break;
		case ZEND_CATCH: return "ZEND_CATCH"; break;
		case ZEND_THROW: return "ZEND_THROW"; break;
		case ZEND_FETCH_CLASS: return "ZEND_FETCH_CLASS"; break;
		case ZEND_CLONE: return "ZEND_CLONE"; break;
		case ZEND_INIT_METHOD_CALL: return "ZEND_INIT_METHOD_CALL"; break;
		case ZEND_INIT_STATIC_METHOD_CALL: return "ZEND_INIT_STATIC_METHOD_CALL"; break;
		case ZEND_ISSET_ISEMPTY_VAR: return "ZEND_ISSET_ISEMPTY_VAR"; break;
		case ZEND_ISSET_ISEMPTY_DIM_OBJ: return "ZEND_ISSET_ISEMPTY_DIM_OBJ"; break;
		case ZEND_PRE_INC_OBJ: return "ZEND_PRE_INC_OBJ"; break;
		case ZEND_PRE_DEC_OBJ: return "ZEND_PRE_DEC_OBJ"; break;
		case ZEND_POST_INC_OBJ: return "ZEND_POST_INC_OBJ"; break;
		case ZEND_POST_DEC_OBJ: return "ZEND_POST_DEC_OBJ"; break;
		case ZEND_ASSIGN_OBJ: return "ZEND_ASSIGN_OBJ"; break;
		case ZEND_INSTANCEOF: return "ZEND_INSTANCEOF"; break;
		case ZEND_DECLARE_CLASS: return "ZEND_DECLARE_CLASS"; break;
		case ZEND_DECLARE_INHERITED_CLASS: return "ZEND_DECLARE_INHERITED_CLASS"; break;
		case ZEND_DECLARE_FUNCTION: return "ZEND_DECLARE_FUNCTION"; break;
		case ZEND_RAISE_ABSTRACT_ERROR: return "ZEND_RAISE_ABSTRACT_ERROR"; break;
		case ZEND_ADD_INTERFACE: return "ZEND_ADD_INTERFACE"; break;
		case ZEND_VERIFY_ABSTRACT_CLASS: return "ZEND_VERIFY_ABSTRACT_CLASS"; break;
		case ZEND_ASSIGN_DIM: return "ZEND_ASSIGN_DIM"; break;
		case ZEND_ISSET_ISEMPTY_PROP_OBJ: return "ZEND_ISSET_ISEMPTY_PROP_OBJ"; break;
		case ZEND_HANDLE_EXCEPTION: return "ZEND_HANDLE_EXCEPTION"; break;
		case ZEND_USER_OPCODE: return "ZEND_USER_OPCODE"; break;
		default: return "UNKNOWN"; break;
	}
}

//遍历hash表
int shurrik_hash_apply(zval **val, Bucket *bHead){
	Bucket *p;
	Bucket *head;
	zval **tmp;
	char shurrik_tmp[25];

	if(bHead == 0){
		return 1;
	}

	if(bHead == NULL){
	}else {
		p = bHead;
	}
	
	if(p == NULL){
		return 1;
	}

	//php_printf("      ");
	strcat(shurrik_data.some_data, "      ");
	//php_printf("array(\n");
	strcat(shurrik_data.some_data, "array(\n");
	
	while (p != NULL) {
			//php_printf("            ");
			strcat(shurrik_data.some_data, "            ");
			tmp = p->pData;

			if(p->nKeyLength){
				//php_printf("%s=>",p->arKey);
				strcat(shurrik_data.some_data,p->arKey);
				strcat(shurrik_data.some_data,"=>");
			}else {
				//php_printf("%ld=>",p->h);
				sprintf(shurrik_tmp,"%ld",p->h);
				strcat(shurrik_data.some_data,shurrik_tmp);
				strcat(shurrik_data.some_data,"=>");
			}
			if(Z_TYPE_PP(tmp) == IS_STRING){
				//php_printf("%s\n",(**tmp).value.str.val);
				strcat(shurrik_data.some_data,(**tmp).value.str.val);
				strcat(shurrik_data.some_data,"\n");
			}
			if(Z_TYPE_PP(tmp) == IS_ARRAY){
				//php_printf("array()\n");
				//shurrik_apply_array(tmp);
				//php_printf("%p",ht->pListHead);
				//php_printf("%p",(**tmp).value.ht->pListHead->pListNext);
				if(p != (**tmp).value.ht->pListHead){
					shurrik_hash_apply(tmp,(**tmp).value.ht->pListHead);
				}else {
					//php_printf("重复引用\n");
					strcat(shurrik_data.some_data, "重复引用\n");
				}
			}
			p = p->pListNext;
		}
	//php_printf("      );\n");
	strcat(shurrik_data.some_data, "      );\n");
	return 1;

	/*Bucket *p;
	zval **tmp;
	
	if(bHead == NULL){
		if((**val).value.ht->pListHead == 0){
			return 1;
		}
		p = (**val).value.ht->pListHead->pListNext;
		if(p == 0){
			p = *(**val).value.ht->arBuckets;
		}
		if(p == 0){
			return 1;
		}
	}else {
		p = bHead;
	}

	if(p == NULL){
		 return 1;
	}
	php_printf("      ");
	php_printf("array(\n");
	while (p != NULL) {
			php_printf("            ");
			tmp = p->pData;

			if(p->nKeyLength){
				php_printf("%s=>",p->arKey);
			}else {
				php_printf("%ld=>",p->h);
			}
			if(Z_TYPE_PP(tmp) == IS_STRING){
				php_printf("%s\n",(**tmp).value.str.val);
			}
			if(Z_TYPE_PP(tmp) == IS_ARRAY){
				//php_printf("array()\n");
				//shurrik_apply_array(tmp);
				//php_printf("%p",ht->pListHead);
				//php_printf("%p",(**tmp).value.ht->pListHead->pListNext);
				shurrik_hash_apply(tmp,NULL);
			}
			p = p->pListNext;
		}
	php_printf("      );\n");
	return 1;
	*/
}







int shurrik_hash_apply_for_array(zval **val,int num_args,va_list args,zend_hash_key *hash_key)
{
    TSRMLS_FETCH();
	php_printf("      ");
    if (hash_key->nKeyLength)
    {
        //如果是字符串类型的key
        PHPWRITE(hash_key->arKey, hash_key->nKeyLength);
    }	
    else
    {
        //如果是数字类型的key
        php_printf("%ld", hash_key->h);
    }
	
	php_printf("=>");

	if(Z_TYPE_PP(val) == IS_ARRAY){
		//php_printf("%d",(**tmp).type);
		//zend_hash_apply((**val).value.ht,shurrik_hash_apply_for_zval, 0);
		/*if(Z_TYPE_PP(tmp) == IS_STRING){
			php_printf("%s",(**tmp).value.str.val);
		}*/
		Bucket *p;
		zval **tmp;

		p = (**val).value.ht->pListHead;
		if(p == NULL){
			return ZEND_HASH_APPLY_STOP; 
		}
		php_printf("array(\n");
		while (p != NULL) {
			php_printf("            ");
			tmp = p->pData;
			if(p->nKeyLength){
				php_printf("%s=>",p->arKey);
			}else {
				php_printf("%ld=>",p->h);
			}
			if(Z_TYPE_PP(tmp) == IS_STRING){
				php_printf("%s\n",(**tmp).value.str.val);
			}
			if(Z_TYPE_PP(tmp) == IS_ARRAY){
				//php_printf("array()\n");
				//shurrik_apply_array(tmp);
				//zend_hash_apply_with_arguments(((**tmp).value.ht)->pListHead->pData,shurrik_hash_apply_for_array, 0);
				//shurrik_hash_apply(tmp,p);
				//return shurrik_hash_apply_for_zval(tmp TSRMLS_CC);
			}
			p = p->pListNext;
		}
		php_printf("      );\n");
	}
	if(Z_TYPE_PP(val) == IS_STRING){
		php_printf("%s",(**val).value.str.val);
	}
	php_printf("\n");

    //返回，继续遍历下一个～
    return ZEND_HASH_APPLY_KEEP;

}

int shurrik_hash_apply_for_zval(zval **val TSRMLS_DC)
{

    php_printf("      ");
	if(Z_TYPE_PP(val) == IS_ARRAY){
		//php_printf("%d",(**tmp).type);
		//zend_hash_apply((**val).value.ht,shurrik_hash_apply_for_zval, 0);
		/*if(Z_TYPE_PP(tmp) == IS_STRING){
			php_printf("%s",(**tmp).value.str.val);
		}*/
		Bucket *p;
		zval **tmp;

		p = (**val).value.ht->pListHead;
		if(p == NULL){
			return ZEND_HASH_APPLY_STOP; 
		}
		php_printf("array(\n");
		while (p != NULL) {
			php_printf("            ");
			tmp = p->pData;
			if(p->nKeyLength){
				php_printf("%s=>",p->arKey);
			}else {
				php_printf("%ld=>",p->h);
			}
			if(Z_TYPE_PP(tmp) == IS_STRING){
				php_printf("%s\n",(**tmp).value.str.val);
			}
			if(Z_TYPE_PP(tmp) == IS_ARRAY){
				//php_printf("array()\n");
				//shurrik_apply_array(tmp);
				return shurrik_hash_apply_for_zval(tmp TSRMLS_CC);
			}
			p = p->pListNext;
		}
		php_printf("      );\n");
	}
	if(Z_TYPE_PP(val) == IS_STRING){
		php_printf("%s",(**val).value.str.val);
	}
	php_printf("\n");

    //返回，继续遍历下一个～
    return ZEND_HASH_APPLY_KEEP;
}

int shurrik_hash_apply_for_zval_and_key(zval **val,int num_args,va_list args,zend_hash_key *hash_key)
{
	char shurrik_tmp[25];
	TSRMLS_FETCH();
	
	if (hash_key->nKeyLength)
    {
		strcat(shurrik_data.some_data,"$");
		//php_printf("$");
        //如果是字符串类型的key
        //PHPWRITE(hash_key->arKey, hash_key->nKeyLength);
		strcat(shurrik_data.some_data, hash_key->arKey);
		//php_printf(" ");
		//strcat(shurrik_data.some_data, " ");
		if(hash_key->nKeyLength >= 14){
			//php_printf("\t");
			strcat(shurrik_data.some_data, "\t");
		}else if(hash_key->nKeyLength >= 6 && hash_key->nKeyLength < 14){
			//php_printf("\t\t");
			strcat(shurrik_data.some_data, "\t\t");
		}else {
			//php_printf("\t\t\t");
			strcat(shurrik_data.some_data, "\t\t\t");
		}
    }	
    else
    {
        //如果是数字类型的key
        //php_printf("%ld", hash_key->h);
		strcat(shurrik_data.some_data, hash_key->h);
    }
	
	if(Z_TYPE_PP(val) == IS_STRING){
		//php_printf("type:[string]\n");
		strcat(shurrik_data.some_data, "type:[string]\t");
		//php_printf("%s",(**val).value.str.val);
		//php_sprintf(shurrik_tmp,"%s",(**val).value.str.val);
		strcat(shurrik_data.some_data, (**val).value.str.val);
		strcat(shurrik_data.some_data,"\n");
	}
	if(Z_TYPE_PP(val) == IS_LONG){
		//php_printf("type:[integer]\n");
		strcat(shurrik_data.some_data, "type:[integer]\t");
		//php_printf("%ld",(**val).value.lval);
		sprintf(shurrik_tmp,"%ld",(**val).value.lval);
		//printf("%s",shurrik_tmp);
		strcat(shurrik_data.some_data, shurrik_tmp);
		strcat(shurrik_data.some_data,"\n");
	}
	if(Z_TYPE_PP(val) == IS_ARRAY){
		//convert_to_string(&(**val));
		//php_printf((**val).value.str.val);
		//php_printf("type:[array]\n");
		strcat(shurrik_data.some_data, "type:[array]\n");
		//zend_hash_apply((**val).value.ht,shurrik_hash_apply_for_zval, 0);
		//zend_hash_apply_with_arguments((**val).value.ht,shurrik_hash_apply_for_array, 0);
		shurrik_hash_apply(val,(**val).value.ht->pListHead);
	}
	if (Z_TYPE_PP(val) == IS_OBJECT){
		strcat(shurrik_data.some_data, "type:[object]\n");
	}
	if(val == NULL){
		//php_printf("type:[NULL]\n");
		strcat(shurrik_data.some_data, "type:[NULL]\n");
		//php_printf("NULL");
		strcat(shurrik_data.some_data, "NULL");
	}

	//php_printf("\n");
	//strcat(shurrik_data.some_data, "\n");
    return ZEND_HASH_APPLY_KEEP;
}

int shurrik_hash_apply_for_function(HashTable *ht,zend_op_array *op_a){
	Bucket *p;
	zend_function *f;
	zend_op_array op_array;
	zval z;
	int i;

	p = ht->pListHead;

	if (op_a == NULL){
		php_printf("null \n");
	}
	while (p != NULL){
		f = p->pData;
		if (f->type == ZEND_USER_FUNCTION){
			//op_a = EG(active_op_array);
			op_array = f->op_array;
			php_printf("%s[%lx] <==> %s\n",p->arKey,p->h,op_array.filename);
			php_printf("%d\n",op_array.line_start);
			php_printf("%d\n",op_array.line_end);
			php_printf("%d\n",op_array.backpatch_count);
			//z = op_array.opcodes->result.u.constant;
			for (i = 0; i < op_array.last; i++){
				php_printf("%d\n",op_array.opcodes[i].opcode);
			}
			php_printf("%d\n",&(op_array.refcount));
		}
		p = p->pListNext;
	}
	
	return 1;
}

int shurrik_function_test(zend_function *function TSRMLS_DC){
	printf("%d\n",function->type);
	return 0;

}

zend_op_array *shurrik_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC){
	zend_op_array *op_array;
	zval *z;
	int i;
	php_printf("%s\n",file_handle->filename);
	op_array = old_compile_file(file_handle, type TSRMLS_CC);
	//char tmp[16];


	if (op_array){
		php_printf("%s\t%s\t%s\t%s\t\n","id","line","opcode","op1");
		for (i = 0; i < op_array->last; i++){
				php_printf("%d\t",i);
				php_printf("%d\t",op_array->opcodes[i].lineno);
				php_printf("%s\t",shurrik_get_opname(op_array->opcodes[i].opcode));
				//php_printf("%d\t",op_array->opcodes[i].opcode);
				/*if (op_array->opcodes[i].op1.u.constant.type == IS_STRING){
					//php_printf("%d",op_array->opcodes[i].op1.u.constant.value.str.val);
					//php_printf("%s\n",tmp);
					if (op_array->opcodes[i].op1.u.constant.value.str.val){
						sprintf(tmp,"%d",op_array->opcodes[i].op1.u.constant.value.str.val);
						php_printf("%s\n",tmp);
						//php_printf("\"%d\"\n",op_array->opcodes[i].op1.u.constant.value.str.val);
					}else {
						php_printf("...\n");
					}
				}else {
					php_printf("...\n");
				}*/
				z = &op_array->opcodes[i].op1.u.constant;
				if(z->type == IS_STRING){
					//sprintf(tmp,"%d",z->value.str.val);
					//php_printf("<<<%s>>>\n",tmp);
					//php_printf("<<<%p>>>\t",z->value.str.val);
					//php_printf("<<<%d>>>\t",(z->value.str.val > 0xff));
					if (z->value.str.val > 0xff){
						php_printf("\"%s\"\n",z->value.str.val);
					}else {
						php_printf("\"...\"\n");
					}
				}else if (z->type == IS_NULL){
					php_printf("NULL\n");
				}else if (z->type == IS_LONG || z->type == IS_BOOL){
					php_printf("%d\n",z->value.lval);
				}else if (z->type == IS_DOUBLE){
					php_printf("%f\n",z->value.dval);
				}else if (z->type == IS_ARRAY){
					php_printf("Array\n");
				}else if (z->type == IS_OBJECT){
					php_printf("Object\n");
				}else if (z->type == IS_RESOURCE){
					php_printf("Resource\n");
				}else {
					php_printf("unknown\n");
				}
		}
	}

	return op_array;
}

static void shurrik_get_value(){
	TSRMLS_FETCH();
    /*zval **fooval;

    if (zend_hash_find(
            EG(active_symbol_table), //这个参数是地址，如果我们操作全局作用域，则需要&EG(symbol_table)
            "foo",
            sizeof("foo"),
            (void**)&fooval
        ) == SUCCESS
    )
    {
        php_printf("成功发现$foo!");
    }
    else
    {
        php_printf("当前作用域下无法发现$foo.");
    }*/
	//zend_hash_apply_with_arguments(EG(active_symbol_table),shurrik_hash_apply_for_zval_and_key, 0);
	//zend_hash_apply_with_arguments(EG(function_table),shurrik_hash_apply_for_zval_and_key, 0);
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("shurrik.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_shurrik_globals, shurrik_globals)
    STD_PHP_INI_ENTRY("shurrik.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_shurrik_globals, shurrik_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_shurrik_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_shurrik_init_globals(zend_shurrik_globals *shurrik_globals)
{
	shurrik_globals->global_value = 0;
	shurrik_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(shurrik)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	old_compile_file = zend_compile_file;
	zend_compile_file = shurrik_compile_file;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(shurrik)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(shurrik)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(shurrik)
{
	shurrik_init();
	//zend_hash_apply_with_arguments(EG(active_symbol_table),shurrik_hash_apply_for_zval_and_key, 0);
	//zend_hash_apply_with_argument(EG(function_table),shurrik_hash_apply_for_function, 0);
	//shurrik_hash_apply_for_function(EG(function_table),CG(active_op_array));
	//zend_hash_apply(EG(function_table),(apply_func_t) shurrik_function_test TSRMLS_CC);
	shurrik_client();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(shurrik)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "shurrik support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_shurrik_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_shurrik_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "shurrik", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


PHP_FUNCTION(say)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "\nShurrik say : \"%s\"\n", arg);
	RETURN_STRINGL(strg, len, 0);
}

PHP_FUNCTION(shurrik_work)
{
    char *arg = NULL;
	int arg_len, len;
	char *strg;
    int loop_expire = 500;
    int test = 1;
    
    for(;;){
        system("cls");
        shurrik_get_value();
        Sleep(loop_expire);        
    }

	RETURN_STRINGL(strg, len, 0);                    
}

ZEND_FUNCTION(sample_array)
{
    zval *subarray;

    array_init(return_value);

    /* Add some scalars */
    add_assoc_long(return_value, "life", 42);
    add_index_bool(return_value, 123, 1);
    add_next_index_double(return_value, 3.1415926535);

    /* Toss in a static string, dup'd by PHP */
    add_next_index_string(return_value, "Foo", 1);

    /* Now a manually dup'd string */
    add_next_index_string(return_value, estrdup("Bar"), 0);

    /* Create a subarray */
    MAKE_STD_ZVAL(subarray);
    array_init(subarray);

    /* Populate it with some numbers */
    add_next_index_long(subarray, 1);
    add_next_index_long(subarray, 20);
    add_next_index_long(subarray, 300);

    /* Place the subarray in the parent */
    add_index_zval(return_value, 444, subarray);
}

ZEND_FUNCTION(shurrik_get)
{
	/*
	zval *fooval;

    MAKE_STD_ZVAL(fooval);
    ZVAL_STRING(fooval, "bar", 1);
    ZEND_SET_SYMBOL( EG(active_symbol_table) ,  "foo" , fooval);
	*/
	//shurrik_get_value();

	//zend_hash_apply(EG(active_symbol_table),shurrik_hash_apply_for_zval TSRMLS_CC);
	zend_hash_apply_with_arguments(EG(active_symbol_table),shurrik_hash_apply_for_zval_and_key, 0);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
