#ifndef UNUSED_H
#define UNUSED_H

#ifndef NS_UNUSED
# define NS_UNUSED(x) ((void)(x))
#endif

#ifndef NS_UNUSED_GLOBAL
#if defined(__GNUC__)
# define NS_UNUSED_GLOBAL(x) x __attribute__((unused))
#elif defined(__LCLINT__)
# define NS_UNUSED_GLOBAL(x) /*@unused@*/ x
#else
# define NS_UNUSED_GLOBAL(x) x
#endif
#endif

#endif /* UNUSED_H */
