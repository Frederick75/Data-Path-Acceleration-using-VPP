#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/pg/pg.h>
#include <vnet/ethernet/ethernet.h>
#include <vnet/ip/ip4.h>

typedef struct {
  u32 teid;
} gtpu_upf_trace_t;

static u8 * format_gtpu_upf_trace (u8 * s, va_list * args) {
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  gtpu_upf_trace_t * t = va_arg (*args, gtpu_upf_trace_t *);
  return format (s, "GTPU-UPF: Processed TEID: 0x%x", t->teid);
}

typedef enum {
  GTPU_UPF_NEXT_IP4_LOOKUP,
  GTPU_UPF_NEXT_DROP,
  GTPU_UPF_N_NEXT,
} gtpu_upf_next_t;

VLIB_NODE_FN (gtpu_upf_node) (vlib_main_t * vm, vlib_node_runtime_t * node, vlib_frame_t * frame) {
  u32 n_left_from, * from, * to_next;
  gtpu_upf_next_t next_index;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  while (n_left_from > 0) {
    u32 n_left_to_next;
    vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);

    /* Vectorized Inner Loop for Packet Acceleration */
    while (n_left_from > 0 && n_left_to_next > 0) {
      u32 bi0 = from[0];
      to_next[0] = bi0;
      from += 1;
      to_next += 1;
      n_left_from -= 1;
      n_left_to_next -= 1;

      vlib_buffer_t * b0 = vlib_get_buffer (vm, bi0);
      ip4_header_t * ip0 = vlib_buffer_get_current (b0);

      /* GTP-U Header processing - stripping GTP-U/UDP/IP header (Decapsulation) */
      u16 gtp_hdr_len = sizeof(ip4_header_t) + 8 + 8; // IP + UDP + GTP-U base
      vlib_buffer_advance (b0, gtp_hdr_len); 

      u32 next0 = GTPU_UPF_NEXT_IP4_LOOKUP;

      if (PREDICT_FALSE(b0->flags & VLIB_BUFFER_IS_TRACED)) {
        gtpu_upf_trace_t * t = vlib_add_trace (vm, node, b0, sizeof (*t));
        t->teid = 0x12345678; // Dummy TEID placeholder
      }

      vlib_validate_buffer_enqueue_x1 (vm, node, next_index, to_next,
                                       n_left_to_next, bi0, next0);
    }
    vlib_put_next_frame (vm, node, next_index, n_left_to_next);
  }
  return frame->n_vectors;
}

VLIB_REGISTER_NODE (gtpu_upf_node) = {
  .name = "5g-upf-gtpu-decap",
  .vector_size = sizeof (u32),
  .format_trace = format_gtpu_upf_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  .n_next_nodes = GTPU_UPF_N_NEXT,
  .next_nodes = {
    [GTPU_UPF_NEXT_IP4_LOOKUP] = "ip4-lookup",
    [GTPU_UPF_NEXT_DROP] = "error-drop",
  },
};
