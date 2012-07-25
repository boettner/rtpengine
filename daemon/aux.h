#ifndef __AUX_H__
#define __AUX_H__



#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <glib.h>
#include <pcre.h>
#include <stdarg.h>
#include <arpa/inet.h>




#define OFFSET_OF(t,e)		((unsigned int) (unsigned long) &(((t *) 0)->e))
#define ZERO(x)			memset(&(x), 0, sizeof(x))

#define IPF			"%u.%u.%u.%u"
#define IPP(x)			((unsigned char *) (&(x)))[0], ((unsigned char *) (&(x)))[1], ((unsigned char *) (&(x)))[2], ((unsigned char *) (&(x)))[3]
#define IP6F			"%x:%x:%x:%x:%x:%x:%x:%x"
#define IP6P(x)			ntohs(((u_int16_t *) (x))[0]), \
				ntohs(((u_int16_t *) (x))[1]), \
				ntohs(((u_int16_t *) (x))[2]), \
				ntohs(((u_int16_t *) (x))[3]), \
				ntohs(((u_int16_t *) (x))[4]), \
				ntohs(((u_int16_t *) (x))[5]), \
				ntohs(((u_int16_t *) (x))[6]), \
				ntohs(((u_int16_t *) (x))[7])
#define D6F			"["IP6F"]:%u"
#define D6P(x)			IP6P((x).sin6_addr.s6_addr), ntohs((x).sin6_port)
#define DF			IPF ":%u"
#define DP(x)			IPP((x).sin_addr.s_addr), ntohs((x).sin_port)

#define BIT_ARRAY_DECLARE(name, size)	int name[((size) + sizeof(int) * 8 - 1) / (sizeof(int) * 8)]




typedef int (*parse_func)(char **, void **, void *);

GList *g_list_link(GList *, GList *);
GQueue *pcre_multi_match(pcre **, pcre_extra **, const char *, const char *, unsigned int, parse_func, void *);
void strmove(char **, char **);
void strdupfree(char **, const char *);


#if !GLIB_CHECK_VERSION(2,14,0)
#define G_QUEUE_INIT { NULL, NULL, 0 }
void g_string_vprintf(GString *string, const gchar *format, va_list args);
void g_queue_clear(GQueue *);
#endif


static inline void nonblock(int fd) {
	fcntl(fd, F_SETFL, O_NONBLOCK);
}
static inline void reuseaddr(int fd) {
	int one = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
}
static inline void ipv6only(int fd, int yn) {
	setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yn, sizeof(yn));
}

static inline int bit_array_isset(int *name, unsigned int bit) {
	return name[(bit) / (sizeof(int) * 8)] & (1 << ((bit) % (sizeof(int) * 8)));
}

static inline void bit_array_set(int *name, unsigned int bit) {
	name[(bit) / (sizeof(int) * 8)] |= 1 << ((bit) % (sizeof(int) * 8));
}

static inline void bit_array_clear(int *name, unsigned int bit) {
	name[(bit) / (sizeof(int) * 8)] &= ~(1 << ((bit) % (sizeof(int) * 8)));
}

static inline char chrtoupper(char x) {
	return x & 0xdf;
}

static inline void swap_ptrs(void *a, void *b) {
	void *t, **aa, **bb;
	aa = a;
	bb = b;
	t = *aa;
	*aa = *bb;
	*bb = t;
}

static inline void in4_to_6(struct in6_addr *o, u_int32_t ip) {
	o->s6_addr32[0] = 0;
	o->s6_addr32[1] = 0;
	o->s6_addr32[2] = htonl(0xffff);
	o->s6_addr32[3] = ip;
}

static inline void smart_ntop(char *o, struct in6_addr *a, size_t len) {
	const char *r;

	if (IN6_IS_ADDR_V4MAPPED(a))
		r = inet_ntop(AF_INET, &(a->s6_addr32[3]), o, len);
	else
		r = inet_ntop(AF_INET6, a, o, len);

	if (!r)
		*o = '\0';
}

static inline void smart_ntop_p(char *o, struct in6_addr *a, size_t len) {
	int l;

	if (IN6_IS_ADDR_V4MAPPED(a)) {
		if (!inet_ntop(AF_INET, &(a->s6_addr32[3]), o, len))
			*o = '\0';
	}
	else {
		*o = '[';
		if (!inet_ntop(AF_INET6, a, o+1, len-2)) {
			*o = '\0';
			return;
		}
		l = strlen(o);
		o[l] = ']';
		o[l+1] = '\0';
	}
}

static inline int smart_pton(int af, char *src, void *dst) {
	char *p;
	int ret;

	if (af == AF_INET6) {
		if (src[0] == '[' && (p = strchr(src, ']'))) {
			*p = '\0';
			ret = inet_pton(af, src+1, dst);
			*p = ']';
			return ret;
		}
	}
	return inet_pton(af, src, dst);
}



#endif
