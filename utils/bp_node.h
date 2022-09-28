#define	DEFAULT_TTL 100

#include <bp.h>

int	receive_bundle(char *ownEid, char *buffer, size_t bufflen);

int	send_bundle(char *sourceEid, char *destEid, char *text, int textlen, int ttl);