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

#include "ckmmc/scsidriverselector.hh"
#include "ckmmc/scsidevice.hh"

namespace ckmmc
{
	/**
	 * Constructs a ScsiDevice object.
	 */
	ScsiDevice::ScsiDevice(const Address &addr) :
		addr_(addr),
		driver_(ckmmc::ScsiDriverSelector::driver())
	{
	}

	/**
	 * Destructs the ScsiDevice object.
	 */
	ScsiDevice::~ScsiDevice()
	{
	}

	/**
	 * Returns the device address.
	 * @return The device address.
	 */
	const ScsiDevice::Address &ScsiDevice::address() const
	{
		return addr_;
	}

	/**
	 * Sets the command timeout value.
	 * @param [in] timeout The new timeout value.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiDevice::timeout(long timeout)
	{
		return driver_.timeout(timeout);
	}

	/**
	 * Enables or disable writing to the program log or error.
	 * @param [in] enable If true no information will be written to the log.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool ScsiDevice::silence(bool enable)
	{
		return driver_.silence(enable);
	}

	/**
	 * Transports data from or to the device using SCSI commands.
	 * @param [in] cdb Buffer to command descriptor block.
	 * @param [in] cdb_len Length of the command descriptor block.
	 * @param [in] data Pointer to data buffer for either receiving or
	 *					writing data.
	 * @param [in] data_len Length of the data buffer.
	 * @param [in] mode Specifies the transport mode.
	 * @param [in] timeout Specifies the maximum time to wait for the device to
						   return from a command.
	 * @return If the transport was successfully carried through true is
	 *         returned, if not false is returned.
	 */
	bool ScsiDevice::transport(unsigned char *cdb,unsigned char cdb_len,
							   unsigned char *data,unsigned long data_len,
							   ScsiDevice::TransportMode mode)
	{
		return driver_.transport(*this,cdb,cdb_len,data,data_len,mode);
	}

	/**
	 * Transports data from or to the device using SCSI commands. This is
	 * similar to the transport function with the exception that the sense
	 * and result is written back to the caller.
	 * @param [in] cdb Buffer to command descriptor block.
	 * @param [in] cdb_len Length of the command descriptor block.
	 * @param [in] data Pointer to data buffer for either receiving or
	 *					writing data.
	 * @param [in] data_len Length of the data buffer.
	 * @param [in] mode Specifies the transport mode.
	 * @param [out] sense Pointer to sense buffer.
	 * @param [out] result Contains the transport result.
	 * @param [in] timeout Specifies the maximum time to wait for the device to
						   return from a command.
	 * @return If the transport was successfully carried through true is
	 *         returned, if not false is returned.
	 */
	bool ScsiDevice::transport_with_sense(unsigned char *cdb,unsigned char cdb_len,
										  unsigned char *data,unsigned long data_len,
										  ScsiDevice::TransportMode mode,
										  unsigned char *sense,unsigned char &result)
	{
		return driver_.transport_with_sense(*this,cdb,cdb_len,data,data_len,
											mode,sense,result);
	}
};
