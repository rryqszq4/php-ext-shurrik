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
#include "shurrik_oparray.h"
#include <ctype.h>

#define EXEC_BUFFER_SIZE 1024*256

ShurrikData shurrik_data;
char  exec_user_data[EXEC_BUFFER_SIZE - 1];
char  exec_internal_data[EXEC_BUFFER_SIZE - 1];
char shurrik_exec_tmp[256];
char shurrik_user_tmp[256];

/* If you declare any globals in php_shurrik.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(shurrik)
*/

/* True global resources - no need for thread safety here */
static int le_shurrik;

/* extension redirection functions  */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* shurrik_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*shurrik_old_execute)(zend_op_array *op_array TSRMLS_DC);
void shurrik_execute(zend_op_array* TSRMLS_DC);

void (*shurrik_old_execute_internal)(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);
void shurrik_execute_internal(zend_execute_data*,int TSRMLS_DC);


int shurrik_hash_apply(zval **val, Bucket *bHead);
int shurrik_hash_apply_for_zval(zval **val TSRMLS_DC);
int shurrik_hash_apply_for_array(zval **val,int num_args,va_list args,zend_hash_key *hash_key);
int shurrik_hash_apply_for_zval_and_key(zval **val,int num_args,va_list args,zend_hash_key *hash_key);

static const char *shurrik_get_base_filename(const char *filename);
static char *shurrik_get_function_name(zend_op_array *op_array TSRMLS_DC);

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
	*shurrik_data.some_data = "";
	sprintf(shurrik_data.some_data,"==================== start ====================\n");
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
	strcat(shurrik_data.some_data,"\n==================== end ====================\n");
	write(server_fifo_fd, &shurrik_data, sizeof(shurrik_data));
	close(server_fifo_fd);
	return 1;
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
	int i;

	strcat(shurrik_data.some_data,"\033[35;4m");
	strcat(shurrik_data.some_data,file_handle->filename);
	strcat(shurrik_data.some_data,"\033[0m\n");
	
	op_array = old_compile_file(file_handle, type TSRMLS_CC);
	
	if (op_array){
		shurrik_apply_op_array(op_array);
	}

	return op_array;
}

