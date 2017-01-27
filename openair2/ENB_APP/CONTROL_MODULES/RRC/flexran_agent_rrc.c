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
 * \brief FlexRAN agent Control Module RRC
 * \author shahab SHARIAT BAGHERI and Navid Nikaein
 * \date 2016
 * \version 0.1
 */

#include "flexran_agent_rrc.h"
#include "flexran_agent_extern.h"
#include "flexran_agent_common.h"
#include "flexran_agent_rrc_internal.h"


#include "liblfds700.h"

#include "log.h"


/*Flags showing if a rrc agent has already been registered*/
unsigned int rrc_agent_registered[NUM_MAX_ENB];

/*Array containing the Agent-RRC interfaces*/
AGENT_RRC_xface *agent_rrc_xface[NUM_MAX_ENB];

/* Ringbuffer related structs used for maintaining the dl rrc config messages */
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


/******************************************************
 *Implementations of flexran_agent_rrc_internal.h functions
 ******************************************************/

err_code_t flexran_agent_init_cont_rrc_stats_update(mid_t mod_id) {

  /*Initialize the Rrc stats update structure*/
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
  /*Disable the continuous updates for the RRC*/
  rrc_stats_context[mod_id].cont_update = 0;
  rrc_stats_context[mod_id].is_initialized = 0;
  flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].stats_req);
  flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].prev_stats_reply);
  free(rrc_stats_context[mod_id].mutex);

  rrc_agent_registered[mod_id] = NULL;
  return 1;
}


