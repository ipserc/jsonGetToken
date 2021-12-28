/*
 * jsmnRipper.c
 *
 *  Created on: 21 may. 2018
 *      Author: ipserc
 *
 *
 *
 */

#include "jsmnRipper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "list.h"
//#include "jsmn.h"
#include "errtra.h"

#define JSMN_PARENT_LINKS

/*
 * Gets the length of the token pointed by jsmntok.
 */
int getJsmnTokenLen(jsmntok_t * jsmntok)
{
	return jsmntok->end - jsmntok->start;
}

/*
 * Transforms the token path from string format to an item of item_t type.
 * Appends the item found in linked list of items.
 */
int listTokenCreate(list_t * tokenList, char * tpath)
{
	item_t item;
	char * strIndex;
	char * tpathtok = strtok(tpath, ".");

	while (tpathtok)
	{
		item.index = 0;
		item.jtype = JSMN_OBJECT;
		strIndex = strchr(tpathtok, '[');
		if (strIndex)
		{
			*strIndex++ = '\0';
			item.jtype = JSMN_ARRAY;
			while (*strIndex != ']')
			{
				item.index *= 10;
				item.index += *strIndex++ - '0';
			}
		}

		item.name = malloc(strlen(tpathtok)+1);
		sprintf(item.name, "%s", tpathtok);
		//TRACE("Allocated item->name(%p):%s",item.name, item.name);
		tokenList = listAppend(tokenList, &item, sizeof(item_t));
		tpathtok = strtok(NULL, ".");
	}
	return (int)listItemsCount(tokenList);
}

/*
 * Prints the info stored in the item.
 * It is useful for debuging purposes
*/
void printItem(void * param)
{
	item_t * item  = (item_t *)param;
	//TRACE("Address item(%p)",item);
	//TRACE("Address item->name(%p):%s",item->name, item->name);
	printf("item->name:%s\n", item->name);
	printf("item->jtype:%d\n", item->jtype);
	printf("item->index:%d\n", item->index);
}

/*
 * Structural Function. Frees the allocated memory for "name" in the item_t structure
 */
void freeItem(item_t * item)
{
	//item = (list_t *)itemList->tail->item
	//TRACE("freeing item->name(%p):%s", item->name, item->name);
	free(item->name);
}

/*
 * Structural Function. Main call to JSMN parser.
 * jsmn_parse parses the JSON message and put it into an array of tokens.
 * jsonTokens points the beginning of the array.
 * The last item of the array is a "0" token, so we can control when we have reached the end of the array if we get a "0" of jsmntok_t type.
 */
int parseJSON(char * jsonMsg, jsmn_parser * parser, jsmntok_t ** jsonTokens)
{
	jsmn_init(parser);
	int tokenCount = jsmn_parse(parser, jsonMsg, strlen(jsonMsg), (jsmntok_t *)NULL, 0);
	if (tokenCount < 0) {
		  printf("unable to read tokens from JSON message");
		  return tokenCount;
	}
	// one token more to end the array with nulls
	*jsonTokens = malloc(sizeof(jsmntok_t) * (tokenCount+1));
	memset(*jsonTokens, 0, sizeof(jsmntok_t) * (tokenCount+1));
	jsmn_init(parser);
	jsmn_parse(parser, jsonMsg, strlen(jsonMsg), *jsonTokens, sizeof(jsmntok_t) * (tokenCount+1));
	return tokenCount;
}

/*
 * Structural Function. Jumps to the next token pointed by jsonToken
 */
bool nextToken(jsmntok_t ** jsonToken)
{
	++(*jsonToken);
	jsmntok_t * jsmnToken = *jsonToken;
	return (jsmnToken->start + jsmnToken->end + jsmnToken->size + jsmnToken->parent);
}

/*
 * Structural Function. Jumps to the previous token pointed by jsonToken
 */
bool prevToken(jsmntok_t ** jsonToken)
{
	jsmntok_t * token = *jsonToken;
	if (--token->start == 0) return false;
	--(*jsonToken);
	return true;
}

/*
 * Checks if the token pointed by jsonToken is the last token of the tokens array.
 * To identify the last token, the las token has to have its values "start", "end", "size" and "parent" equal to ZERO.
 * Returns TRUE if the token pointed by jsonToken is the last.
 */
bool lastToken(jsmntok_t * jsmnToken)
{
	return !(jsmnToken->start + jsmnToken->end + jsmnToken->size + jsmnToken->parent);
}

/**
 * Structural Function. Jumps to the token which starts at the next position given by newPosition.
 * @param jsonToken jsmntok_t**
 * @param newPosition int
 * @return false (0) if there aren't any more tokens, true (any number) if there are one more
 */
bool jumpToTokenPos(jsmntok_t ** jsonToken, int newPosition)
{
	bool retVal = false;
	while (((jsmntok_t *)(*jsonToken))->start < newPosition)
	{
		retVal = nextToken(jsonToken);
		if (retVal == 0) break;
		/* TRACE * /
		TRACE("**************** newPosition:%d", newPosition);
		TRACE("**************** jsonToken->start:%d", ((jsmntok_t *)(*jsonToken))->start);
		TRACE("**************** retVal:%d", retVal);
		if ((((jsmntok_t *)(*jsonToken))->start) == 0) exit(1000);
		/* TRACE */

	}
	return retVal;
}

/*
 * Structural Function. Prints the token value and the start, end, size, type, and parent values of an specific token pointed by jsonToken
 */
void printJsonToken(char * jsonMsg, jsmntok_t * jsonToken)
{
	char tokenfmt[50];

	sprintf(tokenfmt, "%s%lu%s", "Token:%.", (size_t)getJsmnTokenLen(jsonToken), "s start:%d end:%d size:%d type:%d parent:%d\n");
	printf(tokenfmt, &jsonMsg[jsonToken->start], jsonToken->start, jsonToken->end, jsonToken->size, jsonToken->type, jsonToken->parent);
}

