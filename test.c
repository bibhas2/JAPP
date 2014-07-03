#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		"{\"key\" : \"val\", \"key2\":\"val2\"}"
		);
	JSONParser *p = newJSONParser(str);
	JSONObject *o = jsonParse(p);

	Dictionary *d = jsonGetObject(o);
	JSONObject *val = dictionaryGet(d, "key2");
	puts(stringAsCString(jsonGetString(val)));

	return 0;
}
