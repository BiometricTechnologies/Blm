DEL /F /S /Q /A "version.h"
hg parent --template "const char* version = \"1.0.{rev}\";\n" > version.h