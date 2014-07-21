#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		"{\"array\":[\"a1\", \"a2\"]}"
		);
	JSONParser *p = newJSONParser(str);
	JSONObject *o = jsonParse(p);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
	}
	JSONObject *a = jsonGetArray(o, "array");
	String *s = jsonGetStringAt(a, 1);
	if (s == NULL) {
		puts("Not found.");
		return 1;
	}
	puts(stringAsCString(s));

	return 0;
}
