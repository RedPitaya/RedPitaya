/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <ngx_core.h>

/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
#define cJSON_2dFloatArray 7
#define cJSON_VerFloat 8

#define cJSON_IsReference 256

/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	float valuedouble;			/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */

    /* Used for 2d float array - stored in one cJSON! */
    float *d2_val1;
    float *d2_val2;
    int    d2_len;
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

/* Supply malloc, realloc and free functions to cJSON */
extern void cJSON_InitHooks(cJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished. */
extern cJSON *cJSON_Parse(const char *value, ngx_pool_t *pool);
/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *cJSON_Print(cJSON *item, ngx_pool_t *pool);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *cJSON_PrintUnformatted(cJSON *item, ngx_pool_t *pool);
/* Delete a cJSON entity and all subentities. */
extern void   cJSON_Delete(cJSON *c, ngx_pool_t *pool);

/* Returns the number of items in an array (or object). */
extern int	  cJSON_GetArraySize(cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern cJSON *cJSON_GetArrayItem(cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
extern const char *cJSON_GetErrorPtr(void);
	
/* These calls create a cJSON item of the appropriate type. */
extern cJSON *cJSON_CreateNull(ngx_pool_t *pool);
extern cJSON *cJSON_CreateTrue(ngx_pool_t *pool);
extern cJSON *cJSON_CreateFalse(ngx_pool_t *pool);
extern cJSON *cJSON_CreateBool(int b, ngx_pool_t *pool);
extern cJSON *cJSON_CreateNumber(double num, ngx_pool_t *pool);
extern cJSON *cJSON_CreateVerFloat(double num, ngx_pool_t *pool);
extern cJSON *cJSON_CreateString(const char *string, ngx_pool_t *pool);
extern cJSON *cJSON_CreateArray(ngx_pool_t *pool);
extern cJSON *cJSON_CreateObject(ngx_pool_t *pool);
/* These utilities create an Array of count items. */
extern cJSON *cJSON_CreateIntArray(const int *numbers,int count, ngx_pool_t *pool);
extern cJSON *cJSON_CreateFloatArray(const float *numbers,int count, ngx_pool_t *pool);
extern cJSON *cJSON_CreateDoubleArray(const double *numbers,int count, ngx_pool_t *pool);
extern cJSON *cJSON_CreateStringArray(const char **strings,int count, ngx_pool_t *pool);
extern cJSON *cJSON_Create2dFloatArray(const float *num1, const float *num2, 
                                       int count, ngx_pool_t *pool);

/* Append item to the specified array/object. */
extern void cJSON_AddItemToArray(cJSON *array, cJSON *item);
    extern void	cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item, ngx_pool_t *pool);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
extern void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item, ngx_pool_t *pool);
extern void	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item, ngx_pool_t *pool);

/* Remove/Detatch items from Arrays/Objects. */
extern cJSON *cJSON_DetachItemFromArray(cJSON *array,int which);
extern void   cJSON_DeleteItemFromArray(cJSON *array,int which, ngx_pool_t *pool);
extern cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string);
extern void   cJSON_DeleteItemFromObject(cJSON *object,const char *string, ngx_pool_t *pool);
	
/* Update array items. */
extern void cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem, ngx_pool_t *pool);
extern void cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem, ngx_pool_t *pool);

/* Duplicate a cJSON item */
extern cJSON *cJSON_Duplicate(cJSON *item,int recurse, ngx_pool_t *pool);
/* Duplicate will create a new, identical cJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
    extern cJSON *cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated, ngx_pool_t *pool);

extern void cJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define cJSON_AddNullToObject(object,name, pool)		cJSON_AddItemToObject(object, name, cJSON_CreateNull(pool))
#define cJSON_AddTrueToObject(object,name, pool)		cJSON_AddItemToObject(object, name, cJSON_CreateTrue(pool))
#define cJSON_AddFalseToObject(object,name, pool)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse(pool))
#define cJSON_AddBoolToObject(object,name,b, pool)	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b, pool))
#define cJSON_AddNumberToObject(object,name,n, pool)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n, pool))
#define cJSON_AddStringToObject(object,name,s, pool)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s, pool))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cJSON_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))

#ifdef __cplusplus
}
#endif

#endif
