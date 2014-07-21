#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		"[\"a1\", \"a2\"]"
		);
	JSONParser *p = newJSONParser(str);
	JSONObject *o = jsonParseArray(p);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
	}
	String *s = jsonGetStringAt(o, 1);
	if (s == NULL) {
		puts("Not found.");
		return 1;
	}
	puts(stringAsCString(s));

	return 0;
}
