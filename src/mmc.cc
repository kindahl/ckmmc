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

#include "ckmmc/mmc.hh"

namespace ckmmc
{
	/**
	 * Reads a 16-bit integer from memory in Most Significant Byte First order.
	 * @param [in] buffer The buffer to read from.
	 * @return The parsed integer.
	 */
	ckcore::tuint16 read_uint16_msbf(unsigned char *buffer)
	{
		return (static_cast<ckcore::tuint16>(buffer[0]) << 8) | buffer[1];
	}

	/**
	 * Reads a 32-bit integer from memory in Most Significant Byte First order.
	 * @param [in] buffer The buffer to read from.
	 * @return The parsed integer.
	 */
	ckcore::tuint32 read_uint32_msbf(unsigned char *buffer)
	{
		return (static_cast<ckcore::tuint32>(buffer[0]) << 24) |
			   (static_cast<ckcore::tuint32>(buffer[1]) << 16) |
			   (static_cast<ckcore::tuint32>(buffer[2]) <<  8) | buffer[3];
	}

	/**
	 * Writes a 16-bit integer to a memory buffer in Most Significant Byte
	 * First order.
	 * @param [in] i The integer to write to the buffer.
	 * @param [out] buffer The buffer to which the integer data will be
	 *					   written.
	 */
	void write_uint16_msbf(ckcore::tuint16 i,unsigned char *buffer)
	{
		buffer[0] = static_cast<unsigned char>(i >> 8);
		buffer[1] = static_cast<unsigned char>(i & 0xff);
	}

	/**
	 * Writes a 32-bit integer to a memory buffer in Most Significant Byte
	 * First order.
	 * @param [in] i The integer to write to the buffer.
	 * @param [out] buffer The buffer to which the integer data will be
	 *					   written.
	 */
	void write_uint32_msbf(ckcore::tuint32 i,unsigned char *buffer)
	{
		buffer[0] = static_cast<unsigned char>(i >> 24);
		buffer[1] = static_cast<unsigned char>((i >> 16) & 0xff);
		buffer[2] = static_cast<unsigned char>((i >>  8) & 0xff);
		buffer[3] = static_cast<unsigned char>(i & 0xff);
	}

	/**
	 * Parses a buffer containing mode page 05 raw data as defined in MMC 2 -
	 * table 123 into a readable structure. This buffer should include the mode
	 * parameter header as defined in SPC 4 - table 291.
	 * @param [in] buffer Buffer to parse from.
	 * @param [in] buffer_len The length of the buffer.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiModePage05::parse(unsigned char *buffer)
	{
		// Validate page length.
		ckcore::tuint16 page_len = read_uint16_msbf(buffer) - 6;
		if (page_len < 52)
			return false;

		buffer += 8;

		// Validate page code.
		unsigned char page_code = buffer[0] & 0x3f;
		if (page_code != 0x05)
			return false;

		page_code_ = page_code;
		ps_ = (buffer[0] & 0x80) > 0;
		page_len_ = buffer[1];
		write_type_ = static_cast<WriteType>(buffer[2] & 0x0f);
		test_write_ = (buffer[2] & 0x10) > 0;
		ls_v_ = (buffer[2] & 0x20) > 0;
		buf_e_ = (buffer[2] & 0x40) > 0;
		track_mode_ = buffer[3] & 0x0f;
		copy_ = (buffer[3] & 0x10) > 0;
		fp_ = (buffer[3] & 0x20) > 0;
		multi_session_ = static_cast<MultiSession>((buffer[3] & 0xc0) >> 6);
		data_block_type_ = static_cast<DataBlock>(buffer[4] & 0x0f);
		link_size_ = buffer[5];
		host_app_code_ = buffer[7] & 0x3f;
		session_format_ = static_cast<SessionFormat>(buffer[8]);
		packed_size_ = read_uint32_msbf(buffer + 10);
		audio_pulse_len_ = read_uint16_msbf(buffer + 14);

		memcpy(media_cat_num_,buffer + 16,16);
		memcpy(int_std_rec_code_,buffer + 32,16);
		memcpy(int_std_rec_code_,buffer + 48,4);

		return true;
	}

	/**
	 * Reads the local data into a binary buffer. Only the page data will be
	 * written into the buffer, not any header as used in the parse function.
	 * @param [out] buffer The buffer to write the data into.
	 * @param [in] buffer_len The size of the buffer in bytes.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiModePage05::read(unsigned char *buffer,size_t buffer_len)
	{
		if (buffer_len < 52)
			return false;

		// Clear the buffer.
		memset(buffer,0,buffer_len);

		buffer[0] |= page_code_ & 0x3f;
		buffer[0] |= ps_ ? 0x80 : 0x00;
		buffer[1]  = page_len_;
		buffer[2] |= static_cast<unsigned char>(write_type_) & 0x0f;
		buffer[2] |= test_write_ ? 0x10 : 0x00;
		buffer[2] |= ls_v_ ? 0x20 : 0x00;
		buffer[2] |= buf_e_ ? 0x40 : 0x00;
		buffer[3] |= static_cast<unsigned char>(track_mode_) & 0x0f;
		buffer[3] |= copy_ ? 0x10 : 0x00;
		buffer[3] |= fp_ ? 0x20 : 0x00;
		buffer[3] |= (static_cast<unsigned char>(multi_session_) & 0x03) << 6;
		buffer[4] |= static_cast<unsigned char>(data_block_type_) & 0x0f;
		buffer[5]  = link_size_;
		buffer[7] |= host_app_code_ & 0x3f;
		buffer[8] = static_cast<unsigned char>(session_format_);

		write_uint32_msbf(packed_size_,buffer + 10);
		write_uint16_msbf(audio_pulse_len_,buffer + 14);

		memcpy(buffer + 16,media_cat_num_,16);
		memcpy(buffer + 32,int_std_rec_code_,16);
		memcpy(buffer + 48,int_std_rec_code_,4);

		return true;
	}

	/**
	 * Resets the mode page into a TAO default state.
	 */
	void ScsiModePage05::reset_tao()
	{
		write_type_ = ckWT_TAO;
		track_mode_ = ckTM_DATA;
		data_block_type_ = ckDB_MODE_1_2048;
		session_format_ = ckSF_CDROM_CDDA;
		audio_pulse_len_ = 150;
	}

