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
 * @file include/ckmmc/mmcdevice.hh
 * @brief Defines the MMC device class.
 */

#pragma once
#include <vector>
#include <ckcore/types.hh>
#include "ckmmc/scsidevice.hh"

namespace ckmmc
{
	class MmcDevice : public ScsiDevice
    {
	public:
		/**
		 * Defines MMC commands.
		 */
		enum
		{
			ckCMD_INQUIRY = 0x12,
			ckCMD_READ_CAPACITY = 0x25,
			ckCMD_READ_TOC_PMA_ATIP = 0x43,
			ckCMD_GET_CONFIGURATION = 0x46,
			ckCMD_READ_DISC_INFORMATION = 0x51,
			ckCMD_READ_DISC_STRUCTURE = 0xad,
			ckCMD_READ_FORMAT_CAPACITIES = 0x23,
			ckCMD_FORMAT_UNIT = 0x04,
			ckCMD_GET_EVENT_STATUS_NOTIFICATION = 0x4a,
			ckCMD_TEST_UNIT_READY = 0x00,
			ckCMD_START_STOP_UNIT = 0x1b,
			ckCMD_CLOSE_TRACK_SESSION = 0x5b,
			ckCMD_PREVENTALLOW_MEDIUM_REMOVAL = 0x1e,
			ckCMD_GET_PERFORMANCE = 0xac,
			ckCMD_SET_CD_SPEED = 0xbb,
			ckCMD_BLANK = 0xa1,
			ckCMD_MODE_SENSE10 = 0x5a,
			ckCMD_MODE_SELECT10 = 0x55,
			ckCMD_REQUEST_SENSE = 0x03,
			ckCMD_READ_CD = 0xbe,
			ckCMD_READ_TRACK_INFORMATION = 0x52
		};

		/**
		 * Defines disc profiles.
		 */
		enum Profile
		{
			ckPROFILE_NONE = 0x0000,
			ckPROFILE_NONREMOVABLE = 0x0001,
			ckPROFILE_REMOVABLE = 0x0002,
			ckPROFILE_MOPTIC_E = 0x0003,
			ckPROFILE_OPTIC_WO = 0x0004,
			ckPROFILE_AS_MO = 0x0005,
			ckPROFILE_CDROM = 0x0008,
			ckPROFILE_CDR = 0x0009,
			ckPROFILE_CDRW = 0x000a,
			ckPROFILE_DVDROM = 0x0010,
			ckPROFILE_DVDMINUSR_SEQ = 0x0011,
			ckPROFILE_DVDRAM = 0x0012,
			ckPROFILE_DVDMINUSRW_RESTOV = 0x0013,
			ckPROFILE_DVDMINUSRW_SEQ = 0x0014,
			ckPROFILE_DVDMINUSR_DL_SEQ = 0x0015,
			ckPROFILE_DVDMINUSR_DL_JUMP = 0x0016,
			ckPROFILE_DVDPLUSRW = 0x001a,
			ckPROFILE_DVDPLUSR = 0x001b,
			//ckPROFILE_DDCDROM = 0x0020,
			//ckPROFILE_DDCDR = 0x0021,
			//ckPROFILE_DDCDRW = 0x0022,
			ckPROFILE_DVDPLUSRW_DL = 0x002a,
			ckPROFILE_DVDPLUSR_DL = 0x002b,
			ckPROFILE_BDROM = 0x0040,
			ckPROFILE_BDR_SRM = 0x0041,
			ckPROFILE_BDR_RRM = 0x0042,
			ckPROFILE_BDRE = 0x0043,
			ckPROFILE_HDDVDROM = 0x0050,
			ckPROFILE_HDDVDR = 0x0051,
			ckPROFILE_HDDVDRAM = 0x0052,
			ckPROFILE_NONSTANDARD = 0xffff
		};

		/**
		 * Defines device features.
		 */
		enum Feature
		{
			// Media features.
			ckDEVICE_READ_CDR,
			ckDEVICE_READ_CDRW,
			ckDEVICE_READ_DVDROM,
			ckDEVICE_READ_DVDR,
			ckDEVICE_READ_DVDRAM,
			ckDEVICE_WRITE_CDR,
			ckDEVICE_WRITE_CDRW,
			ckDEVICE_WRITE_DVDR,
			ckDEVICE_WRITE_DVDRAM,