/*
 * Structural Function. Prints the information of all the tokens of the parsed JSON.
 */
void printJsonTokens(char * jsonMsg, jsmntok_t * jsonTokens)
{
	jsmntok_t * jsonToken = jsonTokens;

	while (nextToken(&jsonToken))
	{
		printJsonToken(jsonMsg, jsonToken);
	}
}

/*
 * Structural Function. For an specific token prints the next value.
 */
void printJsonTokenValue(char * jsonMsg, jsmntok_t * jsonToken)
{
	printJsonToken(jsonMsg, ++jsonToken);
}

/*
 * Finds the token mapped in the path kept in the list tokenList.
 * Returns the token found or NULL if the token doesn't exist.
 */
jsmntok_t * findJsonEngine(list_t * tokenList, char * jsonMsg, jsmntok_t * jsonTokens)
{
	jsmntok_t * jsonToken = jsonTokens;
	node_t * tokenListNode = tokenList->head;
	item_t * item = (item_t *)((node_t *)(tokenListNode->item));
	char * jsonTokenItem;
	int indexCount = 0;
	int tokenIndex = 0;
	bool cont; //Continue flag

	if (getJsmnTokenLen(jsonToken) < 0) return (jsmntok_t *) NULL;
	if (!memcmp("", &jsonMsg[jsonToken->start], (size_t)getJsmnTokenLen(jsonToken))) return (jsmntok_t *) NULL;
	cont = nextToken(&jsonToken);
	while (cont)
	{
		jsonTokenItem = malloc((size_t)getJsmnTokenLen(jsonToken)+1);
		jsonTokenItem[(size_t)getJsmnTokenLen(jsonToken)] = '\0';
		strncpy(jsonTokenItem, &jsonMsg[jsonToken->start], (size_t)getJsmnTokenLen(jsonToken));
		//TRACE("Token to find item->name:%s", item->name);
		//TRACE("jsonTokenItem:%s", jsonTokenItem);
		//printf("%s", "***** Test "); printJsonToken(jsonMsg, jsonToken); puts("");

		if (!strcmp(item->name, jsonTokenItem))
		{
			if (item->jtype == JSMN_ARRAY)
			{
				tokenIndex = item->index;
			}

			cont = nextToken(&jsonToken);
			if (jsonToken->type == JSMN_ARRAY)
			{
				cont = nextToken(&jsonToken);
			}
			if (jsonToken->type == JSMN_OBJECT)
			{
				while (tokenIndex > indexCount)
				{
					++indexCount;
					cont = jumpToTokenPos(&jsonToken, jsonToken->end);
				}
				cont = nextToken(&jsonToken);
			}
			if (tokenListNode->next)
			{
				indexCount = 0;
				tokenIndex = 0;
				tokenListNode = tokenListNode->next;
				item = (item_t *)((node_t *)(tokenListNode->item));
				//TRACE("**** %s Token to find item->name:%s", (tokenListNode->next) ? "Next" : "Last", item->name);
			}
			else // item found
			{
				free(jsonTokenItem);
				return jsonToken;
			}
		}
		else
		{
			switch (jsonToken->type)
			{
				case JSMN_OBJECT:
					cont = jumpToTokenPos(&jsonToken, jsonToken->end);
					break;
				case JSMN_ARRAY:
				case JSMN_STRING:
				case JSMN_PRIMITIVE:
				case JSMN_UNDEFINED:
					cont = nextToken(&jsonToken);
					break;
			}
		}
		free(jsonTokenItem);
	}
	return (jsmntok_t *)NULL;
}

/*
 * Finds the token mapped in the string tpath.
 * tpath should be expressed in the form "field1.field2[index2].field3[index3]. ... .fieldN"
 * Returns the token found or NULL if the token couldn't be found.
 */
jsmntok_t * findJsonToken(char * tpath, char * jsonMsg, jsmntok_t * jsonTokens)
{
	list_t * tokenList;
	jsmntok_t * jsonTokenFound;
	char * tokenPath = malloc(strlen(tpath)+1);

	sprintf(tokenPath, "%s", tpath);
	listNew(&tokenList);
	//TRACE("tokenPath:%s:", tokenPath);
	if (listTokenCreate(tokenList, tokenPath))
	{
		jsonTokenFound = findJsonEngine(tokenList, jsonMsg, jsonTokens);
		free(tokenPath);
		listDestroy(tokenList, (void *)freeItem);
	}
	else TRACE("%s", "Unable to generate token path list.");
	return jsonTokenFound;
}

/*
 * Finds the token mapped in the string tpath.
 * tpath should be expressed in the form "field1.field2[index2].field3[index3]. ... .fieldN"
 * Returns the VALUE token found which is the next token in the sequence of the tokens array.
 * Returns a NULL string if the token couldn't be found.
 */
char * getTokenValue(char * tpath, char * jsonMsg, jsmntok_t * jsonTokens)
{
	jsmntok_t * jsonTokenFound;
	char * tokenValue = (char *)NULL;
	char tokenfmt[50];

	jsonTokenFound = findJsonToken(tpath, jsonMsg, jsonTokens);
	if (jsonTokenFound)
	{
		tokenValue = malloc((size_t)getJsmnTokenLen(jsonTokenFound)+1);
		sprintf(tokenfmt, "%s%lu%s", "%.", (size_t)getJsmnTokenLen(jsonTokenFound), "s");
		sprintf(tokenValue, tokenfmt, &jsonMsg[jsonTokenFound->start]);
	}
	return tokenValue;
}
