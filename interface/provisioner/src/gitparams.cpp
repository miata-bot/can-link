#include <string.h>

#include "gitparams.h"

#define ADD_QUOTES_HELPER(s) #s
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

#ifdef GIT_DESCRIBE
static const char* __GIT_DESCRIBE = ADD_QUOTES(GIT_DESCRIBE);
#else
static const char* __GIT_DESCRIBE = "unavailable";
#endif

static const char* __PATTERN = "provisioner-";

const char* git_describe()
{
    const char* str = strstr(__GIT_DESCRIBE, __PATTERN);
    if (str != NULL) {
        return str + strlen(__PATTERN);
    }
    return __GIT_DESCRIBE;
}
