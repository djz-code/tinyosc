#include <stdlib.h>
#include <stdarg.h>

typedef struct {
  int32_t size;  // Bigendian.
  char *data;
} osc_packet;

int osc_pack_message(osc_packet *packet, int capacity,
    const char *address, const char *types, ...);

int osc_unpack_message(const osc_packet *packet,
    const char *address, const char *types, ...);

int osc_make_bundle(osc_packet *bundle, int capacity, int64_t time);

int osc_add_packet_to_bundle(
    osc_packet *bundle, int capacity, osc_packet *packet);

osc_packet *osc_next_packet_from_bundle(
    osc_packet *bundle, osc_packet *current);

int osc_is_bundle(osc_packet *packet);
