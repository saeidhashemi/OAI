/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */ 

/*! \file flexran_agent_handler.c
 * \brief FlexRAN agent tx and rx message handler 
 * \author Xenofon Foukas and Navid Nikaein
 * \date 2016
 * \version 0.1
 */

#include "flexran_agent_timer.h"
#include "flexran_agent_defs.h"
#include "flexran_agent_common.h"
#include "flexran_agent_mac.h"
#include "log.h"

#include "assertions.h"

#define NUM_AGENT_LAYER 3

/* Submask Agent */ 

bool agent_check_layer[NUM_AGENT_LAYER];
uint64_t  submask_mac;
uint64_t submask_rrc;
bool c = true;

flexran_agent_message_decoded_callback agent_messages_callback[][3] = {
  {flexran_agent_hello, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_HELLO_MSG*/
  {flexran_agent_echo_reply, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_ECHO_REQUEST_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_ECHO_REPLY_MSG*/ //Must add handler when receiving echo reply
  {flexran_agent_handle_stats, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_STATS_REQUEST_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_STATS_REPLY_MSG*/
  {0, 0, 0}, /*PROTOCOK__FLEXRAN_MESSAGE__MSG_SF_TRIGGER_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_UL_SR_INFO_MSG*/
  {flexran_agent_enb_config_reply, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_ENB_CONFIG_REQUEST_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_ENB_CONFIG_REPLY_MSG*/
  {flexran_agent_ue_config_reply, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_UE_CONFIG_REQUEST_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_UE_CONFIG_REPLY_MSG*/
  {flexran_agent_lc_config_reply, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_LC_CONFIG_REQUEST_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_LC_CONFIG_REPLY_MSG*/
  {flexran_agent_mac_handle_dl_mac_config, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_DL_MAC_CONFIG_MSG*/
  {0, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_UE_STATE_CHANGE_MSG*/
  {flexran_agent_control_delegation, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_CONTROL_DELEGATION_MSG*/
  {flexran_agent_reconfiguration, 0, 0}, /*PROTOCOL__FLEXRAN_MESSAGE__MSG_AGENT_RECONFIGURATION_MSG*/
};

flexran_agent_message_destruction_callback message_destruction_callback[] = {
  flexran_agent_destroy_hello,
  flexran_agent_destroy_echo_request,
  flexran_agent_destroy_echo_reply,
  flexran_agent_mac_destroy_stats_request,
  flexran_agent_mac_destroy_stats_reply,
  flexran_agent_mac_destroy_sf_trigger,
  flexran_agent_mac_destroy_sr_info,
  flexran_agent_destroy_enb_config_request,
  flexran_agent_destroy_enb_config_reply,
  flexran_agent_destroy_ue_config_request,
  flexran_agent_destroy_ue_config_reply,
  flexran_agent_destroy_lc_config_request,
  flexran_agent_destroy_lc_config_reply,
  flexran_agent_mac_destroy_dl_config,
  flexran_agent_destroy_ue_state_change,
  flexran_agent_destroy_control_delegation,
  flexran_agent_destroy_agent_reconfiguration,
};

static const char *flexran_agent_direction2String[] = {
  "", /* not_set  */
  "originating message", /* originating message */
  "successfull outcome", /* successfull outcome */
  "unsuccessfull outcome", /* unsuccessfull outcome */
};


Protocol__FlexranMessage* flexran_agent_handle_message (mid_t mod_id,
							uint8_t *data, 
							uint32_t size){
  
  Protocol__FlexranMessage *decoded_message, *reply_message;
  err_code_t err_code;
  DevAssert(data != NULL);

  if (flexran_agent_deserialize_message(data, size, &decoded_message) < 0) {
    err_code= PROTOCOL__FLEXRAN_ERR__MSG_DECODING;
    goto error; 
  }
  
  
  if ((decoded_message->msg_case > sizeof(agent_messages_callback) / (3 * sizeof(flexran_agent_message_decoded_callback))) || 
      (decoded_message->msg_dir > PROTOCOL__FLEXRAN_DIRECTION__UNSUCCESSFUL_OUTCOME)){

        err_code = PROTOCOL__FLEXRAN_ERR__MSG_NOT_HANDLED;

    goto error;

  }
    printf("==================>   %d    %d  \n", decoded_message->msg_case, decoded_message->msg_dir);
  if (agent_messages_callback[decoded_message->msg_case-1][decoded_message->msg_dir-1] == NULL) {

        err_code= PROTOCOL__FLEXRAN_ERR__MSG_NOT_SUPPORTED;
        goto error;

  }

  err_code = ((*agent_messages_callback[decoded_message->msg_case-1][decoded_message->msg_dir-1])(mod_id, (void *) decoded_message, &reply_message));

  if ( err_code < 0 ){

          goto error;

  } else if (err_code == 1) { //If err_code > 1, we do not want to dispose the message yet

        protocol__flexran_message__free_unpacked(decoded_message, NULL);

  }

  return reply_message;
  
error:

  LOG_E(FLEXRAN_AGENT,"errno %d occured\n",err_code);
  return NULL;

}



void * flexran_agent_pack_message(Protocol__FlexranMessage *msg, 
				  uint32_t * size){

  void * buffer;
  err_code_t err_code = PROTOCOL__FLEXRAN_ERR__NO_ERR;
  
  if (flexran_agent_serialize_message(msg, &buffer, size) < 0 ) {
    err_code = PROTOCOL__FLEXRAN_ERR__MSG_ENCODING;
    goto error;
  }
  
  // free the msg --> later keep this in the data struct and just update the values
  //TODO call proper destroy function
  err_code = ((*message_destruction_callback[msg->msg_case-1])(msg));
  
  DevAssert(buffer !=NULL);
  
  LOG_D(FLEXRAN_AGENT,"Serilized the enb mac stats reply (size %d)\n", *size);
  
  return buffer;
  
 error : 
  LOG_E(FLEXRAN_AGENT,"errno %d occured\n",err_code);
  
  return NULL;   
}

Protocol__FlexranMessage *flexran_agent_handle_timed_task(void *args) {
  err_code_t err_code;
  flexran_agent_timer_args_t *timer_args = (flexran_agent_timer_args_t *) args;

  Protocol__FlexranMessage *timed_task, *reply_message;
  timed_task = timer_args->msg;
  err_code = ((*agent_messages_callback[timed_task->msg_case-1][timed_task->msg_dir-1])(timer_args->mod_id, (void *) timed_task, &reply_message));
  if ( err_code < 0 ){
    goto error;
  }

  return reply_message;
  
 error:
  LOG_E(FLEXRAN_AGENT,"errno %d occured\n",err_code);
  return NULL;
}

Protocol__FlexranMessage* flexran_agent_process_timeout(long timer_id, void* timer_args){
    
  struct flexran_agent_timer_element_s *found = get_timer_entry(timer_id);
 
  if (found == NULL ) goto error;
  LOG_I(FLEXRAN_AGENT, "Found the entry (%p): timer_id is 0x%lx  0x%lx\n", found, timer_id, found->timer_id);
  
  if (timer_args == NULL)
    LOG_W(FLEXRAN_AGENT,"null timer args\n");
  
  return found->cb(timer_args);

 error:
  LOG_E(FLEXRAN_AGENT, "can't get the timer element\n");
  return TIMER_ELEMENT_NOT_FOUND;
}

err_code_t flexran_agent_destroy_flexran_message(Protocol__FlexranMessage *msg) {
  return ((*message_destruction_callback[msg->msg_case-1])(msg));
}


void create_submask(){

   submask_mac = (1 << 16) - 1;
   submask_rrc = submask_mac << 16;


}

void flexran_agent_submask(long data){

    
    create_submask();
    
    if ((data & submask_mac) > 0){

        agent_check_layer[0] = true;
    }
    if((data & submask_rrc) > 0){

        agent_check_layer[1] = true;
    }
    // To be extended


}


/************************
 * \brief flexran_agent_handle_stats
 * \param mod_id, FlexranMessage
 * \return 
 ***********************/



int flexran_agent_handle_stats(mid_t mod_id, const void *params, Protocol__FlexranMessage **msg){


  int i;
  void *buffer;
  int size;
  err_code_t err_code;
  xid_t xid;
  uint32_t usec_interval, sec_interval;

  
  int cc_id = 0;
  int enb_id = mod_id;



  report_config_t report_config;

  uint32_t ue_flags = 0;
  uint32_t c_flags = 0;

  Protocol__FlexranMessage *input = (Protocol__FlexranMessage *)params;

  Protocol__FlexStatsRequest *stats_req = input->stats_request_msg;
  xid = (stats_req->header)->xid;

  

  // Check the type of request that is made
  switch(stats_req->body_case) {


  case PROTOCOL__FLEX_STATS_REQUEST__BODY_COMPLETE_STATS_REQUEST: ;

             
              Protocol__FlexCompleteStatsRequest *comp_req = stats_req->complete_stats_request;

              flexran_agent_submask(comp_req->ue_report_flags);

              if (agent_check_layer[0] && c){

                  AGENT_MAC_xface *mac_agent_xface = (AGENT_MAC_xface *) malloc(sizeof(AGENT_MAC_xface));                                  
                  flexran_agent_register_mac_xface(mod_id, mac_agent_xface);

              }

              if(agent_check_layer[1] && c){

                  AGENT_RRC_xface *rrc_agent_xface = (AGENT_RRC_xface *) malloc(sizeof(AGENT_RRC_xface));
                  flexran_agent_register_rrc_xface(mod_id, rrc_agent_xface);


              }

              c = false;

              if (comp_req->report_frequency == PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_OFF) {
             
                            /*Disable both periodic and continuous updates*/
                            flexran_agent_disable_cont_mac_stats_update(mod_id);
                            flexran_agent_destroy_timer_by_task_id(xid);
                            *msg = NULL;
                            return 0;
              } 
              else { //One-off, periodical or continuous reporting
                            //Set the proper flags
                            ue_flags = comp_req->ue_report_flags;
                            c_flags = comp_req->cell_report_flags;
                            //Create a list of all eNB RNTIs and cells
             
                            //Set the number of UEs and create list with their RNTIs stats configs
                            report_config.nr_ue = flexran_get_num_ues(mod_id); //eNB_UE_list->num_UEs
                            report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t) * report_config.nr_ue);
                            if (report_config.ue_report_type == NULL) {
                                      // TODO: Add appropriate error code
                                      
                                      err_code = -100;
                                      goto error;
                            }

                            for (i = 0; i < report_config.nr_ue; i++) {
 
                                      report_config.ue_report_type[i].ue_rnti = flexran_get_ue_crnti(enb_id, i); //eNB_UE_list->eNB_UE_stats[UE_PCCID(enb_id,i)][i].crnti;
                                      report_config.ue_report_type[i].ue_report_flags = ue_flags;
                            }


                            //Set the number of CCs and create a list with the cell stats configs
                            report_config.nr_cc = MAX_NUM_CCs;
                            report_config.cc_report_type = (cc_report_type_t *) malloc(sizeof(cc_report_type_t) * report_config.nr_cc);
                            if (report_config.cc_report_type == NULL) {
                                      // TODO: Add appropriate error code
                                      err_code = -100;
                                      goto error;
                            }
                            for (i = 0; i < report_config.nr_cc; i++) {
                                    
                                    report_config.cc_report_type[i].cc_id = flexran_get_map_CC_id_rnti_downlink (enb_id, i, report_config.ue_report_type[i].ue_rnti); 
                                    report_config.cc_report_type[i].cc_report_flags = c_flags;
                            }


                            /* Check if request was periodical */
                            if (comp_req->report_frequency == PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_PERIODICAL) {
 
                                        /* Create a one off flexran message as an argument for the periodical task */
                                        Protocol__FlexranMessage *timer_msg;
                                        stats_request_config_t request_config;

                                        request_config.report_type = PROTOCOL__FLEX_STATS_TYPE__FLST_COMPLETE_STATS;
                                        request_config.report_frequency = PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_ONCE;
                                        request_config.period = 0;

                                        /* Need to make sure that the ue flags are saved (Bug) */
                                        // if (report_config.nr_ue == 0) {
                                       //         report_config.nr_ue = 1;
                                       //         report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t));
                                       //          if (report_config.ue_report_type == NULL) {
                                       //                  // TODO: Add appropriate error code
                                       //                  err_code = -100;
                                       //                  goto error;
                                       //          }
                                       //          report_config.ue_report_type[0].ue_rnti = 0; // Dummy value
                                       //          report_config.ue_report_type[0].ue_report_flags = ue_flags;
                                       //   }

                                          request_config.config = &report_config;


                                          flexran_agent_mac_stats_request(enb_id, xid, &request_config, &timer_msg);
                                          /* Create a timer */
                                          long timer_id = 0;
                                          flexran_agent_timer_args_t *timer_args;
                                          timer_args = malloc(sizeof(flexran_agent_timer_args_t));
                                          memset (timer_args, 0, sizeof(flexran_agent_timer_args_t));
                                          timer_args->mod_id = enb_id;
                                          timer_args->msg = timer_msg;
                                          /*Convert subframes to usec time*/
                                          usec_interval = 1000*comp_req->sf;
                                          sec_interval = 0;
                                          /*add seconds if required*/
                                          if (usec_interval >= 1000*1000) {
                                            sec_interval = usec_interval/(1000*1000);
                                            usec_interval = usec_interval%(1000*1000);
                                          }


                                          flexran_agent_create_timer(sec_interval, usec_interval, FLEXRAN_AGENT_DEFAULT, enb_id,
                                           FLEXRAN_AGENT_TIMER_TYPE_PERIODIC, xid,
                                            flexran_agent_handle_timed_task,(void*) timer_args, &timer_id);
                              }


                               else if (comp_req->report_frequency == PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_CONTINUOUS) {
                                        /*If request was for continuous updates, disable the previous configuration and
                                          set up a new one*/
 
                                if (agent_check_layer[0]){
                                          
                                        // flexran_agent_disable_cont_mac_stats_update(mod_id);
                                         flexran_agent_init_cont_mac_stats_update(mod_id);
                                        stats_request_config_t request_config;
                                        request_config.report_type = PROTOCOL__FLEX_STATS_TYPE__FLST_COMPLETE_STATS;
                                        request_config.report_frequency = PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_ONCE;
                                        request_config.period = 0;


                                        /* Need to make sure that the ue flags are saved (Bug) */
                                        if (report_config.nr_ue == 0) {
                                          report_config.nr_ue = 1;
                                          report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t));
                                          if (report_config.ue_report_type == NULL) {
                                            // TODO: Add appropriate error code
                                            err_code = -100;
                                            goto error;
                                          }
                                          report_config.ue_report_type[0].ue_rnti = 0; // Dummy value
                                          report_config.ue_report_type[0].ue_report_flags = ue_flags;
                                        }
                                        request_config.config = &report_config;


                                        flexran_agent_enable_cont_mac_stats_update(enb_id, xid, &request_config);

                                }

                                if (agent_check_layer[1]){
                                        
                                        //flexran_agent_disable_cont_rrc_stats_update(mod_id);
                                         flexran_agent_init_cont_rrc_stats_update(mod_id);
                                        stats_request_config_t request_config;
                                        request_config.report_type = PROTOCOL__FLEX_STATS_TYPE__FLST_COMPLETE_STATS;
                                        request_config.report_frequency = PROTOCOL__FLEX_STATS_REPORT_FREQ__FLSRF_ONCE;
                                        request_config.period = 0;


                                        /* Need to make sure that the ue flags are saved (Bug) */
                                        if (report_config.nr_ue == 0) {
                                          report_config.nr_ue = 1;
                                          report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t));
                                          if (report_config.ue_report_type == NULL) {
                                            // TODO: Add appropriate error code
                                            err_code = -100;
                                            goto error;
                                          }
                                          report_config.ue_report_type[0].ue_rnti = 0; // Dummy value
                                          report_config.ue_report_type[0].ue_report_flags = ue_flags;
                                        }
                                        request_config.config = &report_config;                                        

                                        flexran_agent_enable_cont_rrc_stats_update(enb_id, xid, &request_config);
                                        

                                }

                              }
              }
    break;


  case PROTOCOL__FLEX_STATS_REQUEST__BODY_CELL_STATS_REQUEST:;
            Protocol__FlexCellStatsRequest *cell_req = stats_req->cell_stats_request;
            // UE report config will be blank
 
            report_config.nr_ue = 0;
            report_config.ue_report_type = NULL;
            report_config.nr_cc = cell_req->n_cell;
            report_config.cc_report_type = (cc_report_type_t *) malloc(sizeof(cc_report_type_t) * report_config.nr_cc);
            if (report_config.cc_report_type == NULL) {
                    // TODO: Add appropriate error code
                    err_code = -100;
                    goto error;
            }
            for (i = 0; i < report_config.nr_cc; i++) {
                //TODO: Must fill in the proper cell ids
                    report_config.cc_report_type[i].cc_id = cell_req->cell[i];
                    report_config.cc_report_type[i].cc_report_flags = cell_req->flags;
            }


            break;


  case PROTOCOL__FLEX_STATS_REQUEST__BODY_UE_STATS_REQUEST:;
            Protocol__FlexUeStatsRequest *ue_req = stats_req->ue_stats_request;
            // Cell report config will be blank
 
            report_config.nr_cc = 0;
            report_config.cc_report_type = NULL;
            report_config.nr_ue = ue_req->n_rnti;
            report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t) * report_config.nr_ue);
            if (report_config.ue_report_type == NULL) {
                    // TODO: Add appropriate error code
                    err_code = -100;
                    goto error;
            }
            for (i = 0; i < report_config.nr_ue; i++) {
                    report_config.ue_report_type[i].ue_rnti = ue_req->rnti[i];
                    report_config.ue_report_type[i].ue_report_flags = ue_req->flags;
            }
            break;


  default:
            //TODO: Add appropriate error code
            err_code = -100;
            goto error;

  } // report config 

  // reply part 
  

  if (flexran_agent_stats_reply(enb_id, xid, &report_config, msg )){  
      err_code = PROTOCOL__FLEXRAN_ERR__MSG_BUILD;
      goto error;
    }


  

  free(report_config.ue_report_type);
  free(report_config.cc_report_type);

  return 0;

 error :
  LOG_E(FLEXRAN_AGENT, "errno %d occured\n", err_code);
  return err_code;
}


int flexran_agent_stats_reply(mid_t enb_id, xid_t xid, const report_config_t *report_config, Protocol__FlexranMessage **msg){

  Protocol__FlexHeader *header;
  err_code_t err_code;
  int i;

  if (flexran_create_header(xid, PROTOCOL__FLEX_TYPE__FLPT_STATS_REPLY, &header) != 0)
    goto error;

  
  Protocol__FlexStatsReply *stats_reply_msg;

  stats_reply_msg = malloc(sizeof(Protocol__FlexStatsReply));

  if (stats_reply_msg == NULL)
    goto error;

  protocol__flex_stats_reply__init(stats_reply_msg);
  stats_reply_msg->header = header;

  stats_reply_msg->n_ue_report = report_config->nr_ue;
  stats_reply_msg->n_cell_report = report_config->nr_cc;

  // UE report

  Protocol__FlexUeStatsReport **ue_report;
  

  ue_report = malloc(sizeof(Protocol__FlexUeStatsReport *) * report_config->nr_ue);
          if (ue_report == NULL)
            goto error;
  
  for (i = 0; i < report_config->nr_ue; i++) {

      ue_report[i] = malloc(sizeof(Protocol__FlexUeStatsReport));
      protocol__flex_ue_stats_report__init(ue_report[i]);
      ue_report[i]->rnti = report_config->ue_report_type[i].ue_rnti;
      ue_report[i]->has_rnti = 1;
      ue_report[i]->flags = report_config->ue_report_type[i].ue_report_flags;
      ue_report[i]->has_flags = 1;
  
  }

  // cell rpoert 

  Protocol__FlexCellStatsReport **cell_report;

  
  cell_report = malloc(sizeof(Protocol__FlexCellStatsReport *) * report_config->nr_cc);
  if (cell_report == NULL)
    goto error;
  
  for (i = 0; i < report_config->nr_cc; i++) {

      cell_report[i] = malloc(sizeof(Protocol__FlexCellStatsReport));
      if(cell_report[i] == NULL)
          goto error;

      protocol__flex_cell_stats_report__init(cell_report[i]);
      cell_report[i]->carrier_index = report_config->cc_report_type[i].cc_id;
      cell_report[i]->has_carrier_index = 1;
      cell_report[i]->flags = report_config->cc_report_type[i].cc_report_flags;
      cell_report[i]->has_flags = 1;

  }


if (agent_check_layer[0]){

    if (flexran_agent_mac_stats_reply(enb_id, report_config,  ue_report, cell_report) < 0 ) {
        err_code = PROTOCOL__FLEXRAN_ERR__MSG_BUILD;
        goto error;
    }

  }

  if(agent_check_layer[1]){    

    if (flexran_agent_rrc_stats_reply(enb_id, report_config, ue_report, cell_report) < 0 ){  
      err_code = PROTOCOL__FLEXRAN_ERR__MSG_BUILD;
      goto error;
    }

  }

  stats_reply_msg->cell_report = cell_report;
  stats_reply_msg->ue_report = ue_report;

 *msg = malloc(sizeof(Protocol__FlexranMessage));
  if(*msg == NULL)
    goto error;
  protocol__flexran_message__init(*msg);
  (*msg)->msg_case = PROTOCOL__FLEXRAN_MESSAGE__MSG_STATS_REPLY_MSG;
  (*msg)->msg_dir =  PROTOCOL__FLEXRAN_DIRECTION__SUCCESSFUL_OUTCOME;
  (*msg)->stats_reply_msg = stats_reply_msg;

  return 0;

error :
  LOG_E(FLEXRAN_AGENT, "errno %d occured\n", err_code);
  return err_code;

}

