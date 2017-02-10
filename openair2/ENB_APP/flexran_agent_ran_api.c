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

/*! \file flexran_agent_common.c
 * \brief common primitives for all agents 
 * \author Xenofon Foukas, Shahab SHARIAT BAGHERI and Navid Nikaein
 * \date 2016
 * \version 0.1
 */

#include "flexran_agent_ran_api.h"
/*
 * get generic info from RAN
 */
void * enb[NUM_MAX_ENB];
void * enb_ue[NUM_MAX_ENB];
void * enb_rrc[NUM_MAX_ENB];
void * enb_ue_rrc[NUM_MAX_ENB];
rrc_eNB_ue_context_t * enb_ue_context[NUM_MAX_ENB];



void flexran_set_enb_vars(mid_t mod_id, ran_name_t ran){

  switch (ran){
  case RAN_LTE_OAI :
    enb[mod_id] =  (void *)&eNB_mac_inst[mod_id];
    enb_ue[mod_id] = (void *)&eNB_mac_inst[mod_id].UE_list;
    enb_rrc[mod_id] = (void *)&eNB_rrc_inst[mod_id];
    enb_ue_rrc[mod_id]  = (void *)&UE_rrc_inst[mod_id];
    break;
  default :
    goto error;
  }
   
  return; 

 error:
  LOG_E(FLEXRAN_AGENT, "unknown RAN name %d\n", ran);
}

/*
 * Usefull internl APIs functions
 *
  */




int  flexran_get_map_CC_id_rnti_downlink (mid_t mod_id, int CC_index, uint16_t ue_rnti) {

  return CC_id_rnti_downlink(mod_id, CC_index, ue_rnti);
}

int  flexran_get_map_CC_id_rnti_uplink (mid_t mod_id, int CC_index, uint16_t ue_rnti) {

  return ((eNB_MAC_INST *)enb[mod_id])->UE_list.ordered_ULCCids[CC_index][ue_rnti];;
}



int flexran_get_current_time_ms (mid_t mod_id, int subframe_flag){

  if (subframe_flag == 1){
    return ((eNB_MAC_INST *)enb[mod_id])->frame*10 + ((eNB_MAC_INST *)enb[mod_id])->subframe;
  }else {
    return ((eNB_MAC_INST *)enb[mod_id])->frame*10;
  }
   
}

unsigned int flexran_get_current_frame (mid_t mod_id) {

 // #warning "SFN will not be in [0-1023] when oaisim is used"
  return ((eNB_MAC_INST *)enb[mod_id])->frame;
  
}

unsigned int flexran_get_current_system_frame_num(mid_t mod_id) {
  return (flexran_get_current_frame(mod_id) %1024);
}

unsigned int flexran_get_current_subframe (mid_t mod_id) {

  return ((eNB_MAC_INST *)enb[mod_id])->subframe;
  
}

uint16_t flexran_get_sfn_sf (mid_t mod_id) {
  
  frame_t frame;
  sub_frame_t subframe;
  uint16_t sfn_sf, frame_mask, sf_mask;
  
  frame = (frame_t) flexran_get_current_system_frame_num(mod_id);
  subframe = (sub_frame_t) flexran_get_current_subframe(mod_id);
  frame_mask = ((1<<12) - 1);
  sf_mask = ((1<<4) - 1);
  sfn_sf = (subframe & sf_mask) | ((frame & frame_mask) << 4);
  
  return sfn_sf;
}

uint16_t flexran_get_future_sfn_sf (mid_t mod_id, int ahead_of_time) {
  
  frame_t frame;
  sub_frame_t subframe;
  uint16_t sfn_sf, frame_mask, sf_mask;
  
  frame = (frame_t) flexran_get_current_system_frame_num(mod_id);
  subframe = (sub_frame_t) flexran_get_current_subframe(mod_id);

  subframe = ((subframe + ahead_of_time) % 10);

  int full_frames_ahead = ((ahead_of_time / 10) % 10);
  
  frame = frame + full_frames_ahead;

  if (subframe < flexran_get_current_subframe(mod_id)) {
    frame++;
  }

  frame_mask = ((1<<12) - 1);
  sf_mask = ((1<<4) - 1);
  sfn_sf = (subframe & sf_mask) | ((frame & frame_mask) << 4);
  
  return sfn_sf;
}

