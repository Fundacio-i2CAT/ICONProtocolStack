#define	DEFAULT_TTL 200

#include <bp.h>

int	receive_bundle(char *ownEid, char *buffer, size_t bufflen);

int	send_bundle(char *sourceEid, char *destEid, char *texts, size_t *textslen, int n_texts, int ttl);