JAPP is a high performance JSON parser written in C.

##Build instructions
To build, clone this repo. 

```
git clone https://github.com/bibhas2/JAPP.git
```

Also clone Cute:

```
git clone https://github.com/bibhas2/Cute.git
```

Then build the two projects:

```
cd Cute
make
cd ..
cd JAPP
make
```

##Usage

Example JSON:

```json
{
  "array": [10.5533E-2, "I \u2665 Miami", 22.89, true , false , null ], 
  "list" : ["One", "Two", "Three"],
  "num": 11.23,
  "first-name": "Barry white",
  "address": {
    "street":"123 Ocean Drive",
    "city": "Miami"
  }
}
```

Prasing example:

```c
const char *json = "{...}"; //A JSON string as above.
JSONParser *p = newJSONParser();
JSONObject *root = jsonParseCString(p, json);

double d = jsonGetNumber(root, "num"); //Will be 11.23
const char *s = jsonGetCString(root, "first-name"); //"Barry white"

JSONObject *array = jsonGetArray(root, "array");

d = jsonGetNumberAt(array, 0); //10.5533E-2
s = jsonGetCStringAt(array, 1); //UTF-8 encoded multi byte string
printf("String is: %s\n", s);

bool b = jsonGetBooleanAt(array, 3); //true
b = jsonIsNullAt(array, 5); //true
b = jsonIsNullAt(array, 0); //false

//Array iteration
JSONObject *list = jsonGetArray(root, "list");
for (int i = 0; i < jsonGetArrayLength(list); ++i) {
	s = jsonGetCStringAt(list, i);
}

deleteJSONParser(p); //Free all parsing related memory
```

##Parsing an I/O Stream
Loading a very large JSON document in a string can be memory intensive.
In such cases stream based processing will avoid the need to load the whole
document as a string.

```c
int fd = open(...); //Open a file or network socket

JSONParser *p = newJSONParser();
JSONObject *o = jsonParseStream(p, 0);
```

##Callback Based Processing
Callbacks allow you to process a JSON document without waiting for
the whole document to be fully parsed. You can do all kinds of
optimizations with this. For example, as the parser completes parsing each
element in a large array, you can process that element in a separate thread.

There are two types of callbacks you can register:

1. **Property parsed handler:** A callback function that is called after 
each named property is parsed. In our example document, that will be 
properties like "list" and "first-name".
2. **Value parsed handler:** A callback that is called after each value, 
named or otherwise, is parsed.  This is necessary to process elements 
in an array since they are unnamed.

Example:

```c
//Called after a named property is parsed
void onPropertyParsed(JSONParser *p, 
	String *name, JSONObject *o) {

        printf("Property parsed: %s\n",
                stringAsCString(name));

}

//Called for every value
void onValueParsed(JSONParser *p, JSONObject *o) {
	if (o->type == JSON_OBJECT) {
		String *street = jsonGetString(o, "street");

		if (street != NULL) {
			//This is an address object
		}
	}
}

//Register the callbacks

JSONParser *p = newJSONParser();

p->onPropertyParsed = onPropertyParsed;
p->onValueParsed = onValueParsed;

jsonParse(p, ...);
```

If you are doing all your document processing from a callback, there is
no need to keep a ``JSONObject`` that has already been processed. You can
free up most of the memory held by a JSONObject by calling ``jsonClear()``.
This can significantly reduce memory overhead when parsing a very large
JSON document. For example:

```c
void onValueParsed(JSONParser *p, JSONObject *o) {
	if (o->type == JSON_OBJECT) {
		String *street = jsonGetString(o, "street");

		if (street != NULL) {
			//This is an address object
			//Process the address
			//...
			//Free up most of the memory for the object and
			//all its children.
			jsonClear(o);
		}
	}
}
```

> By combining stream parsing, callback based processing and freeing up
of objects you can process very large documents with almost constant memory
usage.

##String handling

Internally, JAPP uses the String data type from Cute library to store string. It is a very simple
data structure consisting of mainly a buffer and the length. Use of the String
type will lead to faster string manipulation than NULL terminated C string. 
If you choose to use String, then you can call the String version of
the JAPP API functions. Such as:

```
String *jsonString = stringFromCString(json); //Convert to a String
JSONObject *root = jsonParse(p, jsonString);
String *s = jsonGetString(root, "first-name"); //"Barry white"
```

##Error handling

After parsing, check the `errorCode` property of the parser. If it
is not `ERROR_NONE` then an error had occured. The `errorLine` property
gives you an approximate line number of the document where the error
took place. The `errorMessage` property gives a description of the
problem.

```
JSONParser *p = newJSONParser();
JSONObject *o = jsonParse(p, str);

if (p->errorCode != ERROR_NONE) {
        printf("Parsing failed at line: %d. Message: %s\n",
                p->errorLine, p->errorMessage);
        return 3;
}
```

##Memory management

Memory for all objects and strings returned by a parser method,
such as `jsonGetStringAt()` and `jsonGetObject()` is managed by the parser and you do not
need to free them.  Simply call `deleteJSONParser()` to destroy the
parser and it will free up all memory ever allocated by the parser.

Once a parser is created, it can be used repeatedly to call 
`jsonParse()`. The parser will automatically free all memory for the previously
parsed document before parsing the next one.

If you are processing a document using callbacks you may wish to free up
memory for an JSONObject after you are done processing it. This can be done
using ``jsonClear()``. Read more on this in callback based processing section.

Any situation that can cause invalid memory access, causes the program to abort. This is
done for safety. Common situations are:

- In case of any failure to allocate memory
- Buffer overflow or array index out of bounds
- Reading a JSON value using wrong type. For example, reading a String value using 
jsonGetNumber.

