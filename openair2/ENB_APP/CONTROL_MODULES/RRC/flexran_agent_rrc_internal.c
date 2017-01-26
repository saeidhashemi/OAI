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

/*! \file flexran_agent_rrc_internal.c
 * \brief Helper functions for the RRC agent
 * \author Shahab SHARIAT BAGHERI
 * \date 2017
 * \version 0.2
 */

#include <string.h>
#include <dlfcn.h>

#include "flexran_agent_rrc_internal.h"

// Protocol__FlexranMessage * flexran_agent_generate_diff_rrc_stats_report(Protocol__FlexranMessage *new_message,
// 									Protocol__FlexranMessage *old_message) {

//   int i, j;
  
//   Protocol__FlexStatsReply *old_report, *new_report;

//   Protocol__FlexStatsReply *stats_reply_msg = NULL;
//   Protocol__FlexranMessage *msg = NULL;
  
//   Protocol__FlexUeStatsReport **ue_report;
//   Protocol__FlexUeStatsReport *tmp_ue_report[NUM_MAX_UE];
//   Protocol__FlexCellStatsReport **cell_report;
//   Protocol__FlexCellStatsReport *tmp_cell_report[NUM_MAX_UE];
  
//   old_report = old_message->stats_reply_msg;
//   new_report = new_message->stats_reply_msg;

//   /*Flags to designate changes in various levels of the message*/
//   int stats_had_changes = 0;
//   int ue_had_change = 0;
//   int cell_had_change = 0;

//   /*See how many and which UE reports should be included in the final stats message*/
//   int n_ue_report = 0;
//   int ue_found = 0;
  
//   /*Go through each RNTI of the new report and see if it exists in the old one*/
//   for (i = 0; i < new_report->n_ue_report; i++) {

//         for (j = 0; j < old_report->n_ue_report; j++) {
//              if (new_report->ue_report[i]->rnti == old_report->ue_report[j]->rnti) {
//                   	ue_found = 1;
//                 	/*Need to check if something changed*/
//                 	if (compare_ue_stats_reports(new_report->ue_report[i], old_report->ue_report[j]) != 0) {
//                     	  tmp_ue_report[n_ue_report] = copy_ue_stats_report(new_report->ue_report[i]);
//                     	  n_ue_report++;
//                 	}
//                 	break;
//              }
//         }
        
//         if (!ue_found) {
//                 tmp_ue_report[n_ue_report] = copy_ue_stats_report(new_report->ue_report[i]);
//                 n_ue_report++;
//         }

//         ue_found = 0;
//   }

//   /*See how many and which cell reports should be included in the final stats message*/
//   int n_cell_report = 0;
//   int cell_found = 0;
  
//   /*Go through each cell of the new report and see if it exists in the old one*/
//   for (i = 0; i < new_report->n_cell_report; i++) {
//     for (j = 0; j < old_report->n_cell_report; j++) {
//      if (new_report->cell_report[i]->carrier_index == old_report->cell_report[j]->carrier_index) {
//   	cell_found = 1;
// 	/*Need to check if something changed*/
// 	if (compare_cell_stats_reports(new_report->cell_report[i], old_report->cell_report[j]) != 0) {
// 	  tmp_cell_report[n_cell_report] = copy_cell_stats_report(new_report->cell_report[i]);
// 	  n_cell_report++;
// 	}
// 	break;
//      }
//     }
//     if (!cell_found) {
//       tmp_cell_report[n_cell_report] = copy_cell_stats_report(new_report->cell_report[i]);
//       n_cell_report++;
//     }
//     cell_found = 0;
//   }

//   /*TODO: create the reply message based on the findings*/
//   /*Create ue report list*/
//   if (n_ue_report > 0) {
//     ue_report = malloc(sizeof(Protocol__FlexUeStatsReport *));
//     for (i = 0; i<n_ue_report; i++) {
//       ue_report[i] = tmp_ue_report[i];
//     }
//   }

