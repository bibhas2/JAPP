#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "Parser.h"

#define FAIL(cond, msg) if (cond) {puts(msg); return NULL;}

static int printDict(const char *key, void *value) {
	printf("Key: \"%s\"\n", key);
	jsonPrintObject((JSONObject*) value);

	return 1;
}

void
jsonPrintObject(JSONObject *o) {
	switch (o->type) {
		case JSON_STRING:
			puts("Printing String...");
			printf("\"%s\"\n", 
				stringAsCString(o->value.string));
			break;
		case JSON_OBJECT:
			puts("Printing object...");
			dictionaryIterate(o->value.object,
				printDict);
			break;
		default:
			printf("Unsupported type.");
	}
}

JSONParser *newJSONParser(String *dataToParse) {
	JSONParser *parser = malloc(sizeof(JSONParser));

	parser->data = dataToParse;
	parser->position = 0;

	return parser;
}
void deleteJSONParser(JSONParser *parser) {
	parser->data = NULL;
	parser->position = 0;

	free(parser);
}
String *jsonGetString(JSONObject *o) {
	assert(o->type == JSON_STRING);

	return o->value.string;
}

double jsonGetNumber(JSONObject *o) {
	assert(o->type == JSON_NUMBER);

	return o->value.number;
}
Dictionary *jsonGetObject(JSONObject *o) {
	assert(o->type == JSON_OBJECT);

	return o->value.object;
}
Array *jsonGetArray(JSONObject *o) {
	assert(o->type == JSON_ARRAY);

	return o->value.array;
}
bool jsonGetBoolean(JSONObject *o) {
	assert(o->type == JSON_BOOLEAN);

	return o->value.booleanValue;
}
bool jsonIsNull(JSONObject *o) {
	return(o->type == JSON_NULL);
}

static char peek(JSONParser *parser) {
	if (parser->position >= parser->data->length) {
		return 0;
	}
	return stringGetChar(parser->data, parser->position);
}
static char pop(JSONParser *parser) {
	if (parser->position >= parser->data->length) {
		return 0;
	}

	parser->position += 1;

	return stringGetChar(parser->data, parser->position - 1);
}
static void putback(JSONParser *parser) {
	assert(parser->position > 0);
	parser->position -= 1;
}

JSONObject *newJSONObject(JSONType type) {
	JSONObject *o = malloc(sizeof(JSONObject));

	o->type = type;

	return o;
}

static void eatSpace(JSONParser *parser) {
	char ch = pop(parser);

	if (ch == 0) return;

	while (isspace(ch)) {
		ch = pop(parser);

		if (ch == 0) return;
	}
	putback(parser);
}

static JSONObject* parseValue(JSONParser *parser);

static String* parseString(JSONParser *parser) {
	eatSpace(parser);

	char ch = pop(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	assert(ch == '"');

	String *s = newString();

	while ((ch = pop(parser)) != '"') {
		FAIL(ch == 0, "Premature end of JSON string.");

		stringAppendChar(s, ch);
	}

	return s;
}

static double parseNumber(JSONParser *parser) {
	return 0.0;
}

static JSONObject *parseObject(JSONParser *parser) {
	JSONObject *o = newJSONObject(JSON_OBJECT);
	Dictionary *d = newDictionary();
	String *name = NULL;

	o->value.object = d;

	char ch = pop(parser);

	FAIL(ch == 0, "Premature end of JSON string.");
	FAIL(ch != '{', "Invalid JSON object format.");

	while ((ch = pop(parser)) != '}') {
		FAIL(ch == 0, "Premature end of JSON string.");

		if (ch == '"') {
			putback(parser);
			name = parseString(parser);
		} else if (ch == ':') {
			JSONObject *val = parseValue(parser);
			dictionaryPut(d, 
				stringAsCString(name), val);
			deleteString(name);
			name = NULL;
		}
	}

	return o;
}

static Array* parseArray(JSONParser *parser) {
	return NULL;
}

static JSONObject* parseValue(JSONParser *parser) {
	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, "Premature end of JSON string.");

	if (ch == '"') {
		String *str = parseString(parser);
		JSONObject *o = newJSONObject(JSON_STRING);
		o->value.string = str;
		return o;
	} else if (ch == '{') {
		return parseObject(parser);
	} else if (ch == '[') {
		Array *a = parseArray(parser);
		JSONObject *o = newJSONObject(JSON_ARRAY);
		o->value.array = a;

		return o;
	} else if (isdigit(ch)) {
		JSONObject *o = newJSONObject(JSON_NUMBER);
		o->value.number = parseNumber(parser);

		return o;
	} //add true, false, null

	return NULL;
}

JSONObject *jsonParse(JSONParser *parser) {
	eatSpace(parser);

	char ch = peek(parser);

	FAIL(ch == 0, "Premature end of JSON string.");

	if (ch == '{') {
		return parseObject(parser);
	} else if (ch == '[') {
		Array *a = parseArray(parser);
		JSONObject *o = newJSONObject(JSON_ARRAY);
		o->value.array = a;

		return o;
	}

	return NULL;
}
