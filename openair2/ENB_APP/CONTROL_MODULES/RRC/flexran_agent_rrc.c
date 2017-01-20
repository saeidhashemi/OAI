/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@lists.eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/

/*! \file flexran_agent_rrc.c
 * \brief FlexRAN agent message handler for MAC layer
 * \author Navid Nikaein
 * \date 2016
 * \version 0.1
 */

#include "flexran_agent_mac.h"
#include "flexran_agent_extern.h"
#include "flexran_agent_common.h"
#include "flexran_agent_mac_internal.h"

#include "LAYER2/MAC/proto.h"
#include "LAYER2/MAC/flexran_agent_mac_proto.h"
#include "LAYER2/MAC/flexran_agent_scheduler_dlsch_ue_remote.h"

#include "liblfds700.h"

#include "log.h"


/*Flags showing if a mac agent has already been registered*/
unsigned int rrc_agent_registered[NUM_MAX_ENB];

/*Array containing the Agent-MAC interfaces*/
AGENT_RRC_xface *agent_rrc_xface[NUM_MAX_ENB];

/* Ringbuffer related structs used for maintaining the dl mac config messages */
//message_queue_t *rrc_dl_config_queue;
struct lfds700_misc_prng_state rrc_ps[NUM_MAX_ENB];
struct lfds700_ringbuffer_element *rrc_dl_config_array[NUM_MAX_ENB];
struct lfds700_ringbuffer_state rrc_ringbuffer_state[NUM_MAX_ENB];



void flexran_agent_init_rrc_agent(mid_t mod_id) {
  lfds700_misc_library_init_valid_on_current_logical_core();
  lfds700_misc_prng_init(&rrc_ps[mod_id]);
  int num_elements = RINGBUFFER_SIZE + 1;
  //Allow RINGBUFFER_SIZE messages to be stored in the ringbuffer at any time
  rrc_dl_config_array[mod_id] = malloc( sizeof(struct lfds700_ringbuffer_element) *  num_elements);
  lfds700_ringbuffer_init_valid_on_current_logical_core( &rrc_ringbuffer_state[mod_id], rrc_dl_config_array[mod_id], num_elements, &rrc_ps[mod_id], NULL );
}


int flexran_agent_register_rrc_xface(mid_t mod_id, AGENT_RRC_xface *xface) {
  if (rrc_agent_registered[mod_id]) {
    LOG_W(MAC, "RRC agent for eNB %d is already registered\n", mod_id);
    return -1;
  }
  
  xface->flexran_agent_get_ue_context = flexran_agent_get_ue_context;
  
  xface->dl_scheduler_loaded_lib = NULL;

  mac_agent_registered[mod_id] = 1;
  agent_mac_xface[mod_id] = xface;

  return 0;
}

int flexran_agent_unregister_rrc_xface(mid_t mod_id, AGENT_RRC_xface *xface) {

  xface->flexran_agent_get_ue_context = NULL;
  
  xface->dl_scheduler_loaded_lib = NULL;

  rrc_agent_registered[mod_id] = 0;
  agent_rrc_xface[mod_id] = NULL;

  return 0;
}


/******************************************************
 *Implementations of flexran_agent_mac_internal.h functions
 ******************************************************/

err_code_t flexran_agent_init_cont_rrc_stats_update(mid_t mod_id) {

  /*Initialize the Mac stats update structure*/
  /*Initially the continuous update is set to false*/
  rrc_stats_context[mod_id].cont_update = 0;
  rrc_stats_context[mod_id].is_initialized = 1;
  rrc_stats_context[mod_id].stats_req = NULL;
  rrc_stats_context[mod_id].prev_stats_reply = NULL;
  rrc_stats_context[mod_id].mutex = calloc(1, sizeof(pthread_mutex_t));
  if (rrc_stats_context[mod_id].mutex == NULL)
    goto error;
  if (pthread_mutex_init(rrc_stats_context[mod_id].mutex, NULL))
    goto error;;

  return 0;

 error:
  return -1;
}

err_code_t flexran_agent_destroy_cont_rrc_stats_update(mid_t mod_id) {
  /*Disable the continuous updates for the MAC*/
  rrc_stats_context[mod_id].cont_update = 0;
  rrc_stats_context[mod_id].is_initialized = 0;
  flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].stats_req);
  flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].prev_stats_reply);
  free(mac_stats_context[mod_id].mutex);

  rrc_agent_registered[mod_id] = NULL;
  return 1;
}


err_code_t flexran_agent_enable_cont_rrc_stats_update(mid_t mod_id,
						      xid_t xid, stats_request_config_t *stats_req) {
  /*Enable the continuous updates for the MAC*/
  if (pthread_mutex_lock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }

  Protocol__FlexranMessage *req_msg;
 //--------------
//  TODO ...
//---------------
  //flexran_agent_mac_stats_request(mod_id, xid, stats_req, &req_msg);
  rrc_stats_context[mod_id].stats_req = req_msg;
  rrc_stats_context[mod_id].prev_stats_reply = NULL;

  rrc_stats_context[mod_id].cont_update = 1;
  rrc_stats_context[mod_id].xid = xid;

  if (pthread_mutex_unlock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }
  return 0;

 error:
  LOG_E(FLEXRAN_AGENT, "rrc_stats_context for eNB %d is not initialized\n", mod_id);
  return -1;
}

err_code_t flexran_agent_disable_cont_rrc_stats_update(mid_t mod_id) {
  /*Disable the continuous updates for the rrc*/
  if (pthread_mutex_lock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }
  rrc_stats_context[mod_id].cont_update = 0;
  rrc_stats_context[mod_id].xid = 0;
  if (rrc_stats_context[mod_id].stats_req != NULL) {
    flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].stats_req);
  }
  if (rrc_stats_context[mod_id].prev_stats_reply != NULL) {
    flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].prev_stats_reply);
  }
  if (pthread_mutex_unlock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }
  return 0;

 error:
  LOG_E(FLEXRAN_AGENT, "mac_stats_context for eNB %d is not initialized\n", mod_id);
  return -1;

}