//   /*Create cell report list*/
//   if (n_cell_report > 0) {
//     cell_report = malloc(sizeof(Protocol__FlexCellStatsReport *));
//     for (i = 0; i<n_cell_report; i++) {
//       cell_report[i] = tmp_cell_report[i];
//     }
//   }
  
//   if (n_cell_report > 0 || n_ue_report > 0) {
//     /*Create header*/
//     int xid = old_report->header->xid;
//     Protocol__FlexHeader *header;
//     if (flexran_create_header(xid, PROTOCOL__FLEX_TYPE__FLPT_STATS_REPLY, &header) != 0) {
//     goto error;
//     }
//     stats_reply_msg = malloc(sizeof(Protocol__FlexStatsReply));
//     protocol__flex_stats_reply__init(stats_reply_msg);
//     stats_reply_msg->header = header;
//     stats_reply_msg->n_ue_report = n_ue_report;
//     stats_reply_msg->ue_report = ue_report;
//     stats_reply_msg->n_cell_report = n_cell_report;
//     stats_reply_msg->cell_report = cell_report;
//     msg = malloc(sizeof(Protocol__FlexranMessage));
//     if(msg == NULL)
//       goto error;
//     protocol__flexran_message__init(msg);
//     msg->msg_case = PROTOCOL__FLEXRAN_MESSAGE__MSG_STATS_REPLY_MSG;
//     msg->msg_dir = PROTOCOL__FLEXRAN_DIRECTION__SUCCESSFUL_OUTCOME;
//     msg->stats_reply_msg = stats_reply_msg;
//   }
//   return msg;
  
//  error:
//    return NULL;
// }




int parse_rrc_config(mid_t mod_id, yaml_parser_t *parser) {
  
  yaml_event_t event;
  
  int done = 0;

  int sequence_started = 0;
  int mapping_started = 0;

  while (!done) {

    if (!yaml_parser_parse(parser, &event))
      goto error;
   
    switch (event.type) {

    case YAML_SEQUENCE_START_EVENT:

          LOG_D(ENB_APP, "A sequence just started as expected\n");
          sequence_started = 1;
          break;

    case YAML_SEQUENCE_END_EVENT:

          LOG_D(ENB_APP, "A sequence ended\n");
          sequence_started = 0;
          break;

    case YAML_MAPPING_START_EVENT:

          if (!sequence_started) {
               goto error;
          }
          LOG_D(ENB_APP, "A mapping started\n");
          mapping_started = 1;
          break;

    case YAML_MAPPING_END_EVENT:

      if (!mapping_started) {
          goto error;
      }

      LOG_D(ENB_APP, "A mapping ended\n");
      mapping_started = 0;
      break;

    case YAML_SCALAR_EVENT:

      if (!mapping_started) {
         goto error;
      }
      // Check the types of subsystems offered and handle their values accordingly
      if (strcmp(event.data.scalar.value, "dl_scheduler") == 0) {

          LOG_D(ENB_APP, "This is for the dl_scheduler subsystem\n");
          // Call the proper handler
          if (parse_dl_scheduler_config(mod_id, parser) == -1) {
                LOG_D(ENB_APP, "An error occured\n");
                goto error;
          }

      } else if (strcmp(event.data.scalar.value, "ul_scheduler") == 0) {
          // Call the proper handler
          LOG_D(ENB_APP, "This is for the ul_scheduler subsystem\n");
          goto error;
          // TODO
      } else if (strcmp(event.data.scalar.value, "ra_scheduler") == 0) {

  // Call the proper handler
  // TODO
      } else if (strcmp(event.data.scalar.value, "page_scheduler") == 0) {
  // Call the proper handler
  // TODO
      } else {
        // Unknown subsystem
        goto error;
      }

      break;

    default: // We expect nothing else at this level of the hierarchy
      goto error;
    }
   
    done = (event.type == YAML_SEQUENCE_END_EVENT);

    yaml_event_delete(&event);
 
  }
  
  return 0;

  error:
  yaml_event_delete(&event);
  return -1;

}
