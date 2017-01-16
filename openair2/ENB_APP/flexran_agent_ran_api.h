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

/*! \file flexran_agent_common.h
 * \brief common message primitves and utilities 
 * \author Xenofon Foukas, Mohamed Kassem and Navid Nikaein
 * \date 2016
 * \version 0.1
 */


#ifndef FLEXRAN_AGENT_RAN_API_H_
#define FLEXRAN_AGENT_RAN_API_H_

#include <stdio.h>
#include <time.h>

#include "flexran_agent_common.h"
#include "flexran_agent_common_internal.h"
#include "flexran_agent_extern.h"


/*
* get the info from the underlying RAN
*/
#include "enb_config.h"
#include "LAYER2/MAC/extern.h"
#include "LAYER2/RLC/rlc.h"
#include "SCHED/defs.h"
#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "RRC/LITE/rrc_eNB_UE_context.h"
#include "PHY/extern.h"
#include "log.h"


/****************************
 * get generic info from RAN
 ****************************/

void flexran_set_enb_vars(mid_t mod_id, ran_name_t ran);

int flexran_get_current_time_ms (mid_t mod_id, int subframe_flag);

/*Return the current frame number
 *Could be using implementation specific numbering of frames
 */
unsigned int flexran_get_current_frame(mid_t mod_id);

/*Return the current SFN (0-1023)*/ 
unsigned int flexran_get_current_system_frame_num(mid_t mod_id);

unsigned int flexran_get_current_subframe(mid_t mod_id);

/*Return the frame and subframe number in compact 16-bit format.
  Bits 0-3 subframe, rest for frame. Required by FlexRAN protocol*/
uint16_t flexran_get_sfn_sf (mid_t mod_id);

/* Return a future frame and subframe number that is ahead_of_time
   subframes later in compact 16-bit format. Bits 0-3 subframe,
   rest for frame */
uint16_t flexran_get_future_sfn_sf(mid_t mod_id, int ahead_of_time);

/* Return the number of attached UEs */
int flexran_get_num_ues(mid_t mod_id);

/* Get the rnti of a UE with id ue_id */
int flexran_get_ue_crnti (mid_t mod_id, mid_t ue_id);

/* Get the RLC buffer status report of a ue for a designated
   logical channel id */
int flexran_get_ue_bsr (mid_t mod_id, mid_t ue_id, lcid_t lcid);

/* Get power headroom of UE with id ue_id */
int flexran_get_ue_phr (mid_t mod_id, mid_t ue_id);

/* Get the UE wideband CQI */
int flexran_get_ue_wcqi (mid_t mod_id, mid_t ue_id);

/* Get the transmission queue size for a UE with a channel_id logical channel id */
int flexran_get_tx_queue_size(mid_t mod_id, mid_t ue_id, logical_chan_id_t channel_id, char * );

/* Update the timing advance status (find out whether a timing advance command is required) */
int flexran_update_TA(mid_t mod_id, mid_t ue_id, int CC_id);

/* Return timing advance MAC control element for a designated cell and UE */
int flexran_get_MAC_CE_bitmap_TA(mid_t mod_id, mid_t ue_id, int CC_id);

/* MAP CC_id rnti in downlink*/
int  flexran_get_map_CC_id_rnti_downlink (mid_t mod_id, int CC_index, uint16_t ue_rnti);

/* MAP CC_id rnti in uplink*/
int  flexran_get_map_CC_id_rnti_uplink (mid_t mod_id, int CC_index, uint16_t ue_rnti);

/* Get the number of active component carriers for a specific UE */
int flexran_get_active_CC(mid_t mod_id, mid_t ue_id);

/* Get the rank indicator for a designated cell and UE */
int flexran_get_current_RI(mid_t mod_id, mid_t ue_id, int CC_id);

/* See TS 36.213, section 10.1 */
int flexran_get_n1pucch_an(mid_t mod_id, int CC_id);

/* See TS 36.211, section 5.4 */
int flexran_get_nRB_CQI(mid_t mod_id, int CC_id);

