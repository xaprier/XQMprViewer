
#ifndef QVV_EXPORT_H
#define QVV_EXPORT_H

#ifdef QVV_STATIC_DEFINE
#  define QVV_EXPORT
#  define QVV_NO_EXPORT
#else
#  ifndef QVV_EXPORT
#    ifdef XQVtkViewport_EXPORTS
        /* We are building this library */
#      define QVV_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QVV_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QVV_NO_EXPORT
#    define QVV_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QVV_DEPRECATED
#  define QVV_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QVV_DEPRECATED_EXPORT
#  define QVV_DEPRECATED_EXPORT QVV_EXPORT QVV_DEPRECATED
#endif

#ifndef QVV_DEPRECATED_NO_EXPORT
#  define QVV_DEPRECATED_NO_EXPORT QVV_NO_EXPORT QVV_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QVV_NO_DEPRECATED
#    define QVV_NO_DEPRECATED
#  endif
#endif

#endif /* QVV_EXPORT_H */
