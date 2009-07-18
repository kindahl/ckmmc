/*
 * The ckMMC library provides SCSI MMC functionality.
 * Copyright (C) 2006-2009 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file include/ckmmc/scsiparser.hh
 * @brief Defines different structures with parsing functionallity for SCSI command results.
 */

#pragma once
#include <vector>
#include <ckcore/types.hh>
#include "ckmmc/device.hh"

namespace ckmmc
{
	ckcore::tuint16 read_uint16_msbf(unsigned char *buffer);
	ckcore::tuint32 read_uint32_msbf(unsigned char *buffer);
	void write_uint16_msbf(ckcore::tuint16 i,unsigned char *buffer);
	void write_uint32_msbf(ckcore::tuint32 i,unsigned char *buffer);

	/**
	 * @brief Class representing mode page 0x05 data.
	 */
	class ScsiModePage05
	{
	public:
		/**
		 * Defines different write types (modes).
		 */
		enum WriteType
		{
			ckWT_PACKET = 0,
			ckWT_TAO = 1,
			ckWT_SAO = 2,
			ckWT_RAW = 3,
			ckWT_LAYER_JUMP = 4,

			ckWT_AUDIO_MASTER = 8	// Not in official standard.
		};

		/**
		 * Defines different multi-session states.
		 */
		enum MultiSession
		{
			ckMS_NEXT_DISALLOWED_NO_B0 = 0,
			ckMS_NEXT_DISALLOWED_B0 = 1,
			ckMS_NEXT_ALLOWED_B0 = 3
		};

		/**
		 * Defines different data block types.
		 */
		enum DataBlock
		{
			ckDB_RAW_2352 = 0,
			ckDB_RAW_2352_PQ = 1,
			ckDB_RAW_2352_PW_PACK = 2,
			ckDB_RAW_2352_PW = 3,
			ckDB_MODE_1_2048 = 8,
			ckDB_MODE_2_2336 = 9,
			ckDB_MODE_2_XA_FORM_1_2048 = 10,
			ckDB_MODE_2_XA_FORM_1_2056 = 11,
			ckDB_MODE_2_XA_FORM_2_2324 = 12,
			ckDB_MODE_2_XA_MIXED_2332 = 13
		};

		/**
		 * Defines different session formats.
		 */
		enum SessionFormat
		{
			ckSF_CDROM_CDDA = 0,
			ckSF_CD_I = 1,
			ckSF_CDROM_XA = 2
		};

		/**
		 * Defines track mode flags (MMC 2 - table 295).
		 */
		enum TrackModeFlags
		{
			ckTM_AUDIO_2 = 0x00,		// Two channel audio disc.
			ckTM_AUDIO_4 = 0x08,		// Four channel audio disc.
			ckTM_PREEMP = 0x01,			// Audio pre-emphasis flag (for use with ckTM_AUDIO_2 or ckTM_AUDIO_4).

			ckTM_DATA = 0x04,			// Data disc.
			ckTM_INCREMENTAL = 0x01,	// Data incrementa flag (for use with ckTM_DATA).

			ckTM_COPY_ALLOWED = 0x03	// Copying is allowed.
		};

	public:
		unsigned char page_code_;
		bool ps_;
		unsigned char page_len_;
		WriteType write_type_;
		bool test_write_;
		bool ls_v_;
		bool buf_e_;
		unsigned char track_mode_;
		bool copy_;
		bool fp_;
		MultiSession multi_session_;
		DataBlock data_block_type_;
		unsigned char link_size_;
		unsigned char host_app_code_;
		SessionFormat session_format_;
		ckcore::tuint32 packed_size_;
		ckcore::tuint16 audio_pulse_len_;
		unsigned char media_cat_num_[16];
		unsigned char int_std_rec_code_[16];
		unsigned char sub_hdrs_[4];

		bool parse(unsigned char *buffer);
		bool read(unsigned char *buffer,size_t buffer_len);
		void reset_tao();
		void reset_sao();
	};

	/**
	 * @brief Class representing mode page 0x2a data.
	 */
	class ScsiModePage2A
	{
	public:
		unsigned char page_code_;
		bool ps_;
		unsigned char page_len_;
		bool read_cd_r_;
		bool read_cd_rw_;
		bool method_2_;
		bool read_dvd_rom_;
		bool read_dvd_r_;
		bool read_dvd_ram_;
		bool write_cd_r_;
		bool write_cd_rw_;
		bool test_write_;
		bool write_dvd_r_;
		bool write_dvd_ram_;
		bool audio_play_;
		bool composite_;
		bool digital_port_1_;
		bool digital_port_2_;
		bool mode_2_form_1_;
		bool mode_2_form_2_;
		bool multi_session_;
		bool buf_;
		bool cdda_supported_;
		bool ccda_accurate_;
		bool rw_supported_;
		bool rw_deint_corr_;
		bool c2_pointers_;
		bool isrc_;
		bool upc_;
		bool read_bar_code_;
		bool lock_;
		bool lock_state_;
		bool prevent_jumper_;
		bool eject_;
		Device::LoadMechanism load_mechanism_;
		bool sep_chan_vol_;
		bool sep_chan_mute_;
		bool change_disc_prsnt_;
		bool sss_;
		bool change_sides_;
		bool rw_lead_in_;
		ckcore::tuint16 max_read_spd_;	// KB/s.
		ckcore::tuint16 num_vol_lvls_;
		ckcore::tuint16 buf_size_;
		ckcore::tuint16 cur_read_spd_;	// KB/s.
		bool bckf_;
		bool rck_;
		bool lsbf_;
		Device::AudioBlockLen length_;
		ckcore::tuint16 max_write_spd_;	// KB/s.
		ckcore::tuint16 cur_write_spd_;	// KB/s.
		ckcore::tuint16 copy_man_rev_;
		Device::RotCtrl rot_ctrl_;
		std::vector<ckcore::tuint16> write_spds_;	// KB/s.

		bool parse(unsigned char *buffer);
	};

	/**
	 * @brief Class representing inquiry data.
	 */
	class ScsiInquiryData
	{
	public:
		unsigned char perh_dev_type_;
		unsigned char perh_qual_;
		bool rmb_;
		unsigned char version_;
		unsigned char res_data_format_;
		bool hi_sup_;
		bool norm_aca_;
		bool aerc_;
		unsigned char additional_len_;
		bool sccs_;
		bool addr_16_;
		bool mchngr_;
		bool multip_;
		bool vs_1_;
		bool enc_serv_;
		bool bq_ue_;
		bool vs_2_;
		bool cmd_queue_;
		bool linked_;
		bool sync_;
		bool wbus_16_;
		bool rel_addr_;
		char vendor_[9];
		char product_[17];
		char rev_[5];

		bool parse(unsigned char *buffer);
	};

	/**
	 * @brief Class representing configuration data.
	 */
	class ScsiConfigurationData
	{
	public:
		ckcore::tuint32 data_len_;
		Device::Profile cur_profile_;

		bool parse(unsigned char *buffer);
	};
};
