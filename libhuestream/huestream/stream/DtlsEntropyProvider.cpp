/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/DtlsEntropyProvider.h>

namespace huestream {

#if !defined(unix) && !defined(__unix__) && !defined(__unix) && \
    !defined(__APPLE__) && !defined(_WIN32)

#error "Platform entropy sources only work on Unix, Windows, and consoles if enabled. Implement and inject your own entropyprovider."

#endif


#if defined(CONSOLE_SUPPORT) /* PLATFORM */
#include <huestream/stream/console/DefaultEntropyProviderConsole.h>
#elif defined(_WIN32) && !defined(EFIX64) && !defined(EFI32) /* PLATFORM */

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <wincrypt.h>

bool DtlsEntropyProvider::get_entropy(unsigned char *output, size_t len, size_t *olen) {
    HCRYPTPROV provider;
    *olen = 0;

    if (CryptAcquireContext(&provider, NULL, NULL,
                            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE) {
        return false;
    }

    if (CryptGenRandom(provider, (DWORD) len, output) == FALSE) {
        CryptReleaseContext(provider, 0);
        return false;
    }

    CryptReleaseContext(provider, 0);
    *olen = len;

    return true;
}
#else /* PLATFORM */

/*
 * Test for Linux getrandom() support.
 * Since there is no wrapper in the libc yet, use the generic syscall wrapper
 * available in GNU libc and compatible libc's (eg uClibc).
 */
#if defined(__linux__) && defined(__GLIBC__)
#include <unistd.h>
#include <sys/syscall.h>
#if defined(SYS_getrandom)
#define HAVE_GETRANDOM

static int getrandom_wrapper(void *buf, size_t buflen, unsigned int flags) {
    /* MemSan cannot understand that the syscall writes to the buffer */
#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
    memset(buf, 0, buflen);
#endif
#endif

    return(static_cast<int>(syscall(SYS_getrandom, buf, buflen, flags)));
}

#include <sys/utsname.h>
/* Check if version is at least 3.17.0 */
static int check_version_3_17_plus(void) {
    int minor;
    struct utsname un;
    const char *ver;

    /* Get version information */
    uname(&un);
    ver = un.release;

    /* Check major version; assume a single digit */
    if (ver[0] < '3' || ver[0] > '9' || ver[1] != '.')
        return (-1);

    if (ver[0] - '0' > 3)
        return (0);

    /* Ok, so now we know major == 3, check minor.
     * Assume 1 or 2 digits. */
    if (ver[2] < '0' || ver[2] > '9')
        return (-1);

    minor = ver[2] - '0';

    if (ver[3] >= '0' && ver[3] <= '9')
        minor = 10 * minor + ver[3] - '0';
    else if (ver[3] != '.')
        return (-1);

    if (minor < 17)
        return (-1);

    return (0);
}
static int has_getrandom = -1;
#endif /* SYS_getrandom */
#endif /* __linux__ */

#include <stdio.h>

bool DtlsEntropyProvider::get_entropy(unsigned char *output, size_t len, size_t *olen) {
    FILE *file;
    size_t read_len;

#if defined(HAVE_GETRANDOM)
    if (has_getrandom == -1)
        has_getrandom = (check_version_3_17_plus() == 0);

    if (has_getrandom) {
        int ret;

        if ((ret = getrandom_wrapper(output, len, 0)) < 0)
            return false;

        *olen = ret;
        return true;
    }
#endif /* HAVE_GETRANDOM */

    *olen = 0;

    file = fopen("/dev/urandom", "rb");
    if (file == NULL)
        return false;

    read_len = fread(output, 1, len, file);
    if (read_len != len) {
        fclose(file);
        return true;
    }

    fclose(file);
    *olen = len;

    return true;
}

#endif /* PLATFORM */

}  // namespace huestream
