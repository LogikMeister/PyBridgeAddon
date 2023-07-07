#ifndef _UTILS_H_
#define _UTILS_H_

inline int setenv(const char *name, const char *value, int overwrite) {
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}

#endif