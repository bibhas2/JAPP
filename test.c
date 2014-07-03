#include <stdio.h>

#include "Parser.h"

int main() {
	String *str = newStringWithCString(
		" {\"key1\" : \"val1\", \"key2\":\"val2\"   }  "
		);
	JSONParser *p = newJSONParser(str);
	JSONObject *o = jsonParseObject(p);

	if (o == NULL) {
		puts("Parsing failed.");
		return 1;
	}
	String *s = jsonGetString(o, "key1");
	if (s == NULL) {
		puts("Not found.");
		return 1;
	}
	puts(stringAsCString(s));
	s = jsonGetString(o, "key2");
	puts(stringAsCString(s));

	return 0;
}
