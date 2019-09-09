/*
 * jsmnRipper.h
 *
 *  Created on: 21 may. 2018
 *      Author: ipserc
 */

#ifndef SOURCE_JSMNRIPPER_H_
#define SOURCE_JSMNRIPPER_H_

#include "jsmn.h"
#include "list.h"

/*
 * Item structure which stores the string path token in a linked list-
 * name is the token name which indicates a field of the JSON message
 * jtype is the JSMN type of the token (JSMN_OBJECT or JSMN_ARRAY)
 * index is the index of the field if the type is JSMN_ARRAY
 */
typedef struct
{
	char * name;
	jsmntype_t jtype;
	int index; //If array the index of the item
} item_t;

/*
 * Prototypes
 */
/* jsmnRipper.c */
int getJsmnTokenLen(jsmntok_t *jsmntok);
int listTokenCreate(list_t *tokenList, char *tpath);
void printItem(void *param);
void freeItem(item_t *item);
int parseJSON(char *jsonMsg, jsmn_parser *parser, jsmntok_t **jsonTokens);
_Bool nextToken(jsmntok_t **jsonToken);
_Bool prevToken(jsmntok_t **jsonToken);
_Bool lastToken(jsmntok_t *jsonToken);
_Bool jumpToTokenPos(jsmntok_t **jsonToken, int newPosition);
void printJsonToken(char *jsonMsg, jsmntok_t *jsonToken);
void printJsonTokens(char *jsonMsg, jsmntok_t *jsonTokens);
void printJsonTokenValue(char *jsonMsg, jsmntok_t *jsonToken);
jsmntok_t *findJsonEngine(list_t *tokenList, char *jsonMsg, jsmntok_t *jsonTokens);
jsmntok_t *findJsonToken(char *tpath, char *jsonMsg, jsmntok_t *jsonTokens);
char *getTokenValue(char *tpath, char *jsonMsg, jsmntok_t *jsonTokens);

#endif /* SOURCE_JSMNRIPPER_H_ */
