; Length-prefixed string in little-endian format
#fn lenstr(str) => le(strlen(str)`32) @ str