int flexran_get_num_ues (mid_t mod_id){

  return  ((eNB_MAC_INST *)enb[mod_id])->UE_list.num_UEs;
}

int flexran_get_ue_crnti (mid_t mod_id, mid_t ue_id) {

  return  UE_RNTI(mod_id, ue_id);
}

int flexran_get_ue_bsr (mid_t mod_id, mid_t ue_id, lcid_t lcid) {

  return UE_BSR (mod_id, ue_id, lcid) ;
}

int flexran_get_ue_phr (mid_t mod_id, mid_t ue_id) {

  return ((UE_list_t *)enb_ue[mod_id])->UE_template[UE_PCCID(mod_id,ue_id)][ue_id].phr_info;
}

int flexran_get_ue_wcqi (mid_t mod_id, mid_t ue_id) {
  return ((UE_list_t *)enb_ue[mod_id])->eNB_UE_stats[UE_PCCID(mod_id,ue_id)][ue_id].dl_cqi;
}

int flexran_get_ue_pmi(mid_t mod_id){


}

int flexran_get_tx_queue_size(mid_t mod_id, mid_t ue_id, logical_chan_id_t channel_id, char * buffer_status) {
	rnti_t rnti = flexran_get_ue_crnti(mod_id,ue_id);
	uint16_t frame = (uint16_t) flexran_get_current_frame(mod_id);
	mac_rlc_status_resp_t rlc_status = mac_rlc_status_ind(mod_id,rnti, mod_id,frame,ENB_FLAG_YES,MBMS_FLAG_NO,channel_id,0);

	if (strcmp(buffer_status, "bytes_buffer")){

		return rlc_status.bytes_in_buffer;	
	}
	else if(strcmp(buffer_status, "pdu_buffer"))  {
        
        return rlc_status.pdus_in_buffer;	

	}
	else if (strcmp(buffer_status, "head_line")){

		return rlc_status.head_sdu_remaining_size_to_send;	

	}
	
}

int flexran_update_TA(mid_t mod_id, mid_t ue_id, int CC_id) {
  
  UE_list_t *UE_list=&eNB_mac_inst[mod_id].UE_list;
  UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[ue_id];
  int rnti;

  rnti = flexran_get_ue_crnti(mod_id, ue_id);
  if (ue_sched_ctl->ta_timer == 0) {

    // WE SHOULD PROTECT the eNB_UE_stats with a mutex here ...                                                                         
    LTE_eNB_UE_stats		*eNB_UE_stats = mac_xface->get_eNB_UE_stats(mod_id, CC_id, rnti);
    ue_sched_ctl->ta_timer		      = 20;	// wait 20 subframes before taking TA measurement from PHY                                         
    switch (PHY_vars_eNB_g[mod_id][CC_id]->frame_parms.N_RB_DL) {
    case 6:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update;
      break;

    case 15:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update/2;
      break;

    case 25:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update/4;
      break;

    case 50:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update/8;
      break;

    case 75:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update/12;
      break;

    case 100:
      ue_sched_ctl->ta_update		      = eNB_UE_stats->timing_advance_update/16;
      break;
    }
    // clear the update in case PHY does not have a new measurement after timer expiry                                               
    eNB_UE_stats->timing_advance_update	      = 0;
  }
  else {
    ue_sched_ctl->ta_timer--;
    ue_sched_ctl->ta_update		      = 0;	// don't trigger a timing advance command                                                          
  }

  return ue_sched_ctl->ta_update	      = 0;

}

int flexran_get_MAC_CE_bitmap_TA(mid_t mod_id, mid_t ue_id,int CC_id) {
  
  UE_list_t			*UE_list      = &eNB_mac_inst[mod_id].UE_list;
  UE_sched_ctrl			*ue_sched_ctl = &UE_list->UE_sched_ctrl[ue_id];

  rnti_t rnti = flexran_get_ue_crnti(mod_id,ue_id);
  LTE_eNB_UE_stats *eNB_UE_stats = mac_xface->get_eNB_UE_stats(mod_id,CC_id,rnti);
  
  if (eNB_UE_stats == NULL) {
    return 0;
  }

  if (ue_sched_ctl->ta_update == 0) {
    return 1;
  } else {
    return 0;
  }

}