	/**
	 * Resets the mode page into a SAO default state.
	 */
	void ScsiModePage05::reset_sao()
	{
		write_type_ = ckWT_SAO;
		track_mode_ = ckTM_DATA;
		data_block_type_ = ckDB_MODE_1_2048;
		session_format_ = ckSF_CDROM_CDDA;
		audio_pulse_len_ = 150;

		ls_v_ = 0;
		copy_ = 0;
		fp_ = 0;
		multi_session_ = ckMS_NEXT_DISALLOWED_NO_B0;
		host_app_code_ = 0;
	}

	/**
	 * Parses a buffer containing mode page 05 raw data as defined in MMC 3 -
	 * table 361 into a readable structure. This buffer should include the mode
	 * parameter header as defined in SPC 4 - table 291.
	 * @param [in] buffer Buffer to parse from.
	 * @param [in] buffer_len The length of the buffer.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiModePage2A::parse(unsigned char *buffer)
	{
		// Validate page length.
		ckcore::tuint16 page_len = read_uint16_msbf(buffer) - 6;
		if (page_len < 16)
			return false;

		buffer += 8;

		// Validate page code.
		unsigned char page_code = buffer[0] & 0x3f;
		if (page_code != 0x2a)
			return false;

		page_code_ = page_code;
		ps_ = (buffer[0] & 0x80) > 0;
		page_len_ = buffer[1];
		read_cd_r_ = (buffer[2] & 0x01) > 0;
		read_cd_rw_ = (buffer[2] & 0x02) > 0;
		method_2_ = (buffer[2] & 0x04) > 0;
		read_dvd_rom_ = (buffer[2] & 0x08) > 0;
		read_dvd_r_ = (buffer[2] & 0x10) > 0;
		read_dvd_ram_ = (buffer[2] & 0x20) > 0;
		write_cd_r_ = (buffer[3] & 0x01) > 0;
		write_cd_rw_ = (buffer[3] & 0x02) > 0;
		test_write_ = (buffer[3] & 0x04) > 0;
		write_dvd_r_ = (buffer[3] & 0x10) > 0;
		write_dvd_ram_ = (buffer[3] & 0x20) > 0;
		audio_play_ = (buffer[4] & 0x01) > 0;
		composite_ = (buffer[4] & 0x02) > 0;
		digital_port_1_ = (buffer[4] & 0x04) > 0;
		digital_port_2_ = (buffer[4] & 0x08) > 0;
		mode_2_form_1_ = (buffer[4] & 0x10) > 0;
		mode_2_form_2_ = (buffer[4] & 0x20) > 0;
		multi_session_ = (buffer[4] & 0x40) > 0;
		buf_ = (buffer[4] & 0x80) > 0;
		cdda_supported_ = (buffer[5] & 0x01) > 0;
		ccda_accurate_ = (buffer[5] & 0x02) > 0;
		rw_supported_ = (buffer[5] & 0x04) > 0;
		rw_deint_corr_ = (buffer[5] & 0x08) > 0;
		c2_pointers_ = (buffer[5] & 0x10) > 0;
		isrc_ = (buffer[5] & 0x20) > 0;
		upc_ = (buffer[5] & 0x40) > 0;
		read_bar_code_ = (buffer[5] & 0x80) > 0;
		lock_ = (buffer[6] & 0x01) > 0;
		lock_state_ = (buffer[6] & 0x02) > 0;
		prevent_jumper_ = (buffer[6] & 0x04) > 0;
		eject_ = (buffer[6] & 0x08) > 0;
		load_mechanism_ = static_cast<Device::LoadMechanism>((buffer[6] >> 5) & 0x07);
		sep_chan_vol_ = (buffer[7] & 0x01) > 0;
		sep_chan_mute_ = (buffer[7] & 0x02) > 0;
		change_disc_prsnt_ = (buffer[7] & 0x04) > 0;
		sss_ = (buffer[7] & 0x08) > 0;
		change_sides_ = (buffer[7] & 0x10) > 0;
		rw_lead_in_ = (buffer[7] & 0x20) > 0;

		// Validate maximum read speed.
		max_read_spd_ = read_uint16_msbf(buffer + 8);
		if (max_read_spd_ < 176 && max_read_spd_ > 0)
			return false;

		// Validate current read speed.
		num_vol_lvls_ = read_uint16_msbf(buffer + 10);
		buf_size_ = read_uint16_msbf(buffer + 12);

		cur_read_spd_ = read_uint16_msbf(buffer + 14);
		if (cur_read_spd_ < 176 && cur_read_spd_ > 0)
			return false;

		bckf_ = (buffer[17] & 0x02) > 0;
		rck_ = (buffer[17] & 0x04) > 0;
		lsbf_ = (buffer[17] & 0x08) > 0;
		length_ = static_cast<Device::AudioBlockLen>((buffer[17] >> 4) & 0x03);
		max_write_spd_ = read_uint16_msbf(buffer + 18);
		cur_write_spd_ = read_uint16_msbf(buffer + 20);

		// Only available on MMC-2 and newer devices.
		if (page_len >= 24)
			copy_man_rev_ = read_uint16_msbf(buffer + 22);
		else
			copy_man_rev_ = 0;

		// Only available on MMC-3 and newer devices.
		if (page_len_ >= 28)
			rot_ctrl_ = static_cast<Device::RotCtrl>(buffer[27] & 0x03);
		else
			rot_ctrl_ = Device::ckRC_CLV;

		// If MMC-3 use the new current write speed field.
		if (page_len_ >= 28)
			cur_write_spd_ = read_uint16_msbf(buffer + 28);

		write_spds_.clear();

		// Only available on MMC-3 and newer devices.
		if (page_len_ >= 28)
		{
			ckcore::tuint16 num_write_spds = read_uint16_msbf(buffer + 30);
			for (ckcore::tuint16 i = 0; i < num_write_spds; i++)
				write_spds_.push_back(read_uint16_msbf(buffer + 32 + 2 + (i << 2)));
		}

		return true;
	}

	/**
	 * Parses a buffer containing raw inquiry data as defined in SPC 2 -
	 * table 46 into a readable structure.
	 * @param [in] buffer Buffer to parse from.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiInquiryData::parse(unsigned char *buffer)
	{
		perh_dev_type_ = buffer[0] & 0x1f;
		perh_qual_ = buffer[0] >> 5;
		rmb_ = (buffer[1] & 0x80) > 0;
		version_ = buffer[2];
		res_data_format_ = buffer[3] & 0x0f;
		hi_sup_ = (buffer[3] & 0x10) > 0;
		norm_aca_ = (buffer[3] & 0x20) > 0;
		aerc_ = (buffer[3] & 0x80) > 0;
		additional_len_ = buffer[4];
		sccs_ = (buffer[5] & 0x80f) > 0;
		addr_16_ = (buffer[6] & 0x01) > 0;
		mchngr_ = (buffer[6] & 0x08) > 0;
		multip_ = (buffer[6] & 0x10) > 0;
		vs_1_ = (buffer[6] & 0x20) > 0;
		enc_serv_ = (buffer[6] & 0x40) > 0;
		bq_ue_ = (buffer[6] & 0x80) > 0;
		vs_2_ = (buffer[7] & 0x01) > 0;
		cmd_queue_ = (buffer[7] & 0x08) > 0;
		linked_ = (buffer[7] & 0x10) > 0;
		sync_ = (buffer[7] & 0x20) > 0;
		wbus_16_ = (buffer[7] & 0x40) > 0;
		rel_addr_ = (buffer[7] & 0x80) > 0;

		// Copy and trim the vendor identifier.
		memcpy(vendor_,buffer + 8,8);
		vendor_[8] = '\0';
		for (int i = 7; i >= 0; i--)
		{
			if (vendor_[i] == ' ')
				vendor_[i] = '\0';
			else
				break;
		}

		// Copy and trim the product identifier.
		memcpy(product_,buffer + 16,16);
		product_[16] = '\0';
		for (int i = 15; i >= 0; i--)
		{
			if (product_[i] == ' ')
				product_[i] = '\0';
			else
				break;
		}

		// Copy and trim the product revision identifier.
		memcpy(rev_,buffer + 32,4);
		rev_[4] = '\0';
		for (int i = 3; i >= 0; i--)
		{
			if (rev_[i] == ' ')
				rev_[i] = '\0';
			else
				break;
		}

		return true;
	}

	/**
	 * Parses a buffer containing raw configuration data as defined in MMC 3 -
	 * table 74 into a readable structure.
	 * @param [in] buffer Buffer to parse from.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiConfigurationData::parse(unsigned char *buffer)
	{
		data_len_ = read_uint32_msbf(buffer);
		cur_profile_ = static_cast<Device::Profile>(read_uint16_msbf(buffer + 6));

		return true;
	}
};
