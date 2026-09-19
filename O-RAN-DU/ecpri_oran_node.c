#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/ethernet/ethernet.h>

#define ECPRI_ETHERTYPE 0xAEFE

typedef struct {
  u8 ecpri_version_type;
  u8 ecpri_message_type;
  u16 ecpri_payload_size;
  u16 ecpri_pc_id; // Physical Channel / Real-time Control Data ID
  u16 ecpri_seq_id;
} __attribute__((packed)) ecpri_header_t;

typedef enum {
  ECPRI_NEXT_L2_OUTPUT,
  ECPRI_NEXT_DROP,
  ECPRI_N_NEXT,
} ecpri_next_t;

VLIB_NODE_FN (ecpri_oran_node) (vlib_main_t * vm, vlib_node_runtime_t * node, vlib_frame_t * frame) {
  u32 n_left_from, * from, * to_next;
  ecpri_next_t next_index = node->cached_next_index;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;

  while (n_left_from > 0) {
    u32 n_left_to_next;
    vlib_get_next_frame (vm, node, next_index, to_next, n_left_to_next);

    while (n_left_from > 0 && n_left_to_next > 0) {
      u32 bi0 = from[0];
      to_next[0] = bi0;
      from += 1;
      to_next += 1;
      n_left_from -= 1;
      n_left_to_next -= 1;

      vlib_buffer_t * b0 = vlib_get_buffer (vm, bi0);
      
      /* Access Ethernet header */
      ethernet_header_t * eth0 = vlib_buffer_get_current (b0);

      /* Validate eCPRI EtherType */
      u32 next0 = ECPRI_NEXT_DROP;
      if (clib_net_to_host_u16(eth0->type) == ECPRI_ETHERTYPE) {
        ecpri_header_t * ecpri0 = (ecpri_header_t *)(eth0 + 1);
        
        /* Process IQ Data (Type 0x00) or Real-time Control (Type 0x02) */
        if (ecpri0->ecpri_message_type == 0x00) {
            // Forward IQ payload down the fast path
            next0 = ECPRI_NEXT_L2_OUTPUT;
        }
      }

      vlib_validate_buffer_enqueue_x1 (vm, node, next_index, to_next,
                                       n_left_to_next, bi0, next0);
    }
    vlib_put_next_frame (vm, node, next_index, n_left_to_next);
  }
  return frame->n_vectors;
}

VLIB_REGISTER_NODE (ecpri_oran_node) = {
  .name = "oran-ecpri-input",
  .vector_size = sizeof (u32),
  .type = VLIB_NODE_TYPE_INTERNAL,
  .n_next_nodes = ECPRI_N_NEXT,
  .next_nodes = {
    [ECPRI_NEXT_L2_OUTPUT] = "interface-output",
    [ECPRI_NEXT_DROP] = "error-drop",
  },
};
