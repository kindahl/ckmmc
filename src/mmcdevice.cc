/*
 * The ckMMC library provides SCSI MMC functionality.
 * Copyright (C) 2006-2011 Christian Kindahl
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

#include <ckcore/log.hh>
#include <ckcore/string.hh>
#include "ckmmc/scsisilencer.hh"
#include "ckmmc/mmc.hh"
#include "ckmmc/mmcdevice.hh"

namespace ckmmc
{
	/**
	 * Constructs a MmcDevice object.
	 */
	MmcDevice::MmcDevice(const Address &addr) : ScsiDevice(addr),write_modes_(0),features_(0)
	{
		memset(properties_,0,sizeof(properties_));

		vendor_[0] = '\0';
		identifier_[0] = '\0';
		revision_[0] = '\0';

		// Try to obtain vendor and product identifiers.
		unsigned char buffer[192];
		if (inquiry(buffer,sizeof(buffer)))
		{
			ckmmc::ScsiInquiryData inquiry_data;
			inquiry_data.parse(buffer);

			ckcore::string::ansi_to_auto(inquiry_data.vendor_,vendor_,9);
			ckcore::string::ansi_to_auto(inquiry_data.product_,identifier_,17);
			ckcore::string::ansi_to_auto(inquiry_data.rev_,revision_,5);
		}
		else
		{
			ckcore::log::print_line(ckT("[mmcdevice]: unable to obtain device inquiry data from %d,%d,%d."),
									addr.bus_,addr.target_,addr.lun_);
		}
	}

	/**
	 * Destructs the MmcDevice object.
	 */
	MmcDevice::~MmcDevice()
	{
	}

	/**
	 * Checks if the recorder is of Yamaha brand.
	 * @return If a Yamaha recorder true is returned, if not false is returned.
	 */
	bool MmcDevice::is_yamaha() const
	{
		return ckcore::string::astrncmp(vendor_,ckT("YAMAHA"),6) == 0;
	}

	/**
	 * Checks if the recorder is of Plextor brand.
	 * @return If a Plextor recorder true is returned, if not false is returned.
	 */
	bool MmcDevice::is_plextor() const
	{
		return ckcore::string::astrncmp(vendor_,ckT("PLEXTOR"),7) == 0;
	}

	/**
     * Returns the device vendor.
     * @return The device vendor.
     */
	const ckcore::tchar *MmcDevice::vendor() const
	{
		return vendor_;
	}

	/**
     * Returns the device identifier.
     * @return The device identifier.
     */
	const ckcore::tchar *MmcDevice::identifier() const
	{
		return identifier_;
	}

	/**
     * Returns the device revision.
     * @return The device revision.
     */
	const ckcore::tchar *MmcDevice::revision() const
	{
		return revision_;
	}

	/**
	 * Obtains the supported read speeds of the inserted medium.
	 * @param [out] speeds List of read speeds measured in sectors per second.
	 * @return If successful true is returned, if unsuccessful false is returend.
	 */
	const std::vector<ckcore::tuint32> &MmcDevice::read_speeds()
	{
		return read_speeds_;
	}

	/**
	 * Obtains the supported write speeds of the inserted medium.
	 * @param [out] speeds List of write speeds measured in sectors per second.
	 * @return If successful true is returned, if unsuccessful false is returend.
	 */
	const std::vector<ckcore::tuint32> &MmcDevice::write_speeds()
	{
		return write_speeds_;
	}

	/**
	 * Returns the value of the specified property.
	 * @param [in] prop 
	 * @return The value of the specified property.
	 */
	ckcore::tuint32 MmcDevice::property(Property prop) const
	{
		if (prop < ckPROP_INTERNAL_COUNT)
			return properties_[prop];

		return 0;
	}

	/**
	 * Checks if the device has recording capabilities.
	 * @return If the device is a recorder true is returned, otherwise false is
	 *         returned.
	 */
	bool MmcDevice::recorder()
	{
		return support(ckDEVICE_WRITE_CDR) ||
			   support(ckDEVICE_WRITE_CDRW) ||
			   support(ckDEVICE_WRITE_DVDR) ||
			   support(ckDEVICE_WRITE_DVDRAM);
	}

	/**
	 * Checks if the device support the specified feature.
	 * @return If the feature is supported true is returned, if not false is
	 *         returned.
	 */
	bool MmcDevice::support(Feature feature)
	{
		return (features_ & (static_cast<ckcore::tuint64>(1) << feature)) != 0;
	}

	/**
	 * Checks if the device support the specified write mode.
	 * @param [in] mode The write mode to test.
	 * @return If the write mode is supported true is returned, if not false
	 *		   is returned.
	 */
	bool MmcDevice::support(WriteMode mode)
	{
		return (write_modes_ & (static_cast<ckcore::tuint16>(1) << mode)) != 0;
	}

	/**
	 * Refreshes the device capabilities.
	 * @return If successful true is returned, if unsuccessful false is
	 *		   returned.
	 */
	bool MmcDevice::refresh()
	{
		ScsiSilencer(*this);
		unsigned char buffer[192];

		// Request mode page 0x2a.
		if (!mode_sense(0x2a,buffer,sizeof(buffer)))
		{
			ckcore::log::print_line(ckT("[mmcdevice]: requesting mode sense for page 0x2a failed."));
			return false;
		}

		// Parse the mode page 0x2a data.
		ScsiModePage2A mode_page_2a;
		if (!mode_page_2a.parse(buffer))
		{
			ckcore::log::print_line(ckT("[mmcdevice]: parsing of mode page 0x2a failed."));
			return false;
		}

		// Setup features.
		features_ = 0;

		if (mode_page_2a.read_cd_r_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_CDR;

		if (mode_page_2a.read_cd_rw_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_CDRW;

		if (mode_page_2a.method_2_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_METHOD_2;

		if (mode_page_2a.read_dvd_rom_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDROM;

		if (mode_page_2a.read_dvd_r_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDR;

		if (mode_page_2a.read_dvd_ram_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDRAM;

		if (mode_page_2a.write_cd_r_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_CDR;

		if (mode_page_2a.write_cd_rw_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_CDRW;

		if (mode_page_2a.test_write_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_TEST_WRITE;

		if (mode_page_2a.write_dvd_r_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDR;

		if (mode_page_2a.write_dvd_ram_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDRAM;

		if (mode_page_2a.audio_play_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_AUDIO_PLAY;

		if (mode_page_2a.composite_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_COMPOSITE;

		if (mode_page_2a.digital_port_1_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_DIGITAL_PORT_1;

		if (mode_page_2a.digital_port_2_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_DIGITAL_PORT_2;

		if (mode_page_2a.mode_2_form_1_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_MODE_2_FORM_1;

		if (mode_page_2a.mode_2_form_2_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_MODE_2_FORM_2;

		if (mode_page_2a.multi_session_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_MULTI_SESSION;

		if (mode_page_2a.buf_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_BUP;

		if (mode_page_2a.cdda_supported_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_CDDA_SUPPORTED;

		if (mode_page_2a.ccda_accurate_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_CDDA_ACCURATE;

		if (mode_page_2a.rw_supported_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_RW_SUPPORTED;

		if (mode_page_2a.rw_deint_corr_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_RW_DEINT_CORR;

		if (mode_page_2a.c2_pointers_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_C2_POINTERS;

		if (mode_page_2a.isrc_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_ISRC;

		if (mode_page_2a.upc_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_UPC;

		if (mode_page_2a.read_bar_code_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_BAR_CODE;

		if (mode_page_2a.lock_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_LOCK;

		if (mode_page_2a.lock_state_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_LOCK_STATE;

		if (mode_page_2a.prevent_jumper_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_PREVENT_JUMPER;

		if (mode_page_2a.eject_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_EJECT;

		if (mode_page_2a.sep_chan_vol_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_SEP_CHAN_VOL;

		if (mode_page_2a.sep_chan_mute_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_SEP_CHAN_MUTE;

		if (mode_page_2a.change_disc_prsnt_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_CHANGE_DISC_PRSNT;

		if (mode_page_2a.sss_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_SSS;

		if (mode_page_2a.change_sides_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_CHANGE_SIDES;

		if (mode_page_2a.rw_lead_in_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_RW_LEAD_IN;

		if (mode_page_2a.bckf_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_BCKF;

		if (mode_page_2a.rck_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_RCK;

		if (mode_page_2a.lsbf_)
			features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_LSBF;

		// Setup properties.
		memset(properties_,0,sizeof(properties_));

		properties_[ckPROP_NUM_VOL_LVLS] = mode_page_2a.num_vol_lvls_;
		properties_[ckPROP_BUFFER_SIZE] = mode_page_2a.buf_size_;
		properties_[ckPROP_COPY_MGMT_REV] = mode_page_2a.copy_man_rev_;
		properties_[ckPROP_LOAD_MECHANISM] = static_cast<Device::LoadMechanism>(mode_page_2a.load_mechanism_);
		properties_[ckPROP_ROT_CTRL] = static_cast<Device::RotCtrl>(mode_page_2a.rot_ctrl_);
		properties_[ckPROP_DA_BLOCK_LEN] = static_cast<Device::AudioBlockLen>(mode_page_2a.length_);
		properties_[ckPROP_MAX_READ_SPD] = mode_page_2a.max_read_spd_ * 1000 / 2352;
		properties_[ckPROP_CUR_READ_SPD] = mode_page_2a.cur_read_spd_ * 1000 / 2352;
		properties_[ckPROP_MAX_WRITE_SPD] = mode_page_2a.max_write_spd_ * 1000 / 2352;
		properties_[ckPROP_CUR_WRITE_SPD] = mode_page_2a.cur_write_spd_ * 1000 / 2352;

		// Setup read speeds.
		read_speeds_.clear();

		double ext_speed = static_cast<double>(property(ckPROP_MAX_READ_SPD))/75;
		ckcore::tuint32 cur_speed = static_cast<ckcore::tuint32>(ext_speed + 0.5);

		while (cur_speed > 0)
		{
			read_speeds_.push_back(cur_speed*75);
			cur_speed >>= 1;
		}

		// Setup write speeds (if the device have recording capabilities).
		if (recorder())
		{
			write_speeds_.clear();

			// Try to obtain the actual write speeds of any medium that is present.
			std::vector<ckcore::tuint16>::iterator it;
			for (it = mode_page_2a.write_spds_.begin(); it != mode_page_2a.write_spds_.end(); it++)
				write_speeds_.push_back(static_cast<ckcore::tuint32>(*it) * 1000 / 2352);

			// If no medium is present, calculate guessed write speeds (based on known maximum)b.
			if (write_speeds_.empty())
			{
				double ext_speed = static_cast<double>(property(ckPROP_MAX_WRITE_SPD))/75;
				ckcore::tuint32 cur_speed = static_cast<ckcore::tuint32>(ext_speed + 0.5);

				while (cur_speed > 0)
				{
					write_speeds_.push_back(cur_speed*75);
					cur_speed >>= 1;
				}
			}
		}

		// Try to obtain the supported write modes.
		if (recorder())
		{
			// Request mode page 0x05.
			if (!mode_sense(0x05,buffer,sizeof(buffer)))
			{
				ckcore::log::print_line(ckT("[mmcdevice]: requesting mode sense for page 0x05 failed."));
				return false;
			}

			// Parse the mode page 0x05 data.
			ScsiModePage05 mode_page_05;
			if (!mode_page_05.parse(buffer))
			{
				ckcore::log::print_line(ckT("[mmcdevice]: parsing of mode page 0x05 failed."));
				return false;
			}

			// Reset previous write modes.
			write_modes_ = 0;

			ckcore::tuint16 page_len = read_uint16_msbf(buffer) + 2;

			// Test packet writing.
			mode_page_05.write_type_ = ScsiModePage05::ckWT_PACKET;
			mode_page_05.track_mode_ = ScsiModePage05::ckTM_DATA | ScsiModePage05::ckTM_INCREMENTAL;
			mode_page_05.data_block_type_ = ScsiModePage05::ckDB_MODE_1_2048;
			mode_page_05.fp_ = false;			// Disable fixed packet size.
			mode_page_05.packed_size_ = 0;		// Set fixed packet size to zero.

			if (mode_select(buffer,page_len,false,true))
				write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_PACKET;

			mode_page_05.fp_ = false;
			mode_page_05.packed_size_ = 0;

			// Test TAO support.
			mode_page_05.write_type_ = ScsiModePage05::ckWT_TAO;
			mode_page_05.track_mode_ = ScsiModePage05::ckTM_DATA;
			mode_page_05.data_block_type_ = ScsiModePage05::ckDB_MODE_1_2048;

			if (mode_select(buffer,page_len,false,true))
				write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_TAO;

			// Test SAO support.
			mode_page_05.write_type_ = ScsiModePage05::ckWT_SAO;
			mode_page_05.track_mode_ = ScsiModePage05::ckTM_DATA;
			mode_page_05.data_block_type_ = ScsiModePage05::ckDB_MODE_1_2048;

			if (mode_select(buffer,page_len,false,true))
				write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_SAO;

			// Test RAW support.
			mode_page_05.write_type_ = ScsiModePage05::ckWT_RAW;
			mode_page_05.track_mode_ = ScsiModePage05::ckTM_DATA;
			mode_page_05.data_block_type_ = ScsiModePage05::ckDB_RAW_2352_PQ;

			if (mode_select(buffer,page_len,false,true))
			{
				mode_page_05.data_block_type_ = ScsiModePage05::ckDB_RAW_2352_PW_PACK;
				if (mode_select(buffer,page_len,false,true))
					write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_RAW16;

				mode_page_05.data_block_type_ = ScsiModePage05::ckDB_RAW_2352_PW;
				if (mode_select(buffer,page_len,false,true))
					write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_RAW96P;

				mode_page_05.data_block_type_ = ScsiModePage05::ckDB_RAW_2352_PQ;
				if (mode_select(buffer,page_len,false,true))
					write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_RAW96R;
			}

			// Test layer jump support.
			mode_page_05.write_type_ = ScsiModePage05::ckWT_LAYER_JUMP;
			mode_page_05.track_mode_ = ScsiModePage05::ckTM_DATA;
			mode_page_05.data_block_type_ = ScsiModePage05::ckDB_RAW_2352_PW;

			if (mode_select(buffer,page_len,false,true))
				write_modes_ |= static_cast<ckcore::tuint16>(1) << ckWM_LAYER_JUMP;
		}

		// Finally try to detect vendor specific features.
		if (recorder())
		{
			// Request mode page 0x05.
			if (!mode_sense(0x05,buffer,sizeof(buffer)))
			{
				ckcore::log::print_line(ckT("[mmcdevice]: requesting mode sense for page 0x05 failed."));
				return false;
			}

			// Parse the mode page 0x05 data.
			ScsiModePage05 mode_page_05;
			if (!mode_page_05.parse(buffer))
			{
				ckcore::log::print_line(ckT("[mmcdevice]: parsing of mode page 0x05 failed."));
				return false;
			}

			ckcore::tuint16 page_len = read_uint16_msbf(buffer) + 2;

			// Check for Yamaha and Plextor features.
			if (is_yamaha() || is_plextor())
			{
				// Reset the mode page.
				mode_page_05.reset_tao();
				if (!mode_select(buffer,page_len,false,true))
				{
					mode_page_05.reset_tao();
					if (!mode_select(buffer,page_len,false,true))
						ckcore::log::print_line(ckT("[mmcdevice]: unable to reset page 0x05."));
				}

				mode_page_05.buf_e_ = false;
				mode_page_05.write_type_ = ckmmc::ScsiModePage05::ckWT_AUDIO_MASTER;
				mode_page_05.track_mode_ = 0;
				mode_page_05.data_block_type_ = ckmmc::ScsiModePage05::ckDB_RAW_2352;

				if (mode_select(buffer,page_len,false,true))
					features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_AUDIO_MASTER;
			}

			// Check for Yamaha features.
			if (is_yamaha())
			{
				if (mode_page_05.page_len_ >= 26)
					features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_FORCE_SPEED;
			}

			// Check for plextor features.
			if (is_plextor())
			{
				// FIXME: Add a check since not all plextor drives support VARIREC.
				features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_VARIREC;
			}
		}

		// Obtain configuration feature set.
		unsigned char feature_buffer[32 * 1024];
		memset(feature_buffer,0,sizeof(feature_buffer));

		if (get_configuration(feature_buffer,sizeof(feature_buffer)))
		{
			unsigned char *ptr = feature_buffer;
			unsigned char *ptr_end = &feature_buffer[sizeof(feature_buffer)];

			ptr += 8;	// Skip header.
			while (ptr < ptr_end)
			{
				ckcore::tuint16 feature_code = (static_cast<ckcore::tuint16>(ptr[0]) << 8) | ptr[1];

				switch (feature_code)
				{
					case mmc::ckFEATURE_DVDPLUSRW:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDPLUSRW;

						// Warning: This assumes that mode page 2a has been evaluated already.
						if (support(ckDEVICE_WRITE_DVDR))
							features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDPLUSRW;
						break;

					case mmc::ckFEATURE_DVDPLUSR:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDPLUSR;

						// Warning: This assumes that mode page 2a has been evaluated already.
						if (support(ckDEVICE_WRITE_DVDR))
							features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDPLUSR;
						break;

					case mmc::ckFEATURE_DVDPLUSRW_DL:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDPLUSRW_DL;

						// Warning: This assumes that mode page 2a has been evaluated already.
						if (support(ckDEVICE_WRITE_DVDR))
							features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDPLUSRW_DL;
						break;

					case mmc::ckFEATURE_DVDPLUSR_DL:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_DVDPLUSR_DL;

						// Warning: This assumes that mode page 2a has been evaluated already.
						if (support(ckDEVICE_WRITE_DVDR))
							features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_DVDPLUSR_DL;
						break;

					case mmc::ckFEATURE_BD_READ:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_BD;
						break;

					case mmc::ckFEATURE_BD_WRITE:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_BD;
						break;

					case mmc::ckFEATURE_HDDVD_READ:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_READ_HDDVD;
						break;

					case mmc::ckFEATURE_HDDVD_WRITE:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_WRITE_HDDVD;
						break;

					case mmc::ckFEATURE_MULTIREAD:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_MULTIREAD;
						break;

					case mmc::ckFEATURE_CD_READ:
						features_ |= static_cast<ckcore::tuint64>(1) << ckDEVICE_CD_READ;
						break;
				}

				bool current = (ptr[2] & 0x01) > 0;
				bool persist = (ptr[2] & 0x02) > 0;
				unsigned char version = (ptr[2] >> 2) & 0x0f;

				ptr += ptr[3];
				ptr += 4;
			}
		}
		else
		{
			ckcore::log::print_line(ckT("[mmcdevice]: requesting configuration with buffer size %d failed."),
									sizeof(feature_buffer));
		}

		return true;
	}

	/**
	 * Returns the current media profile.
	 * @return The current media profile.
	 */
	Device::Profile MmcDevice::profile()
	{
		unsigned char buffer[8];

		// Request mode page 0x2a.
		if (!get_configuration(buffer,sizeof(buffer)))
		{
			ckcore::log::print_line(ckT("[mmcdevice]: requesting device configuration failed."));
			return ckPROFILE_NONE;
		}

		// Parse the configuration data.
		ScsiConfigurationData config_data;
		if (!config_data.parse(buffer))
		{
			ckcore::log::print_line(ckT("[mmcdevice]: parsing of configuration data failed."));
			return ckPROFILE_NONE;
		}

		return config_data.cur_profile_;
	}

	/**
	 * Executes a INQUIRY command on the device. This command is useful for
	 * obtaining device information.
	 * @param [out] buffer The buffer to which the returned data will be
	 *					   written.
	 * @param [in] buffer_len The size of the specified buffer.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool MmcDevice::inquiry(unsigned char *buffer,ckcore::tuint16 buffer_len)
	{
		// Initialize buffer.
		memset(buffer,0,buffer_len);

		// Prepare CDB.
		unsigned char cdb[16];
		memset(cdb,0,sizeof(cdb));

		cdb[0] = ckCMD_INQUIRY;
		cdb[4] = 0x24;

		if (!transport(cdb,6,buffer,buffer_len,ScsiDevice::ckTM_READ))
			return false;

		return true;
	}

	/**
	 * Executes a GET CONFIGURATION command on the device. This command is
	 * useful for obtaining the current media profile.
	 * @param [out] buffer The buffer to which the returned data will be
	 *					   written.
	 * @param [in] buffer_len The size of the specified buffer.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool MmcDevice::get_configuration(unsigned char *buffer,
									  ckcore::tuint16 buffer_len)
	{
		// Initialize buffer.
		memset(buffer,0,buffer_len);

		// Prepare CDB.
		unsigned char cdb[16];
		memset(cdb,0,sizeof(cdb));

		cdb[0] = ckCMD_GET_CONFIGURATION;
		cdb[7] = static_cast<unsigned char>(buffer_len >> 8);	// Allocation length (MSB).
		cdb[8] = static_cast<unsigned char>(buffer_len & 0xff);	// Allocation length (LSB).
		cdb[9] = 0x00;		

		if (!transport(cdb,10,buffer,buffer_len,ScsiDevice::ckTM_READ))
			return false;

		return true;
	}

	/**
	 * Executes a MODE SENSE (10) command on the device. This command is useful
	 * for obtaining device capabilities information.
	 * @param [in] page_code The code of the page to retrieve.
	 * @param [out] buffer The buffer to which the returned data will be
	 *					   written.
	 * @param [in] buffer_len The size of the specified buffer.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool MmcDevice::mode_sense(unsigned char page_code,unsigned char *buffer,
							   ckcore::tuint16 buffer_len)
	{
		if (page_code > 0x3f)
		{
			ckcore::log::print_line(ckT("[mmcdevice]: invalid page code 0x%x."),
									page_code);
			return false;
		}

		// Initialize buffer.
		memset(buffer,0,buffer_len);

		// Prepare CDB.
		unsigned char cdb[16];
		memset(cdb,0,sizeof(cdb));

		cdb[0] = ckCMD_MODE_SENSE10;
		cdb[1] = 0x08;					// Disable block descriptors.
		cdb[2] = page_code & 0x3f;		// Defined in MMC-2 standard (5.5.10).
		cdb[7] = static_cast<unsigned char>(buffer_len >> 8);	// Allocation length (MSB).
		cdb[8] = static_cast<unsigned char>(buffer_len & 0xff);	// Allocation length (LSB).
		cdb[9] = 0x00;

		if (!transport(cdb,10,buffer,buffer_len,ScsiDevice::ckTM_READ))
			return false;

		// Verify that we received the correct page.
		if ((buffer[8] & 0x3f) != page_code)
			return false;

		return true;
	}

	/**
	 * Executes a MODE SELECT (10) command on the device. This command is
	 * useful for setting the actual device configuration. This function is
	 * implemented according to SPC 3 - table 94.
	 * @param [in] buffer The buffer containing the data to be written to
	 *                    the device.
	 * @param [in] buffer_len The size of the specified buffer.
	 * @param [in] save_page If set to false the device will execute the
	 *						 command without saving it. For devices that
	 *						 does not support this separation, setting this
	 *						 parameter to false may cause the operation to
	 *						 fail.
	 * @param [in] page_format If set to true the written data is assumed
	 *						   to contain vendor specific information.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool MmcDevice::mode_select(unsigned char *buffer,
								ckcore::tuint16 buffer_len,bool save_page,
								bool page_format)
	{
		// Prepare header.
		buffer[0] = buffer[1] = 0;	// Reserved according to SPC 4 - table 291.
		buffer[4] = buffer[5] = 0;	// Reserved according to SPC 4 - table 291.

		// Prepare CDB.
		unsigned char cdb[16];
		memset(cdb,0,sizeof(cdb));

		cdb[0] = ckCMD_MODE_SELECT10;
		cdb[1] = (save_page ? 0x01 : 0x00) | (page_format ? 0x10 : 0x00);
		cdb[7] = static_cast<unsigned char>(buffer_len >> 8);
		cdb[8] = static_cast<unsigned char>(buffer_len & 0xff);
		cdb[9] = 0x00;

		if (!transport(cdb,10,buffer,buffer_len,ScsiDevice::ckTM_WRITE))
			return false;

		return true;
	}
};