int flexran_get_active_CC(mid_t mod_id, mid_t ue_id) {
	return ((UE_list_t *)enb_ue[mod_id])->numactiveCCs[ue_id];
}

int flexran_get_current_RI(mid_t mod_id, mid_t ue_id, int CC_id) {
	LTE_eNB_UE_stats	*eNB_UE_stats = NULL;

	int			 pCCid	      = UE_PCCID(mod_id,ue_id);
	rnti_t			 rnti	      = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats			      = mac_xface->get_eNB_UE_stats(mod_id,CC_id,rnti);
	
	if (eNB_UE_stats == NULL) {
	  return 0;
	}

	return eNB_UE_stats[CC_id].rank;
}

int flexran_get_tpc(mid_t mod_id, mid_t ue_id) {
	LTE_eNB_UE_stats	*eNB_UE_stats = NULL;
	int32_t			 normalized_rx_power, target_rx_power;
	int			 tpc	      = 1;

	int			 pCCid	      = UE_PCCID(mod_id,ue_id);
	rnti_t			 rnti	      = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats = mac_xface->get_eNB_UE_stats(mod_id, pCCid, rnti);

	target_rx_power = mac_xface->get_target_pusch_rx_power(mod_id,pCCid);

	if (eNB_UE_stats == NULL) {
	  normalized_rx_power = target_rx_power;
	} else if (eNB_UE_stats->UL_rssi != NULL) {
	  normalized_rx_power = eNB_UE_stats->UL_rssi[0];
	} else {
	  normalized_rx_power = target_rx_power;
	}

	if (normalized_rx_power>(target_rx_power+1)) {
		tpc = 0;	//-1
	} else if (normalized_rx_power<(target_rx_power-1)) {
		tpc = 2;	//+1
	} else {
		tpc = 1;	//0
	}
	return tpc;
}

int flexran_get_harq(const mid_t mod_id, const uint8_t CC_id, const mid_t ue_id, const int frame, const uint8_t subframe, int *id, int *round)	{ //flag_id_status = 0 then id, else status
	/*TODO: Add int TB in function parameters to get the status of the second TB. This can be done to by editing in
	 * get_ue_active_harq_pid function in line 272 file: phy_procedures_lte_eNB.c to add
	 * DLSCH_ptr = PHY_vars_eNB_g[Mod_id][CC_id]->dlsch_eNB[(uint32_t)UE_id][1];*/

  uint8_t harq_pid;
  uint8_t harq_round;
  

  uint16_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

  mac_xface->get_ue_active_harq_pid(mod_id,CC_id,rnti,frame,subframe,&harq_pid,&harq_round,openair_harq_DL);

  *id = harq_pid;
  *round = harq_round;
  /* if (round > 0) { */
  /*   *status = 1; */
  /* } else { */
  /*   *status = 0; */
  /* } */

  /* return 0; */
  return round;
}


int flexran_get_rsrp(mid_t mod_id, mid_t ue_id, int CC_id){

	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	return  mac_xface->get_RSRP(mod_id, CC_id, rnti);
	

}

int flexran_get_rsrq(mid_t mod_id, mid_t ue_id, int CC_id){

	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	return  mac_xface->get_RSRQ(mod_id, CC_id, rnti);
	
}

int flexran_get_Ul_rssi(mid_t mod_id, mid_t ue_id, int CC_id){

    LTE_eNB_UE_stats *eNB_UE_stats = NULL;
	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats =  mac_xface->get_eNB_UE_stats(mod_id, CC_id, rnti);
	
	if (eNB_UE_stats == NULL) {
	  return -1;
	}
		
		return eNB_UE_stats->UL_rssi[0];
	
	


}


int flexran_get_p0_pucch_dbm(mid_t mod_id, mid_t ue_id, int CC_id) {
	LTE_eNB_UE_stats *eNB_UE_stats = NULL;
	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats =  mac_xface->get_eNB_UE_stats(mod_id, CC_id, rnti);
	
	if (eNB_UE_stats == NULL) {
	  return -1;
	}
	
	if(eNB_UE_stats->Po_PUCCH_update == 1) {
		return eNB_UE_stats->Po_PUCCH_dBm;
	}
	else
		return -1;
}