/* See TS 36.211, section 5.4 */
int flexran_get_deltaPUCCH_Shift(mid_t mod_id, int CC_id);

/* See TS 36.211, section 5.7.1 */
int flexran_get_prach_ConfigIndex(mid_t mod_id, int CC_id);

/* See TS 36.211, section 5.7.1 */
int flexran_get_prach_FreqOffset(mid_t mod_id, int CC_id);

/* See TS 36.321 */
int flexran_get_maxHARQ_Msg3Tx(mid_t mod_id, int CC_id);

/* Get the length of the UL cyclic prefix */
int flexran_get_ul_cyclic_prefix_length(mid_t mod_id, int CC_id);

/* Get the length of the DL cyclic prefix */
int flexran_get_dl_cyclic_prefix_length(mid_t mod_id, int CC_id);

/* Get the physical cell id of a cell */
int flexran_get_cell_id(mid_t mod_id, int CC_id);

/* See TS 36.211, section 5.5.3.2 */
int flexran_get_srs_BandwidthConfig(mid_t mod_id, int CC_id);

/* See TS 36.211, table 5.5.3.3-1 and 2 */
int flexran_get_srs_SubframeConfig(mid_t mod_id, int CC_id);

/* Boolean value. See TS 36.211,
   section 5.5.3.2. TDD only */
int flexran_get_srs_MaxUpPts(mid_t mod_id, int CC_id);

/* Get number of DL resource blocks */
int flexran_get_N_RB_DL(mid_t mod_id, int CC_id);

/* Get number of UL resource blocks */
int flexran_get_N_RB_UL(mid_t mod_id, int CC_id);

/* Get number of resource block groups */
int flexran_get_N_RBG(mid_t mod_id, int CC_id);

/* Get DL/UL subframe assignment. TDD only */
int flexran_get_subframe_assignment(mid_t mod_id, int CC_id);

/* TDD only. See TS 36.211, table 4.2.1 */
int flexran_get_special_subframe_assignment(mid_t mod_id, int CC_id);

/* Get the duration of the random access response window in subframes */
int flexran_get_ra_ResponseWindowSize(mid_t mod_id, int CC_id);

/* Get timer used for random access */
int flexran_get_mac_ContentionResolutionTimer(mid_t mod_id, int CC_id);

/* Get type of duplex mode (FDD/TDD) */
int flexran_get_duplex_mode(mid_t mod_id, int CC_id);

/* Get the SI window length */
long flexran_get_si_window_length(mid_t mod_id, int CC_id);

/* Get SIB periodicity from RRC typedef*/
int flexran_get_sib_periodicity(mid_t);



/* Get the number of PDCCH symbols configured for the cell */
int flexran_get_num_pdcch_symb(mid_t mod_id, int CC_id);

/* See TS 36.213, sec 5.1.1.1 */
int flexran_get_tpc(mid_t mod_id, mid_t ue_id);

/* Get the first available HARQ process for a specific cell and UE during 
   a designated frame and subframe. Returns 0 for success. The id and the 
   status of the HARQ process are stored in id and status respectively */
int flexran_get_harq(const mid_t mod_id, const uint8_t CC_id, const mid_t ue_id,
		     const int frame, const uint8_t subframe, int *id, int *round);

/* Uplink power control management*/
int flexran_get_p0_pucch_dbm(mid_t mod_id, mid_t ue_id, int CC_id);

int flexran_get_p0_nominal_pucch(mid_t mod_id, int CC_id);

int flexran_get_p0_pucch_status(mid_t mod_id, mid_t ue_id, int CC_id);

int flexran_update_p0_pucch(mid_t mod_id, mid_t ue_id, int CC_id);


/*
 * ************************************
 * Get Messages for UE Configuration Reply
 * ************************************
 */

/* Get timer in subframes. Controls the synchronization
   status of the UE, not the actual timing 
   advance procedure. See TS 36.321 */
int flexran_get_time_alignment_timer(mid_t mod_id, mid_t ue_id);

