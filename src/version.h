#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

#define ZERO_NUM 0
#define FILE_DESCRIPTION "mini fix patch DLL."
#define FILE_VERSION_STR "0.0.0.0"
#define FILE_VERSION ZERO_NUM, ZERO_NUM, ZERO_NUM, ZERO_NUM
#define PRODUCT_VERSION FILE_VERSION_STR

#endif