int flexran_get_p0_nominal_pucch(mid_t mod_id, int CC_id) {
	int32_t pucch_rx_received = mac_xface->get_target_pucch_rx_power(mod_id, CC_id);
	return pucch_rx_received;
}

int flexran_get_p0_pucch_status(mid_t mod_id, mid_t ue_id, int CC_id) {
        LTE_eNB_UE_stats *eNB_UE_stats = NULL;
	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats =  mac_xface->get_eNB_UE_stats(mod_id, CC_id, rnti);
	return eNB_UE_stats->Po_PUCCH_update;
}

int flexran_update_p0_pucch(mid_t mod_id, mid_t ue_id, int CC_id) {
          LTE_eNB_UE_stats *eNB_UE_stats = NULL;
	uint32_t rnti = flexran_get_ue_crnti(mod_id,ue_id);

	eNB_UE_stats =  mac_xface->get_eNB_UE_stats(mod_id, CC_id, rnti);
	eNB_UE_stats->Po_PUCCH_update = 0;

	return 0;
}


/*
 * ************************************
 * Get Messages for eNB Configuration Reply
 * ************************************
 */

int flexran_get_hopping_offset(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pusch_config_common.pusch_HoppingOffset;
}

int flexran_get_hopping_mode(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pusch_config_common.hoppingMode;
}

int flexran_get_n_SB(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pusch_config_common.n_SB;
}

int flexran_get_enable64QAM(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pusch_config_common.enable64QAM;
}

int flexran_get_phich_duration(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->phich_config_common.phich_duration;
}

int flexran_get_phich_resource(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	if(frame_parms->phich_config_common.phich_resource == oneSixth)
		return 0;
	else if(frame_parms->phich_config_common.phich_resource == half)
		return 1;
	else if(frame_parms->phich_config_common.phich_resource == one)
		return 2;
	else if(frame_parms->phich_config_common.phich_resource == two)
		return 3;

	return -1;
}

int flexran_get_n1pucch_an(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pucch_config_common.n1PUCCH_AN;
}

int flexran_get_nRB_CQI(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pucch_config_common.nRB_CQI;
}

int flexran_get_deltaPUCCH_Shift(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->pucch_config_common.deltaPUCCH_Shift;
}

int flexran_get_prach_ConfigIndex(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex;
}

int flexran_get_prach_FreqOffset(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->prach_config_common.prach_ConfigInfo.prach_FreqOffset;
}

int flexran_get_maxHARQ_Msg3Tx(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->maxHARQ_Msg3Tx;
}

int flexran_get_ul_cyclic_prefix_length(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->Ncp_UL;
}

int flexran_get_dl_cyclic_prefix_length(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->Ncp;
}

int flexran_get_cell_id(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->Nid_cell;
}

int flexran_get_srs_BandwidthConfig(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->soundingrs_ul_config_common.srs_BandwidthConfig;
}

int flexran_get_srs_SubframeConfig(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->soundingrs_ul_config_common.srs_SubframeConfig;
}

int flexran_get_srs_MaxUpPts(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->soundingrs_ul_config_common.srs_MaxUpPts;
}

int flexran_get_N_RB_DL(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->N_RB_DL;
}

int flexran_get_N_RB_UL(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->N_RB_UL;
}

int flexran_get_N_RBG(mid_t mod_id, int CC_id) {
  	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->N_RBG;
}

int flexran_get_subframe_assignment(mid_t mod_id, int CC_id) {
	
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->tdd_config;
}

int flexran_get_special_subframe_assignment(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->tdd_config_S;
}

int flexran_get_ra_ResponseWindowSize(mid_t mod_id, int CC_id) {
	Enb_properties_array_t *enb_properties;
	enb_properties = enb_config_get();
	return enb_properties->properties[mod_id]->rach_raResponseWindowSize[CC_id];
}

int flexran_get_mac_ContentionResolutionTimer(mid_t mod_id, int CC_id) {
	Enb_properties_array_t *enb_properties;
	enb_properties = enb_config_get();
	return enb_properties->properties[mod_id]->rach_macContentionResolutionTimer[CC_id];
}

