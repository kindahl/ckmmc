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

/**
 * @file include/ckmmc/scsidevice.hh
 * @brief Defines the Windows SCSI device class.
 */

#pragma once
#include <ckcore/types.hh>

namespace ckmmc
{
	class ScsiDriver;

	/**
     * @brief Class representing a SCSI device.
     */
	class ScsiDevice
    {
	public:
		/**
		 * @brief Device address class.
		 * Class representing a device address. The class contains two different sets
		 * of data (device or bus, target and lun), only one set of data is garantied
		 * to be valid. Which of them depends of the device implementation.
		 */
		class Address
		{
		public:
			ckcore::tstring device_;
			ckcore::tint32 bus_,target_,lun_;

			/**
			 * Constructs a default (invalid) address.
			 */
			Address() : bus_(-1),target_(-1),lun_(-1) {}

			/**
			 * Makes this object a copy of another Address object.
			 * @param [in] addr The address to copy from.
			 */
			Address(const Address &addr)
			{
				device_ = addr.device_;
				bus_ = addr.bus_;
				target_ = addr.target_;
				lun_ = addr.lun_;
			}
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
		 * Defines status codes.
		 */
		enum
		{
			ckSCSISTAT_GOOD = 0x00,
			ckSCSISTAT_CHECK_CONDITION = 0x02,
			ckSCSISTAT_CONDITION_MET = 0x04,
			ckSCSISTAT_BUSY = 0x08,
			ckSCSISTAT_INTERMEDIATE = 0x10,
			ckSCSISTAT_INTERMEDIATE_COND_MET = 0x14,
			ckSCSISTAT_RESERVATION_CONFLICT = 0x18,
			ckSCSISTAT_COMMAND_TERMINATED = 0x22,
			ckSCSISTAT_QUEUE_FULL = 0x28
		};

	protected:
		Address addr_;

	private:
		ScsiDriver &driver_;

    public:
		ScsiDevice(const Address &addr);
		virtual ~ScsiDevice();

		const Address &address() const;

		bool timeout(long timeout);

		bool silence(bool enable);

		bool transport(unsigned char *cdb,unsigned char cdb_len,
					   unsigned char *data,unsigned long data_len,
					   ScsiDevice::TransportMode mode);

		bool transport_with_sense(unsigned char *cdb,unsigned char cdb_len,
								  unsigned char *data,unsigned long data_len,
								  ScsiDevice::TransportMode mode,
								  unsigned char *sense,unsigned char &result);
    };
};
