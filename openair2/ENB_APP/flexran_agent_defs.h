/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2016 Eurecom

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

/*! \file flexran_agent_defs.h
 * \brief FlexRAN agent common definitions 
 * \author Navid Nikaein and Xenofon Foukas and shahab SHARIAT BAGHERI
 * \date 2016
 * \version 0.1
 */
#ifndef FLEXRAN_AGENT_DEFS_H_
#define FLEXRAN_AGENT_DEFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "link_manager.h"
//#include "flexran_agent_common.h"
//#include "flexran_agent_mac_defs.h"
//#include "flexran_agent_rrc_defs.h"
//#include "flexran_agent_defs.h"

#define NUM_MAX_ENB 2
#define NUM_MAX_UE 2048
#define DEFAULT_FLEXRAN_AGENT_IPv4_ADDRESS "127.0.0.1"
#define DEFAULT_FLEXRAN_AGENT_PORT          2210
#define DEFAULT_FLEXRAN_AGENT_CACHE        "/mnt/oai_agent_cache"
#define NUM_AGENT_LAYERS 2
#define SUBMASK_LENGTH_LAYER 16

typedef uint8_t xid_t;  
typedef uint8_t mid_t;  // module or enb id 
typedef uint8_t lcid_t;
typedef int32_t  err_code_t; 

typedef enum {
  
  RAN_LTE_OAI= 0,
  
 /* Max number of states available */
 RAN_NAME_MAX = 0x7f,
} ran_name_t;


/*---------Agent Enums --------- */

typedef enum {
  
  FLEXRAN_AGENT_DEFAULT=0,
  
  FLEXRAN_AGENT_PHY=1,
  FLEXRAN_AGENT_MAC=2,
  FLEXRAN_AGENT_RLC=3,
  FLEXRAN_AGENT_PDCP=4,
  FLEXRAN_AGENT_RRC=5,
  FLEXRAN_AGENT_S1AP=6,
  FLEXRAN_AGENT_GTP=7,
  FLEXRAN_AGENT_X2AP=8,

  FLEXRAN_AGENT_MAX=9,
    
} agent_id_t;


typedef enum {
  /* no action  */
  FLEXRAN_AGENT_ACTION_NONE = 0x0,

  /* send action  */
  FLEXRAN_AGENT_ACTION_SEND = 0x1,

 /* apply action  */
  FLEXRAN_AGENT_ACTION_APPLY = 0x2,

  /* clear action  */
  FLEXRAN_AGENT_ACTION_CLEAR = 0x4,

  /* write action  */
  FLEXRAN_AGENT_ACTION_WRITE = 0x8,

  /* filter action  */
  FLEXRAN_AGENT_ACTION_FILTER = 0x10,

  /* preprocess action  */
  FLEXRAN_AGENT_ACTION_PREPROCESS = 0x20,

  /* meter action  */
  FLEXRAN_AGENT_ACTION_METER = 0x40,
  
  /* Max number of states available */
  FLEXRAN_AGENT_ACTION_MAX = 0x7f,
} flexran_agent_action_t;


/*---------Timer Enums --------- */

typedef enum {
  /* oneshot timer:  */
  FLEXRAN_AGENT_TIMER_TYPE_ONESHOT = 0,

  /* periodic timer  */
  FLEXRAN_AGENT_TIMER_TYPE_PERIODIC = 1,

  /* Inactive state: initial state for any timer. */
  FLEXRAN_AGENT_TIMER_TYPE_EVENT_DRIVEN = 2,
  
  /* Max number of states available */
  FLEXRAN_AGENT_TIMER_TYPE_MAX,
} flexran_agent_timer_type_t;


typedef enum {
  /* Inactive state: initial state for any timer. */
  FLEXRAN_AGENT_TIMER_STATE_INACTIVE = 0x0,

  /* Inactive state: initial state for any timer. */
  FLEXRAN_AGENT_TIMER_STATE_ACTIVE = 0x1,

  /* Inactive state: initial state for any timer. */
  FLEXRAN_AGENT_TIMER_STATE_STOPPED = 0x2,
  
  /* Max number of states available */
  FLEXRAN_AGENT_TIMER_STATE_MAX,
} flexran_agent_timer_state_t;


/* Not Used for the moment ! */

typedef struct {

  uint32_t number;
  uint32_t major;
  uint32_t minor;

  uint32_t protobuf;
} flexran_protocol_version_t;


typedef struct {
  /* general info */ 
 
  /* stats */

  uint32_t total_rx_msg;
  uint32_t total_tx_msg;
   
  uint32_t rx_msg[NUM_MAX_ENB];
  uint32_t tx_msg[NUM_MAX_ENB];

} flexran_agent_info_t;


/* These structs will be used to give
   instructions for the type of stats reports
   we need to create */


typedef struct {
  uint16_t ue_rnti;
  uint32_t ue_report_flags; /* Indicates the report elements
             required for this UE id. See
             FlexRAN specification 1.2.4.2 */
} ue_report_type_t;

typedef struct {
  uint16_t cc_id;
  uint32_t cc_report_flags; /* Indicates the report elements
            required for this CC index. See
            FlexRAN specification 1.2.4.3 */
} cc_report_type_t;

typedef struct {
  int nr_ue;
  ue_report_type_t *ue_report_type;
  int nr_cc;
  cc_report_type_t *cc_report_type;
} report_config_t;

typedef struct stats_request_config_s{
  uint8_t report_type;
  uint8_t report_frequency;
  uint16_t period; /*In number of subframes*/
  report_config_t *config;
} stats_request_config_t;



/* Control Module Capabilities */


typedef struct {
  agent_id_t flexran_agent;
  
  flexran_agent_timer_type_t timer_type;

  flexran_agent_action_t action_type;

} flexran_agent_control_module_t;


typedef struct {
  mid_t enb_id;
   
  flexran_protocol_version_t version;
   
  flexran_agent_info_t agent_info;

  flexran_agent_control_module_t cm_capabilities[FLEXRAN_AGENT_MAX];
 
} flexran_agent_instance_t;



#endif 