int flexran_get_duplex_mode(mid_t mod_id, int CC_id) {
	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	if(frame_parms->frame_type == TDD)
		return PROTOCOL__FLEX_DUPLEX_MODE__FLDM_TDD;
	else if (frame_parms->frame_type == FDD)
		return PROTOCOL__FLEX_DUPLEX_MODE__FLDM_FDD;

	return -1;
}

int flexran_get_antenna_ports(mid_t mod_id, int CC_id){

	LTE_DL_FRAME_PARMS   *frame_parms;

	frame_parms = mac_xface->get_lte_frame_parms(mod_id, CC_id);
	return frame_parms->nb_antenna_ports_eNB;

}



long flexran_get_si_window_length(mid_t mod_id, int CC_id) {
	return  ((eNB_RRC_INST *)enb_rrc[mod_id])->carrier[CC_id].sib1->si_WindowLength;
}

int flexran_get_sib1_length(mid_t mod_id, int CC_id) {
	return  ((eNB_RRC_INST *)enb_rrc[mod_id])->carrier[CC_id].sizeof_SIB1;
}

int flexran_get_sib_periodicity(mid_t mod_id){
   
   int NB_SIG_CNX_UE_temporary = 0; // For The Moment ... (NB_SIG_CNX_UE = 2)

   return ((UE_RRC_INST *)enb_ue_rrc[mod_id])->Info[0].SIperiod;
}


int flexran_get_sib_periodicity_length(mid_t mod_id){

  int NB_SIG_CNX_UE_temporary = 0; // For The Moment ... (NB_SIG_CNX_UE = 2)

  return ((UE_RRC_INST *)enb_ue_rrc[mod_id])->Info[NB_SIG_CNX_UE_temporary].SIwindowsize;  
  
}

int flexran_get_num_mbsfn(mid_t mod_id, mid_t CC_id){

  return ((eNB_RRC_INST *)enb_rrc[mod_id])->carrier[CC_id].num_mbsfn_sync_area;	
}

int flexran_get_num_pdcch_symb(mid_t mod_id, int CC_id) {
  /* TODO: This should return the number of PDCCH symbols initially used by the cell CC_id */
  
    return ((eNB_MAC_INST *) enb[mod_id])->common_channels[CC_id].DCI_pdu.num_pdcch_symbols;
}



/*
 * ************************************
 * Get Messages for UE Configuration Reply
 * ************************************
 */


int flexran_get_time_alignment_timer(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.mac_MainConfig != NULL)
	    return ue_context_p->ue_context.mac_MainConfig->timeAlignmentTimerDedicated;
	}
	else
	  return -1;
}

int flexran_get_meas_gap_config(mid_t mod_id, mid_t ue_id) {
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.measGapConfig != NULL) {
      if(ue_context_p->ue_context.measGapConfig->present == MeasGapConfig_PR_setup) {
	if (ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp0) {
	  return PROTOCOL__FLEX_MEAS_GAP_CONFIG_PATTERN__FLMGCP_GP1;
	} else if (ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp1) {
	  return PROTOCOL__FLEX_MEAS_GAP_CONFIG_PATTERN__FLMGCP_GP2;
	} else {
	  return PROTOCOL__FLEX_MEAS_GAP_CONFIG_PATTERN__FLMGCP_OFF;
	}
      }
    }
  }
  return -1;
}


int flexran_get_meas_gap_config_offset(mid_t mod_id, mid_t ue_id) {
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);
  
  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.measGapConfig != NULL){
      if(ue_context_p->ue_context.measGapConfig->present == MeasGapConfig_PR_setup) {
	if (ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp0) {
	  return ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.choice.gp0;
	} else if (ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.present == MeasGapConfig__setup__gapOffset_PR_gp1) {
	  return ue_context_p->ue_context.measGapConfig->choice.setup.gapOffset.choice.gp0;
	} 
      }
    }
  }
  return -1;
}

int flexran_get_ue_aggregated_max_bitrate_dl (mid_t mod_id, mid_t ue_id) {
	return ((UE_list_t *)enb_ue[mod_id])->UE_sched_ctrl[ue_id].ue_AggregatedMaximumBitrateDL;
}