			// Other features.
			ckDEVICE_TEST_WRITE,
			ckDEVICE_AUDIO_PLAY,
			ckDEVICE_COMPOSITE,
			ckDEVICE_DIGITAL_PORT_1,
			ckDEVICE_DIGITAL_PORT_2,
			ckDEVICE_MODE_2_FORM_1,
			ckDEVICE_MODE_2_FORM_2,
			ckDEVICE_MULTI_SESSION,
			ckDEVICE_BUP,
			ckDEVICE_CDDA_SUPPORTED,
			ckDEVICE_CDDA_ACCURATE,
			ckDEVICE_RW_SUPPORTED,
			ckDEVICE_RW_DEINT_CORR,
			ckDEVICE_C2_POINTERS,
			ckDEVICE_ISRC,
			ckDEVICE_UPC,
			ckDEVICE_READ_BAR_CODE,
			ckDEVICE_LOCK,
			ckDEVICE_LOCK_STATE,
			ckDEVICE_PREVENT_JUMPER,
			ckDEVICE_EJECT,
			ckDEVICE_SEP_CHAN_VOL,
			ckDEVICE_SEP_CHAN_MUTE,
			ckDEVICE_CHANGE_DISC_PRSNT,
			ckDEVICE_SSS,
			ckDEVICE_CHANGE_SIDES,
			ckDEVICE_RW_LEAD_IN,
			ckDEVICE_BCKF,
			ckDEVICE_RCK,
			ckDEVICE_LSBF,

			// Vendor specific.
			ckDEVICE_AUDIO_MASTER,
			ckDEVICE_FORCE_SPEED,
			ckDEVICE_VARIREC,

			ckINTERNAL_NUM_FEAT
		};

		/**
		 * Defines device properties.
		 */
		enum Property
		{
			ckPROP_NUM_VOL_LVLS,	// ckcore::tuint32
			ckPROP_BUFFER_SIZE,		// ckcore::tuint32 (in KB)
			ckPROP_COPY_MGMT_REV,	// ckcore::tuint32
			ckPROP_LOAD_MECHANISM,	// LoadMechanism
			ckPROP_ROT_CTRL,		// RotCtrl
			ckPROP_DA_BLOCK_LEN,	// AudioBlockLen
			ckPROP_MAX_READ_SPD,	// ckcore::tuint32 (sectors per second)
			ckPROP_CUR_READ_SPD,	// ckcore::tuint32 (sectors per second)
			ckPROP_MAX_WRITE_SPD,	// ckcore::tuint32 (sectors per second)
			ckPROP_CUR_WRITE_SPD,	// ckcore::tuint32 (sectors per second)

			// Has no function, is used for keeping track of the number of defined
			// properties.
			ckPROP_INTERNAL_COUNT
		};

		/**
		 * Defines different load mechanisms.
		 */
		enum LoadMechanism
		{
			ckLM_CADDY = 0x00,
			ckLM_TRAY = 0x01,
			ckLM_POPUP = 0x02,
			ckLM_CHANGER_INDIVIDUAL = 0x04,
			ckLM_CHANGER_MAGAZINE = 0x05
		};

		/**
		 * Defines different rotation control modes.
		 */
		enum RotCtrl
		{
			ckRC_CLV = 0x00,
			ckRC_CAV = 0x01
		};

		/**
		 * Defines block lengths of digital audio.
		 */
		enum AudioBlockLen
		{
			ckABL_32 = 0x00,
			ckABL_16 = 0x01,
			ckABL_24 = 0x02,
			ckABL_24I2S = 0x03
		};

		/**
		 * Defines transport modes.
		 */
		enum TransportMode
		{
			ckTM_UNSPECIFIED,
			ckTM_READ,
			ckTM_WRITE
		};

		/**
		 * Defines write modes.
		 */
		enum WriteMode
		{
			ckWM_PACKET,
			ckWM_TAO,
			ckWM_SAO,
			ckWM_RAW16,
			ckWM_RAW96P,
			ckWM_RAW96R,
			ckWM_LAYER_JUMP,

			ckWM_INTERNAL_COUNT
		};

	protected:
		ckcore::tchar vendor_[9];
		ckcore::tchar identifier_[17];
		ckcore::tchar revision_[5];

		ckcore::tuint16 write_modes_;
		ckcore::tuint64 features_;
		ckcore::tuint32 properties_[ckPROP_INTERNAL_COUNT];

		std::vector<ckcore::tuint32> read_speeds_;	// Used for caching read speeds.
		std::vector<ckcore::tuint32> write_speeds_;	// Used for caching write speeds.

		bool is_yamaha() const;
		bool is_plextor() const;

	public:
		MmcDevice(const Address &addr);
		virtual ~MmcDevice();

		const ckcore::tchar *vendor() const;
		const ckcore::tchar *identifier() const;
		const ckcore::tchar *revision() const;

		const std::vector<ckcore::tuint32> &read_speeds();
		const std::vector<ckcore::tuint32> &write_speeds();

		ckcore::tuint32 property(Property prop) const;
		bool recorder();
		bool support(Feature feature);
		bool support(WriteMode mode);
		bool refresh();
		Profile profile();

		/*
		 * Strongly MMC Related Functions.
		 */
		bool inquiry(unsigned char *buffer,ckcore::tuint16 buffer_len);
		bool get_configuration(unsigned char *buffer,
							   ckcore::tuint16 buffer_len);
		bool mode_sense(unsigned char page_code,unsigned char *buffer,
						ckcore::tuint16 buffer_len);
		bool mode_select(unsigned char *buffer,ckcore::tuint16 buffer_len,
						 bool save_page,bool page_format);		
    };
};