void shurrik_apply_op_array(zend_op_array *op_array){
	zend_op *op;
	zval *z;
	zval *z2;
	zval *r1;
	int i;
	char shurrik_tmp[256];
	
		sprintf(shurrik_tmp,"%-4s%-6s%-30s%-60s%-10s%-10s%-10s\n","id","line","opcode","handler","op1","op2","result");
		strcat(shurrik_data.some_data,shurrik_tmp);
		
		for (i = 0; i < op_array->last; i++){
				sprintf(shurrik_tmp,"%-4d",i);
				strcat(shurrik_data.some_data,shurrik_tmp);
				
				sprintf(shurrik_tmp,"%-6d",op_array->opcodes[i].lineno);
				strcat(shurrik_data.some_data,shurrik_tmp);
				
				sprintf(shurrik_tmp,"%-30s",shurrik_get_opname(op_array->opcodes[i].opcode));
				strcat(shurrik_data.some_data,shurrik_tmp);
				
				op = &op_array->opcodes[i];
				z = &op->op1.u.constant;
				z2 = &op->op2.u.constant;
				r1 = &op->result.u.constant;
				
				sprintf(shurrik_tmp,"%-60s",shurrik_get_opcode_handler(op->opcode,op));
				strcat(shurrik_data.some_data,shurrik_tmp);

			strcat(shurrik_data.some_data,"\033[31m");
			if (op->op1.op_type == IS_CONST){
				if(z->type == IS_STRING || z->type == IS_CONSTANT){
					if (strlen(z->value.str.val) <= 10){
						sprintf(shurrik_tmp,"%-10s",z->value.str.val);
						strcat(shurrik_data.some_data,shurrik_tmp);
					}else {
						strcat(shurrik_data.some_data,z->value.str.val);
						strcat(shurrik_data.some_data,"\t");
					}
				}else if (z->type == IS_NULL){
					strcat(shurrik_data.some_data,"NULL");
				}else if (z->type == IS_LONG || z->type == IS_BOOL){
					sprintf(shurrik_tmp,"%-10d",z->value.lval);
					strcat(shurrik_data.some_data,shurrik_tmp);
				}else if (z->type == IS_DOUBLE){
					sprintf(shurrik_tmp,"%-10d",z->value.dval);
					strcat(shurrik_data.some_data,shurrik_tmp);
				}else if (z->type == IS_ARRAY || z->type == IS_CONSTANT_ARRAY){
					strcat(shurrik_data.some_data,"Array");
				}else if (z->type == IS_OBJECT){
					strcat(shurrik_data.some_data,"Object");
				}else if (z->type == IS_RESOURCE){
					strcat(shurrik_data.some_data,"Resource");
				}else {
					strcat(shurrik_data.some_data,"unknow");
				}
			}else if (op->op1.op_type == IS_TMP_VAR) {
				sprintf(shurrik_tmp,"~%-10p",op->op1.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->op1.op_type == IS_VAR){
				sprintf(shurrik_tmp,"$%-10p",op->op1.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->op1.op_type == IS_CV){
				sprintf(shurrik_tmp,"!%-10p",op->op1.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else {
				sprintf(shurrik_tmp,"%-10s"," ");
				strcat(shurrik_data.some_data,shurrik_tmp);
			}
			strcat(shurrik_data.some_data,"\033[0m");

			strcat(shurrik_data.some_data,"\033[33m");
			if (op->op2.op_type == IS_CONST){
				if (z2->type == IS_STRING || z2->type == IS_CONSTANT){
					if (strlen(z2->value.str.val) <= 10){
						sprintf(shurrik_tmp,"%-10s",z2->value.str.val);
						strcat(shurrik_data.some_data,shurrik_tmp);
					}else {
						strcat(shurrik_data.some_data,z2->value.str.val);
						strcat(shurrik_data.some_data,"\t");
					}
				}else {
					sprintf(shurrik_tmp,"%-10s"," ");
					strcat(shurrik_data.some_data,shurrik_tmp);
				}
			}else if (op->op2.op_type == IS_TMP_VAR){
				sprintf(shurrik_tmp,"~%-10p",op->op2.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->op2.op_type == IS_VAR){
				sprintf(shurrik_tmp,"$%-10p",op->op2.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->op2.op_type == IS_CV){
				sprintf(shurrik_tmp,"!%-10p",op->op2.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else {
				sprintf(shurrik_tmp,"%-10s"," ");
				strcat(shurrik_data.some_data,shurrik_tmp);
			}
			strcat(shurrik_data.some_data,"\033[0m");

			if (op->result.op_type == IS_CONST){
				if (r1->type == IS_STRING || r1->type == IS_CONSTANT){
					strcat(shurrik_data.some_data,r1->value.str.val);
					strcat(shurrik_data.some_data,"\n");
				}else {
					strcat(shurrik_data.some_data,"\n");
				}
			}else if (op->result.op_type == IS_TMP_VAR){
				sprintf(shurrik_tmp,"~%-10p\n",op->result.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->result.op_type == IS_VAR){
				sprintf(shurrik_tmp,"$%-10p\n",op->result.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else if (op->result.op_type == IS_CV){
				sprintf(shurrik_tmp,"!%-10p\n",op->result.u.var);
				strcat(shurrik_data.some_data,shurrik_tmp);
			}else {
				sprintf(shurrik_tmp,"%-10s"," ");
				strcat(shurrik_data.some_data,shurrik_tmp);
				strcat(shurrik_data.some_data,"\n");
			}
		}
}

void shurrik_internal_cat_opline(zend_op *opline){
	char shurrik_tmp[256];
		
	if (opline->op1.op_type == IS_CONST){
		sprintf(shurrik_tmp,"%d",opline->lineno);
		strcat(shurrik_data.some_data,shurrik_tmp);
		strcat(shurrik_data.some_data,"\t");
		strcat(shurrik_data.some_data,opline->op1.u.constant.value.str.val);
		strcat(shurrik_data.some_data,"\n");
	}
}

void shurrik_user_cat_opline(zend_op *opline){
	char shurrik_tmp[256];
		
	if (opline->op1.op_type == IS_CONST){
		if (strcmp(shurrik_get_opname(opline->opcode),"ZEND_DO_FCALL") == 0){
			sprintf(shurrik_tmp,"%d",opline->lineno);
			strcat(shurrik_data.some_data,shurrik_tmp);
			strcat(shurrik_data.some_data,"\t");
			strcat(shurrik_data.some_data,opline->op1.u.constant.value.str.val);
			strcat(shurrik_data.some_data,"\n");
		}
	}
}

static const char *shurrik_get_base_filename(const char *filename){
	const char *ptr;
	int 	found = 0;

	if (!filename)
		return "";

	for (ptr = filename + strlen(filename) - 1; ptr >= filename; ptr--){
		if (*ptr == '/')
			found++;
		if (found == 2)
			return ptr + 1;
	}

	return filename;
}

static char *shurrik_get_function_name(zend_op_array *op_array TSRMLS_DC){
	zend_execute_data *data;
	const char		*func = NULL;
	const char 		*cls = NULL;
	char 			*ret = NULL;
	int 			len;
	zend_function 	*curr_func;

	data = EG(current_execute_data);

	if (data){
		curr_func = data->function_state.function;
		func = curr_func->common.function_name;

		if (func){

			if (curr_func->common.scope)
				cls = curr_func->common.scope->name;
			else if (data->object) 
				cls = Z_OBJCE(*data->object)->name;

			if (cls){
				len = strlen(cls) + strlen(func) + 10;
				ret = (char*)emalloc(len);
				snprintf(ret, len, "%s::%s", cls, func);
			}else {
				ret = estrdup(func);
			}


		}else {
			long     curr_op;
      		int      add_filename = 0;

#if ZEND_EXTENSION_API_NO >= 220100525
      		curr_op = data->opline->extended_value;
#else
      		curr_op = data->opline->op2.u.constant.value.lval;
#endif

		      switch (curr_op) {
		        case ZEND_EVAL:
		          func = "eval";
		          break;
		        case ZEND_INCLUDE:
		          func = "include";
		          add_filename = 1;
		          break;
		        case ZEND_REQUIRE:
		          func = "require";
		          add_filename = 1;
		          break;
		        case ZEND_INCLUDE_ONCE:
		          func = "include_once";
		          add_filename = 1;
		          break;
		        case ZEND_REQUIRE_ONCE:
		          func = "require_once";
		          add_filename = 1;
		          break;
		        default:
		          func = "???_op";
		          break;
		      }

		      if (add_filename){
		      	const char *filename;
		      	int 	len;
		      	filename = shurrik_get_base_filename((curr_func->op_array).filename);
		      	len = strlen("run_init") + strlen(filename) + 3;
		      	ret = (char*)emalloc(len);
		      	snprintf(ret, len, "run_init::%s", filename);
		      }else {
		      	ret = estrdup(func);
		      }
		}

	}

	return ret;
}

void shurrik_execute(zend_op_array *op_array TSRMLS_DC){
	int i;
	char shurrik_tmp[256];
	char *func = NULL;

	/*if (strcmp(shurrik_user_tmp,op_array->filename) != 0){
		sprintf(shurrik_user_tmp,op_array->filename);
		strcat(shurrik_data.some_data,shurrik_user_tmp);
		sprintf(shurrik_tmp,"\n%s\t%s\n","line","function");
		strcat(shurrik_data.some_data,shurrik_tmp);
	}

	for (i = 0; i < op_array->last; i++){
		shurrik_user_cat_opline(&op_array->opcodes[i]);
	}*/

	func = shurrik_get_function_name(op_array TSRMLS_CC);

	php_printf("%s\n", func);
	
	shurrik_old_execute(op_array TSRMLS_CC);
}

void shurrik_execute_internal(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC){
	char shurrik_tmp[256];

	if (strcmp(shurrik_exec_tmp,execute_data_ptr->op_array->filename) != 0){
		sprintf(shurrik_exec_tmp,execute_data_ptr->op_array->filename);
		strcat(shurrik_data.some_data,shurrik_exec_tmp);
		sprintf(shurrik_tmp,"\n%s\t%s\n","line","function");
		strcat(shurrik_data.some_data,shurrik_tmp);
	}

	shurrik_internal_cat_opline(execute_data_ptr->opline);
	
	if (!shurrik_old_execute_internal){
		execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
	}else {
		shurrik_old_execute_internal(execute_data_ptr , return_value_used TSRMLS_CC);
	}
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

	//shurrik_init();

	//old_compile_file = zend_compile_file;
	//zend_compile_file = shurrik_compile_file;

	shurrik_old_execute = zend_execute;	
	zend_execute = shurrik_execute;

	shurrik_old_execute_internal = zend_execute_internal;
	zend_execute_internal = shurrik_execute_internal;

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

	shurrik_init();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(shurrik)
{
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