err_code_t flexran_agent_enable_cont_rrc_stats_update(mid_t mod_id,
						      xid_t xid, stats_request_config_t *stats_req) {
  /*Enable the continuous updates for the RRC*/
  if (pthread_mutex_lock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }

  Protocol__FlexranMessage *req_msg;
 
  flexran_agent_rrc_stats_request(mod_id, xid, stats_req, &req_msg);

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




int flexran_agent_rrc_stats_request(mid_t mod_id,
            xid_t xid,
            const stats_request_config_t *report_config,
            Protocol__FlexranMessage **msg) {
  Protocol__FlexHeader *header;
  int i;

  if (flexran_create_header(xid, PROTOCOL__FLEX_TYPE__FLPT_STATS_REQUEST, &header) != 0)
    goto error;

  Protocol__FlexStatsRequest *stats_request_msg;
  stats_request_msg = malloc(sizeof(Protocol__FlexStatsRequest));
  if(stats_request_msg == NULL)
    goto error;

  protocol__flex_stats_request__init(stats_request_msg);
  stats_request_msg->header = header;

  stats_request_msg->type = report_config->report_type;
  stats_request_msg->has_type = 1;

  switch (report_config->report_type) {

  case PROTOCOL__FLEX_STATS_TYPE__FLST_COMPLETE_STATS:

            stats_request_msg->body_case =  PROTOCOL__FLEX_STATS_REQUEST__BODY_COMPLETE_STATS_REQUEST;
            Protocol__FlexCompleteStatsRequest *complete_stats;
            complete_stats = malloc(sizeof(Protocol__FlexCompleteStatsRequest));

            if(complete_stats == NULL)
              goto error;
            protocol__flex_complete_stats_request__init(complete_stats);
            complete_stats->report_frequency = report_config->report_frequency;
            complete_stats->has_report_frequency = 1;
            complete_stats->sf = report_config->period;
            complete_stats->has_sf = 1;
            complete_stats->has_cell_report_flags = 1;
            complete_stats->has_ue_report_flags = 1;
            // TODO :flags for each report types 
            if (report_config->config->nr_cc > 0) {
                  complete_stats->cell_report_flags = report_config->config->cc_report_type[0].cc_report_flags;
            }
            if (report_config->config->nr_ue > 0) {
                  complete_stats->ue_report_flags = report_config->config->ue_report_type[0].ue_report_flags;
            }

            stats_request_msg->complete_stats_request = complete_stats;

            break;

  case  PROTOCOL__FLEX_STATS_TYPE__FLST_CELL_STATS:
            stats_request_msg->body_case = PROTOCOL__FLEX_STATS_REQUEST__BODY_CELL_STATS_REQUEST;
             Protocol__FlexCellStatsRequest *cell_stats;
             cell_stats = malloc(sizeof(Protocol__FlexCellStatsRequest));

            if(cell_stats == NULL)
              goto error;

            protocol__flex_cell_stats_request__init(cell_stats);
            cell_stats->n_cell = report_config->config->nr_cc;
            cell_stats->has_flags = 1;
            if (cell_stats->n_cell > 0) {
                    uint32_t *cells;
                    cells = (uint32_t *) malloc(sizeof(uint32_t)*cell_stats->n_cell);
                    for (i = 0; i < cell_stats->n_cell; i++) {
                cells[i] = report_config->config->cc_report_type[i].cc_id;
                    }
                    cell_stats->cell = cells;
                    cell_stats->flags = report_config->config->cc_report_type[i].cc_report_flags;
            }
            stats_request_msg->cell_stats_request = cell_stats;
            break;

  case PROTOCOL__FLEX_STATS_TYPE__FLST_UE_STATS:
          stats_request_msg->body_case = PROTOCOL__FLEX_STATS_REQUEST__BODY_UE_STATS_REQUEST;
           Protocol__FlexUeStatsRequest *ue_stats;
           ue_stats = malloc(sizeof(Protocol__FlexUeStatsRequest));
          if(ue_stats == NULL)
            goto error;
          protocol__flex_ue_stats_request__init(ue_stats);
          ue_stats->n_rnti = report_config->config->nr_ue;
          ue_stats->has_flags = 1;
          if (ue_stats->n_rnti > 0) {
            uint32_t *ues;
            ues = (uint32_t *) malloc(sizeof(uint32_t)*ue_stats->n_rnti);
            for (i = 0; i < ue_stats->n_rnti; i++) {
        ues[i] = report_config->config->ue_report_type[i].ue_rnti;
            }
            ue_stats->rnti = ues;
            ue_stats->flags = report_config->config->ue_report_type[i].ue_report_flags;
          }
          stats_request_msg->ue_stats_request = ue_stats;
          break;
  default:
        goto error;


  }

  *msg = malloc(sizeof(Protocol__FlexranMessage));
  if(*msg == NULL)
    goto error;
  protocol__flexran_message__init(*msg);
  
  (*msg)->msg_dir = PROTOCOL__FLEXRAN_DIRECTION__INITIATING_MESSAGE;
  (*msg)->msg_case = PROTOCOL__FLEXRAN_MESSAGE__MSG_STATS_REQUEST_MSG;
  (*msg)->stats_request_msg = stats_request_msg;
  return 0;

 error:
  // TODO: Need to make proper error handling
  if (header != NULL)
    free(header);
  if (stats_request_msg != NULL)
    free(stats_request_msg);
  if(*msg != NULL)
    free(*msg);
  //LOG_E(RRC, "%s: an error occured\n", __FUNCTION__);
  return -1;
}

int flexran_agent_rrc_stats_reply(mid_t mod_id,        
          const report_config_t *report_config,
          Protocol__FlexUeStatsReport **ue_report,
          Protocol__FlexCellStatsReport **cell_report) {


  int i, j, k;
  int cc_id = 0;
  int enb_id = mod_id;
  

  /* Allocate memory for list of UE reports */
  if (report_config->nr_ue > 0) {

          for (i = 0; i < report_config->nr_ue; i++) {

                /* Check the types of reports that need to be constructed based on flag values */

                             
                 
                     if (report_config->ue_report_type[i].ue_report_flags & PROTOCOL__FLEX_UE_STATS_TYPE__FLUST_RRC_MEASUREMENTS) {
                            //TODO: Fill in the actual paging buffer status report. For this field to be valid, the RNTI
                            
                            Protocol__FlexRrcMeasurements *rrc_measurements;
                            rrc_measurements = malloc(sizeof(Protocol__FlexRrcMeasurements));
                            if (rrc_measurements == NULL)
                              goto error;

                            protocol__flex_rrc_measurements__init(rrc_measurements);
                            
                            rrc_measurements->measid = 1 ; //flexran_get_measId(i); Fore the Moment ...
                            rrc_measurements->has_measid = 1;

                            rrc_measurements->pcell_rsrp = flexran_get_rsrp(0, 0, 0); // Should be changed in side ...
                            rrc_measurements->has_pcell_rsrp = 1;

                            rrc_measurements->pcell_rsrq = flexran_get_rsrq(0, 0, 0); // Should be changed inside ...                          
                            rrc_measurements->has_pcell_rsrq = 1 ;
                            //Provide a report for each pending paging message
                            Protocol__FlexNeighCellsMeasurements *n_meas;
                            n_meas = malloc(sizeof(Protocol__FlexNeighCellsMeasurements *));
                            if (n_meas == NULL)
                              goto error;

                            
                                                      
                            ue_report[i]->rrc_measurements = rrc_measurements;
                    }


                            
             }       

      
         
     } 

  /* Allocate memory for list of cell reports */
  if (report_config->nr_cc > 0) {
    
            // Fill in the Cell reports
            for (i = 0; i < report_config->nr_cc; i++) {

                      /* Check flag for creation of noise and interference report */
                      if(report_config->cc_report_type[i].cc_report_flags & PROTOCOL__FLEX_CELL_STATS_TYPE__FLCST_NOISE_INTERFERENCE) {
                            // TODO: Fill in the actual noise and interference report for this cell
                            Protocol__FlexNoiseInterferenceReport *ni_report;
                            ni_report = malloc(sizeof(Protocol__FlexNoiseInterferenceReport));
                            if(ni_report == NULL)
                              goto error;
                            protocol__flex_noise_interference_report__init(ni_report);
                            // Current frame and subframe number
                            ni_report->sfn_sf = flexran_get_sfn_sf(enb_id);
                            ni_report->has_sfn_sf = 1;
                            //TODO:Received interference power in dbm
                            ni_report->rip = 0;
                            ni_report->has_rip = 1;
                            //TODO:Thermal noise power in dbm
                            ni_report->tnp = 0;
                            ni_report->has_tnp = 1;

                            ni_report->p0_nominal_pucch = flexran_get_p0_nominal_pucch(enb_id, 0);
                            ni_report->has_p0_nominal_pucch = 1;
                            cell_report[i]->noise_inter_report = ni_report;
                      }
            }
            /* Add list of all cell reports to the message */

            
  }

  return 0;

 error:

     if (cell_report != NULL)
        free(cell_report);
     if (ue_report != NULL)
        free(ue_report);

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
  LOG_E(FLEXRAN_AGENT, "rrc_stats_context for eNB %d is not initialized\n", mod_id);
  return -1;

}


void flexran_agent_send_update_rrc_stats(mid_t mod_id) {

  Protocol__FlexranMessage *current_report = NULL, *msg;
  void *data;
  int size;
  err_code_t err_code;
  int priority;

  rrc_stats_updates_context_t stats_context = rrc_stats_context[mod_id];
  
  if (pthread_mutex_lock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }

  if (rrc_stats_context[mod_id].cont_update == 1) {
  
    /*Create a fresh report with the required flags*/
    err_code = flexran_agent_handle_stats(mod_id, (void *) rrc_stats_context[mod_id].stats_req, &current_report);
    if (err_code < 0) {
      goto error;
    }
  }
  /* /\*TODO:Check if a previous reports exists and if yes, generate a report */
  /*  *that is the diff between the old and the new report, */
  /*  *respecting the thresholds. Otherwise send the new report*\/ */
  /* if (rrc_stats_context[mod_id].prev_stats_reply != NULL) { */

  /*   msg = flexran_agent_generate_diff_rrc_stats_report(current_report, rrc_stats_context[mod_id].prev_stats_reply); */

  /*   /\*Destroy the old stats*\/ */
  /*    flexran_agent_destroy_flexran_message(rrc_stats_context[mod_id].prev_stats_reply); */
  /* } */
  /* /\*Use the current report for future comparissons*\/ */
  /* rrc_stats_context[mod_id].prev_stats_reply = current_report; */


  if (pthread_mutex_unlock(rrc_stats_context[mod_id].mutex)) {
    goto error;
  }

  if (current_report != NULL){
    data = flexran_agent_pack_message(current_report, &size);
    /*Send any stats updates using the RRC channel of the eNB*/
    if (flexran_agent_msg_send(mod_id, FLEXRAN_AGENT_DEFAULT, data, size, priority)) {
      err_code = PROTOCOL__FLEXRAN_ERR__MSG_ENQUEUING;
      goto error;
    }

    LOG_D(FLEXRAN_AGENT,"sent message with size %d\n", size);
    return;
  }
 error:
  LOG_D(FLEXRAN_AGENT, "Could not send sf trigger message\n");
}



int flexran_agent_ue_state_change(mid_t mod_id, uint32_t rnti, uint8_t state_change) {
  int size;
  Protocol__FlexranMessage *msg;
  Protocol__FlexHeader *header;
  void *data;
  int priority;
  err_code_t err_code;

  int xid = 0;

  if (flexran_create_header(xid, PROTOCOL__FLEX_TYPE__FLPT_UE_STATE_CHANGE, &header) != 0)
    goto error;

  Protocol__FlexUeStateChange *ue_state_change_msg;
  ue_state_change_msg = malloc(sizeof(Protocol__FlexUeStateChange));
  if(ue_state_change_msg == NULL) {
    goto error;
  }
  protocol__flex_ue_state_change__init(ue_state_change_msg);
  ue_state_change_msg->has_type = 1;
  ue_state_change_msg->type = state_change;

  Protocol__FlexUeConfig *config;
  config = malloc(sizeof(Protocol__FlexUeConfig));
  if (config == NULL) {
    goto error;
  }
  protocol__flex_ue_config__init(config);
  if (state_change == PROTOCOL__FLEX_UE_STATE_CHANGE_TYPE__FLUESC_DEACTIVATED) {
    // Simply set the rnti of the UE
    config->has_rnti = 1;
    config->rnti = rnti;
  } else if (state_change == PROTOCOL__FLEX_UE_STATE_CHANGE_TYPE__FLUESC_UPDATED
       || state_change == PROTOCOL__FLEX_UE_STATE_CHANGE_TYPE__FLUESC_ACTIVATED) {
        int i = find_UE_id(mod_id, rnti);
      config->has_rnti = 1;
      config->rnti = rnti;
        if(flexran_get_time_alignment_timer(mod_id,i) != -1) {
          config->time_alignment_timer = flexran_get_time_alignment_timer(mod_id,i);
          config->has_time_alignment_timer = 1;
        }
        if(flexran_get_meas_gap_config(mod_id,i) != -1){
          config->meas_gap_config_pattern = flexran_get_meas_gap_config(mod_id,i);
            config->has_meas_gap_config_pattern = 1;
        }
        if(config->has_meas_gap_config_pattern == 1 &&
         config->meas_gap_config_pattern != PROTOCOL__FLEX_MEAS_GAP_CONFIG_PATTERN__FLMGCP_OFF) {
        config->meas_gap_config_sf_offset = flexran_get_meas_gap_config_offset(mod_id,i);
        config->has_meas_gap_config_sf_offset = 1;
        }
        //TODO: Set the SPS configuration (Optional)
        //Not supported for now, so we do not set it

        //TODO: Set the SR configuration (Optional)
        //We do not set it for now

        //TODO: Set the CQI configuration (Optional)
        //We do not set it for now
      
      if(flexran_get_ue_transmission_mode(mod_id,i) != -1) {
          config->transmission_mode = flexran_get_ue_transmission_mode(mod_id,i);
          config->has_transmission_mode = 1;
        }

      config->ue_aggregated_max_bitrate_ul = flexran_get_ue_aggregated_max_bitrate_ul(mod_id,i);
        config->has_ue_aggregated_max_bitrate_ul = 1;

      config->ue_aggregated_max_bitrate_dl = flexran_get_ue_aggregated_max_bitrate_dl(mod_id,i);
        config->has_ue_aggregated_max_bitrate_dl = 1;

        //TODO: Set the UE capabilities
        Protocol__FlexUeCapabilities *c_capabilities;
        c_capabilities = malloc(sizeof(Protocol__FlexUeCapabilities));
        protocol__flex_ue_capabilities__init(c_capabilities);
        //TODO: Set half duplex (FDD operation)
        c_capabilities->has_half_duplex = 0;
        c_capabilities->half_duplex = 1;//flexran_get_half_duplex(i);
        //TODO: Set intra-frame hopping flag
        c_capabilities->has_intra_sf_hopping = 0;
        c_capabilities->intra_sf_hopping = 1;//flexran_get_intra_sf_hopping(i);
        //TODO: Set support for type 2 hopping with n_sb > 1
        c_capabilities->has_type2_sb_1 = 0;
        c_capabilities->type2_sb_1 = 1;//flexran_get_type2_sb_1(i);
        //TODO: Set ue category
        c_capabilities->has_ue_category = 0;
        c_capabilities->ue_category = 1;//flexran_get_ue_category(i);
        //TODO: Set UE support for resource allocation type 1
        c_capabilities->has_res_alloc_type1 = 0;
        c_capabilities->res_alloc_type1 = 1;//flexran_get_res_alloc_type1(i);
        //Set the capabilites to the message
        config->capabilities = c_capabilities;
      
        if(flexran_get_ue_transmission_antenna(mod_id,i) != -1) {
        config->has_ue_transmission_antenna = 1;
        config->ue_transmission_antenna = flexran_get_ue_transmission_antenna(mod_id,i);
        }

        if(flexran_get_tti_bundling(mod_id,i) != -1) {
        config->has_tti_bundling = 1;
        config->tti_bundling = flexran_get_tti_bundling(mod_id,i);
        }

        if(flexran_get_maxHARQ_TX(mod_id,i) != -1){
        config->has_max_harq_tx = 1;
        config->max_harq_tx = flexran_get_maxHARQ_TX(mod_id,i);
        }

        if(flexran_get_beta_offset_ack_index(mod_id,i) != -1) {
        config->has_beta_offset_ack_index = 1;
        config->beta_offset_ack_index = flexran_get_beta_offset_ack_index(mod_id,i);
        }

        if(flexran_get_beta_offset_ri_index(mod_id,i) != -1) {
        config->has_beta_offset_ri_index = 1;
        config->beta_offset_ri_index = flexran_get_beta_offset_ri_index(mod_id,i);
        }

        if(flexran_get_beta_offset_cqi_index(mod_id,i) != -1) {
        config->has_beta_offset_cqi_index = 1;
        config->beta_offset_cqi_index = flexran_get_beta_offset_cqi_index(mod_id,i);
        }

        if(flexran_get_ack_nack_simultaneous_trans(mod_id,i) != -1) {
        config->has_ack_nack_simultaneous_trans = 1;
        config->ack_nack_simultaneous_trans = flexran_get_ack_nack_simultaneous_trans(mod_id,i);
        }

        if(flexran_get_simultaneous_ack_nack_cqi(mod_id,i) != -1) {
        config->has_simultaneous_ack_nack_cqi = 1;
        config->simultaneous_ack_nack_cqi = flexran_get_simultaneous_ack_nack_cqi(mod_id,i);
        }

        if(flexran_get_aperiodic_cqi_rep_mode(mod_id,i) != -1) {
        config->has_aperiodic_cqi_rep_mode = 1;
        int mode = flexran_get_aperiodic_cqi_rep_mode(mod_id,i);
        if (mode > 4) {
          config->aperiodic_cqi_rep_mode = PROTOCOL__FLEX_APERIODIC_CQI_REPORT_MODE__FLACRM_NONE;
        } else {
          config->aperiodic_cqi_rep_mode = mode;
        }
        }

        if(flexran_get_tdd_ack_nack_feedback(mod_id, i) != -1) {
        config->has_tdd_ack_nack_feedback = 1;
        config->tdd_ack_nack_feedback = flexran_get_tdd_ack_nack_feedback(mod_id,i);
        }

        if(flexran_get_ack_nack_repetition_factor(mod_id, i) != -1) {
        config->has_ack_nack_repetition_factor = 1;
        config->ack_nack_repetition_factor = flexran_get_ack_nack_repetition_factor(mod_id,i);
        }

        if(flexran_get_extended_bsr_size(mod_id, i) != -1) {
        config->has_extended_bsr_size = 1;
        config->extended_bsr_size = flexran_get_extended_bsr_size(mod_id,i);
        }

      config->has_pcell_carrier_index = 1;
      config->pcell_carrier_index = UE_PCCID(mod_id, i);
        //TODO: Set carrier aggregation support (boolean)
        config->has_ca_support = 0;
        config->ca_support = 0;
        if(config->has_ca_support){
        //TODO: Set cross carrier scheduling support (boolean)
        config->has_cross_carrier_sched_support = 1;
        config->cross_carrier_sched_support = 0;
        //TODO: Set secondary cells configuration
        // We do not set it for now. No carrier aggregation support
        
        //TODO: Set deactivation timer for secondary cell
        config->has_scell_deactivation_timer = 0;
        config->scell_deactivation_timer = 0;
        }
  } else if (state_change == PROTOCOL__FLEX_UE_STATE_CHANGE_TYPE__FLUESC_MOVED) {
    // TODO: Not supported for now. Leave blank
  }

  ue_state_change_msg->config = config;
  msg = malloc(sizeof(Protocol__FlexranMessage));
  if (msg == NULL) {
    goto error;
  }
  protocol__flexran_message__init(msg);
  msg->msg_case = PROTOCOL__FLEXRAN_MESSAGE__MSG_UE_STATE_CHANGE_MSG;
  msg->msg_dir = PROTOCOL__FLEXRAN_DIRECTION__INITIATING_MESSAGE;
  msg->ue_state_change_msg = ue_state_change_msg;

  data = flexran_agent_pack_message(msg, &size);
  /*Send sr info using the MAC channel of the eNB*/
  if (flexran_agent_msg_send(mod_id, FLEXRAN_AGENT_DEFAULT, data, size, priority)) {
    err_code = PROTOCOL__FLEXRAN_ERR__MSG_ENQUEUING;
    goto error;
  }

  LOG_D(FLEXRAN_AGENT,"sent message with size %d\n", size);
  return;
 error:
  LOG_D(FLEXRAN_AGENT, "Could not send UE state message\n");
}



int flexran_agent_destroy_ue_state_change(Protocol__FlexranMessage *msg) {
  if(msg->msg_case != PROTOCOL__FLEXRAN_MESSAGE__MSG_UE_STATE_CHANGE_MSG)
    goto error;
  free(msg->ue_state_change_msg->header);
  //TODO: Free the contents of the UE config structure
  free(msg->ue_state_change_msg);
  free(msg);
  return 0;

 error:
  //LOG_E(MAC, "%s: an error occured\n", __FUNCTION__);
  return -1;
}


int flexran_agent_register_rrc_xface(mid_t mod_id, AGENT_RRC_xface *xface) {
  if (rrc_agent_registered[mod_id]) {
    LOG_E(RRC, "RRC agent for eNB %d is already registered\n", mod_id);
    return -1;
  }

  xface->flexran_agent_send_update_rrc_stats = flexran_agent_send_update_rrc_stats;
  
  xface->flexran_agent_notify_ue_state_change = flexran_agent_ue_state_change;
  

  rrc_agent_registered[mod_id] = 1;
  agent_rrc_xface[mod_id] = xface;

  return 0;
}

int flexran_agent_unregister_rrc_xface(mid_t mod_id, AGENT_RRC_xface *xface) {

  //xface->agent_ctxt = NULL;
  xface->flexran_agent_send_update_rrc_stats = NULL;

  xface->flexran_agent_notify_ue_state_change = NULL;

  rrc_agent_registered[mod_id] = 0;
  agent_rrc_xface[mod_id] = NULL;

  return 0;
}