int flexran_get_ue_aggregated_max_bitrate_ul (mid_t mod_id, mid_t ue_id) {
	return ((UE_list_t *)enb_ue[mod_id])->UE_sched_ctrl[ue_id].ue_AggregatedMaximumBitrateUL;
}

int flexran_get_half_duplex(mid_t ue_id) {
  // TODO
	int halfduplex = 0;
	int bands_to_scan = ((UE_RRC_INST *)enb_ue_rrc[ue_id])->UECap->UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list.count;
	for (int i =0; i < bands_to_scan; i++){
		if(((UE_RRC_INST *)enb_ue_rrc[ue_id])->UECap->UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list.array[i]->halfDuplex > 0)
			halfduplex = 1;
	}
	return halfduplex;
}


int flexran_get_type2_sb_1(mid_t ue_id) {
	//TODO:Get proper value
	uint8_t temp = 0;
	temp = (((UE_RRC_INST *)enb_ue_rrc[ue_id])->UECap->UE_EUTRA_Capability->featureGroupIndicators->buf);
	return (temp & ( 1 << (11)));
}


int flexran_get_res_alloc_type1(mid_t ue_id) {
	//TODO:Get proper value
	uint8_t temp = 0;
	temp = (((UE_RRC_INST *)enb_ue_rrc[ue_id])->UECap->UE_EUTRA_Capability->featureGroupIndicators->buf);
	return (temp & ( 1 << (30)));
}

int flexran_get_ue_transmission_mode(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);

	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->choice.explicitValue.transmissionMode;
	  }
	}
	else
	  return -1;
}

int flexran_get_tti_bundling(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.mac_MainConfig != NULL){
	    return ue_context_p->ue_context.mac_MainConfig->ul_SCH_Config->ttiBundling;
	  }
	}
	else
	  return -1;
}

int flexran_get_maxHARQ_TX(mid_t mod_id, mid_t ue_id) {
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);
  
  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.mac_MainConfig != NULL){
      return *ue_context_p->ue_context.mac_MainConfig->ul_SCH_Config->maxHARQ_Tx;
    }
  }
  return -1;
}

int flexran_get_beta_offset_ack_index(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_ACK_Index;
	  }
	}
	else
	  return -1;
}

int flexran_get_beta_offset_ri_index(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_RI_Index;
	  }
	}
	else
	  return -1;
}

int flexran_get_beta_offset_cqi_index(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_CQI_Index;
	  }
	}
	else
	  return -1;
}

int flexran_get_simultaneous_ack_nack_cqi(mid_t mod_id, mid_t ue_id) {
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);
  
  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
      if (ue_context_p->ue_context.physicalConfigDedicated->cqi_ReportConfig->cqi_ReportPeriodic != NULL) {
	return ue_context_p->ue_context.physicalConfigDedicated->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.simultaneousAckNackAndCQI;
      }
    }
  }
  return -1;
}

int flexran_get_ack_nack_simultaneous_trans(mid_t mod_id,mid_t ue_id) {
	return (&eNB_rrc_inst[mod_id])->carrier[0].sib2->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.setup.ackNackSRS_SimultaneousTransmission;
}

int flexran_get_aperiodic_cqi_rep_mode(mid_t mod_id,mid_t ue_id) {
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);
  
  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
      return *ue_context_p->ue_context.physicalConfigDedicated->cqi_ReportConfig->cqi_ReportModeAperiodic;
    }
  }
  return -1;
}

int flexran_get_tdd_ack_nack_feedback(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);

	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->pucch_ConfigDedicated->tdd_AckNackFeedbackMode;
	  }
	}
	else
	  return -1;
}

int flexran_get_ack_nack_repetition_factor(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    return ue_context_p->ue_context.physicalConfigDedicated->pucch_ConfigDedicated->ackNackRepetition.choice.setup.repetitionFactor;
	  }
	}
	else
	  return -1;
}

