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

/*! \file flexran_agent_rrc_internal.h
 * \brief Implementation specific definitions for the FlexRAN RRC agent
 * \author Shahab SHARIAT BAGHERI
 * \date 2017
 * \version 0.2
 */

#ifndef FLEXRAN_AGENT_RRC_INTERNAL_H_
#define FLEXRAN_AGENT_RRC_INTERNAL_H_

#include <pthread.h>

#include <yaml.h>

#include "flexran_agent_rrc.h"
#include "flexran_agent_common.h"
#include "flexran_agent_defs.h"

/*This will be used for producing continuous status updates for the RRC
 *Needs to be thread-safe
 */
typedef struct {
  /*Flag showing if continuous rrc stats update is enabled*/
  uint8_t is_initialized;
  volatile uint8_t cont_update;
  xid_t xid;
  Protocol__FlexranMessage *stats_req;
  Protocol__FlexranMessage *prev_stats_reply;

  pthread_mutex_t *mutex;
} rrc_stats_updates_context_t;

/*Array holding the last stats reports for each eNB. Used for continuous reporting*/
rrc_stats_updates_context_t rrc_stats_context[NUM_MAX_ENB];

/*Functions to initialize and destroy the struct required for the
 *continuous stats update report*/
err_code_t flexran_agent_init_cont_rrc_stats_update(mid_t mod_id);

err_code_t flexran_agent_destroy_cont_rrc_stats_update(mid_t mod_id);


/*Enable/Disable the continuous stats update service for the RRC*/
err_code_t flexran_agent_enable_cont_rrc_stats_update(mid_t mod_id, xid_t xid,
						  stats_request_config_t *stats_req);

err_code_t flexran_agent_disable_cont_rrc_stats_update(mid_t mod_id);


// /* Functions for parsing the RRC agent policy reconfiguration command */

int parse_rrc_config(mid_t mod_id, yaml_parser_t *parser);



#endif /*FLEXRAN_AGENT_RRC_INTERNAL_H_*/