/* Get measurement gap configuration. See TS 36.133 */
int flexran_get_meas_gap_config(mid_t mod_id, mid_t ue_id);

/* Get measurement gap configuration offset if applicable */
int flexran_get_meas_gap_config_offset(mid_t mod_id, mid_t ue_id);

/* DL aggregated bit-rate of non-gbr bearer
   per UE. See TS 36.413 */
int flexran_get_ue_aggregated_max_bitrate_dl (mid_t mod_id, mid_t ue_id);

/* UL aggregated bit-rate of non-gbr bearer
   per UE. See TS 36.413 */
int flexran_get_ue_aggregated_max_bitrate_ul (mid_t mod_id, mid_t ue_id);

/* Only half-duplex support. FDD
   operation. Boolean value */
int flexran_get_half_duplex(mid_t ue_id);

/* Support of intra-subframe hopping.
   Boolean value */
int flexran_get_intra_sf_hopping(mid_t ue_id);

/* UE support for type 2 hopping with
   n_sb>1 */
int flexran_get_type2_sb_1(mid_t ue_id);

/* Get the UE category */
int flexran_get_ue_category(mid_t ue_id);

/* UE support for resource allocation
   type 1 */
int flexran_get_res_alloc_type1(mid_t ue_id);

/* Get UE transmission mode */
int flexran_get_ue_transmission_mode(mid_t mod_id, mid_t ue_id);

/* Boolean value. See TS 36.321 */
int flexran_get_tti_bundling(mid_t mod_id, mid_t ue_id);

/* The max HARQ retransmission for UL.
   See TS 36.321 */
int flexran_get_maxHARQ_TX(mid_t mod_id, mid_t ue_id);

/* See TS 36.213 */
int flexran_get_beta_offset_ack_index(mid_t mod_id, mid_t ue_id);

/* See TS 36.213 */
int flexran_get_beta_offset_ri_index(mid_t mod_id, mid_t ue_id);

/* See TS 36.213 */
int flexran_get_beta_offset_cqi_index(mid_t mod_id, mid_t ue_id);

/* Boolean. See TS36.213, Section 10.1 */
int flexran_get_simultaneous_ack_nack_cqi(mid_t mod_id, mid_t ue_id);

/* Boolean. See TS 36.213, Section 8.2 */
int flexran_get_ack_nack_simultaneous_trans(mid_t mod_id,mid_t ue_id);

/* Get aperiodic CQI report mode */
int flexran_get_aperiodic_cqi_rep_mode(mid_t mod_id,mid_t ue_id);

/* Get ACK/NACK feedback mode. TDD only */
int flexran_get_tdd_ack_nack_feedback(mid_t mod_id, mid_t ue_id);

/* See TS36.213, section 10.1 */
int flexran_get_ack_nack_repetition_factor(mid_t mod_id, mid_t ue_id);

/* Boolean. Extended buffer status report size */
int flexran_get_extended_bsr_size(mid_t mod_id, mid_t ue_id);

/* Get number of UE transmission antennas */
int flexran_get_ue_transmission_antenna(mid_t mod_id, mid_t ue_id);

/* Get logical channel group of a channel with id lc_id */
int flexran_get_lcg(mid_t ue_id, mid_t lc_id);

/* Get direction of logical channel with id lc_id */
int flexran_get_direction(mid_t ue_id, mid_t lc_id);

/* get the ue imsi for its context*/
int flexran_get_ue_imsi(mid_t mod_id, mid_t ue_id);


int flexran_get_ue_plmn(mid_t mod_id, mid_t ue_id);

int flexran_get_ue_status(mid_t mod_id, mid_t ue_id);

int flexran_get_ue_measgap_config(mid_t mod_id, mid_t ue_id);

int flexran_get_ue_measgap_offset(mid_t mod_id, mid_t ue_id);

int flexran_get_ue_num_bands(mid_t mod_id, mid_t ue_id);

uint64_t* flexran_get_ue_bands(mid_t mod_id, mid_t ue_id);



#endif