int flexran_get_extended_bsr_size(mid_t mod_id, mid_t ue_id) {
  //TODO: need to double check
  struct rrc_eNB_ue_context_s* ue_context_p = NULL;
  uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

  ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);
  if(ue_context_p != NULL) {
    if(ue_context_p->ue_context.mac_MainConfig != NULL){
      if(ue_context_p->ue_context.mac_MainConfig->ext2 != NULL){
	long val = (*(ue_context_p->ue_context.mac_MainConfig->ext2->mac_MainConfig_v1020->extendedBSR_Sizes_r10));
	if (val > 0) {
	  return 1;
	}
      }
    }
  }
  return -1;
}

int flexran_get_ue_transmission_antenna(mid_t mod_id, mid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = flexran_get_ue_crnti(mod_id,ue_id);

	ue_context_p = rrc_eNB_get_ue_context(&eNB_rrc_inst[mod_id],rntiP);

	if(ue_context_p != NULL) {
	  if(ue_context_p->ue_context.physicalConfigDedicated != NULL){
	    if(ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->choice.explicitValue.ue_TransmitAntennaSelection.choice.setup == AntennaInfoDedicated__ue_TransmitAntennaSelection__setup_closedLoop)
	      return 2;
	    else if(ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->choice.explicitValue.ue_TransmitAntennaSelection.choice.setup == AntennaInfoDedicated__ue_TransmitAntennaSelection__setup_openLoop)
	      return 1;
	    else
	      return 0;
	  }
	}
	else
	  return -1;
}

int flexran_get_lcg(mid_t ue_id, mid_t lc_id) {
  if (UE_mac_inst == NULL) {
    return -1;
  }
  if(UE_mac_inst[ue_id].logicalChannelConfig[lc_id] != NULL) {
    return *UE_mac_inst[ue_id].logicalChannelConfig[lc_id]->ul_SpecificParameters->logicalChannelGroup;
  } else {
    return -1;
  }
}

int flexran_get_direction(mid_t ue_id, mid_t lc_id) {
	/*TODO: fill with the value for the rest of LCID*/
  if(lc_id == DCCH | lc_id == DCCH1)
    return 2;
  else if(lc_id == DTCH)
    return 1;
}

// New added, Morely RRC facilities

int flexran_get_measId(mid_t ue_id){

	int CNX_UE = 0; // NB_CNX_UE  2
	int MEAS_ID = 0; //  MAX_MEAS_ID 6

	return ((UE_RRC_INST *)enb_ue_rrc[ue_id])->measReportList[CNX_UE][MEAS_ID]->measId;

}



int flexran_get_ue_imsi(mid_t mod_id, mid_t ue_id){

 // uint32_t rnti = flexran_get_ue_crnti(mod_id, ue_id);
 // enb_ue_context[mod_id]= flexran_agent_get_ue_context (mod_id, rnti);
    
 // if (enb_ue_context[mod_id] != NULL) {
 //   return enb_ue_context[mod_id]->ue_context.ue_imsi; 
 // }
 // return -1;
}

int flexran_get_ue_plmn(mid_t mod_id, mid_t ue_id){

// enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

//  if ((enb_ue_context[mod_id] != NULL)) {
//    return enb_ue_context[mod_id]>ue_context.plmn_id; 
//  }
//  return -1;
}

int flexran_get_ue_status(mid_t mod_id, mid_t ue_id){

 //  enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

 // if ((&enb_ue_context[mod_id] != NULL) && (&enb_ue_context[mod_id]->ue_context != NULL)) {
 //   return enb_ue_context[mod_id]>ue_context.Status; 
 // }
 // return -1;
}

int flexran_get_ue_measgap_config(mid_t mod_id, mid_t ue_id){

 //  enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

 // if(enb_ue_context[mod_id] != NULL &&
 //    enb_ue_context[mod_id]->ue_context.measGapConfig != NULL &&
 //    enb_ue_context[mod_id]->ue_context.measGapConfig->present == MeasGapConfig_PR_setup) {
 //    if (enb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.present ==
 //                  MeasGapConfig__setup__gapOffset_PR_gp0) {
 //      return MEAS_GAP_PATTERN__MGP_GP0;
 //    } else if (enb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.present ==
 //                  MeasGapConfig__setup__gapOffset_PR_gp1) {
 //      return MEAS_GAP_PATTERN__MGP_GP1;
 //    } else {
 //      return MEAS_GAP_PATTERN__MGP_OFF;
 //    }
 //  }
 //  return -1;
}

