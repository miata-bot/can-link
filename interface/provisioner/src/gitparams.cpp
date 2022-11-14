#include <string.h>

#include "gitparams.h"

#define ADD_QUOTES_HELPER(s) #s
#define ADD_QUOTES(s) ADD_QUOTES_HELPER(s)

#ifdef GIT_DESCRIBE
static const char* __GIT_DESCRIBE = ADD_QUOTES(GIT_DESCRIBE);
#else
static const char* __GIT_DESCRIBE = "unavailable";
#endif

#ifdef GIT_COMMIT
static const char* __GIT_COMMIT = ADD_QUOTES(GIT_COMMIT);
#else
static const char* __GIT_COMMIT = "unavailable";
#endif

#ifdef BUILD_EPOCH
static const char* __BUILD_EPOCH = ADD_QUOTES(BUILD_EPOCH);
#else
static const char* __BUILD_EPOCH = "unavailable";
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

const char* git_commit()
{
    return __GIT_COMMIT;
}

const char* build_epoch()
{
    return __BUILD_EPOCH;
}
