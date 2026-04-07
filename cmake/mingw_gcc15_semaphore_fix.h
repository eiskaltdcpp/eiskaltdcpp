/*
 * Workaround for GCC 15 on MSYS2/MinGW: libstdc++ <semaphore> uses the POSIX
 * sem_t backend because _GLIBCXX_HAVE_POSIX_SEMAPHORE was set when GCC was
 * built, but the actual POSIX semaphore functions (sem_init, sem_timedwait,
 * etc.) are unavailable at application compile time on Windows/UCRT.
 *
 * By pre-including <bits/c++config.h> (which has an include guard) and then
 * undefining the macro, we force the atomic-based fallback in
 * <bits/semaphore_base.h>. Any subsequent #include of <bits/c++config.h> is
 * a no-op due to the include guard, so the #undef persists.
 *
 * This header is injected via  -include cmake/mingw_gcc15_semaphore_fix.h
 * only when MINGW AND GCC >= 15.
 */
#if defined(__MINGW32__) && defined(__cplusplus)
#  include <bits/c++config.h>
#  undef _GLIBCXX_HAVE_POSIX_SEMAPHORE
#endif