int flexran_get_ue_measgap_offset(mid_t mod_id, mid_t ue_id){

 //  enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

 // if(enb_ue_context[mod_id] != NULL &&
 //    enb_ue_context[mod_id]->ue_context.measGapConfig != NULL &&
 //    enb_ue_context[mod_id]->ue_context.measGapConfig->present == MeasGapConfig_PR_setup) {
 //    if (enb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.present ==
 //                  MeasGapConfig__setup__gapOffset_PR_gp0) {
 //      return enb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.choice.gp0;
 //    } else if (enb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.present ==
 //                  MeasGapConfig__setup__gapOffset_PR_gp1) {
 //      return nb_ue_context[mod_id]->ue_context.measGapConfig->choice.setup.gapOffset.choice.gp;
 //    } 
 //  }
 //  return -1;
}

int flexran_get_ue_num_bands(mid_t mod_id, mid_t ue_id){

 //  enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

 // if(enb_ue_context[mod_id] != NULL &&
 //    enb_ue_context[mod_id]->ue_context.eutra_Capability != NULL) {
 //      return enb_ue_context[mod_id]->ue_context.eutra_Capability->rf_Parameters.supportedBandListEUTRA.list.count;
 //  }
 //  return -1;
}

uint64_t* flexran_get_ue_bands(mid_t mod_id, mid_t ue_id){

 //  int i;
 //  int num_bands=0;
 //  uint64_t *rf_bands;
 //  enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

 // if(enb_ue_context[mod_id] != NULL &&
 //    enb_ue_context[mod_id]->ue_context.eutra_Capability != NULL) {
 //    num_bands = flexran_get_ue_num_bands(mod_id, ue_id);
 //    rf_bands = (uint64_t) malloc (num_bands * sizeof(uint64_t));
 //    for (i=0; i <num_bands; i++){
 //      rf_band[i]= enb_ue_context[mod_id]->ue_context.eutra_Capability->rf_Parameters.supportedBandListEUTRA.list.array[i]->bandEUTRA;                                bandEUTRA
 //    }
 //   return rf_bands; 
 //  }
 //  return -1;
}

uint32_t * flexran_get_ue_duplex_mode(mid_t mod_id, mid_t ue_id) {
  
  // int i;
  // uint32_t *duplex_mode;
  // int num_bands = 0;
  // enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

  // if(enb_ue_context[mod_id] != NULL &&
  //   enb_ue_context[mod_id]->ue_context.eutra_Capability != NULL) {
  //   num_bands = flexran_get_ue_num_bands(mod_id, ue_id);
  //   duplex_mode = (uint32_t) malloc (num_bands * sizeof(uint32_t));
  //   for (i =0; i < num_bands; i++){
  //     if(enb_ue_context[mod_id]->UECap->UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list.array[i]->halfDuplex > 0){
  //       duplex_mode[i] = 0;
  //     }
  //     else {
  //       duplex_mode[i] = 1;
  //     }
  // } 
  // return duplex_mode;
}


// TODO --------------


struct rrc_eNB_ue_context_s* flexran_agent_get_ue_context (mid_t mod_id, uint32_t rnti) {
  return rrc_eNB_get_ue_context( (eNB_RRC_INST *)enb_rrc[mod_id], rnti);
}


// int flexran_get_ue_category(mid_t mod_id, mid_t ue_id){

//  //enb_ue_context[mod_id]= flexran_agent_get_ue_context (enb[mod_id], ue_id);

//  if(enb_ue_context[mod_id] != NULL &&
//     enb_ue_context[mod_id]->ue_context.eutra_Capability != NULL) {
//     return enb_ue_context[mod_id]->ue_context.UE_EUTRA_Capability->ue_Category;
//   }
//   return -1;
// }


int flexran_get_intra_sf_hopping(mid_t ue_id) {
  //TODO:Get proper value
  //temp = (((UE_RRC_INST *)enb_ue_rrc[ue_id])->UECap->UE_EUTRA_Capability->featureGroupIndicators->buf);
  //return (0 & ( 1 << (31)));
}

