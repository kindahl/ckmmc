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
 * @file include/ckmmc/scsidriver.hh
 * @brief Defines the SCSI driver interface.
 */

#pragma once
#include <vector>
#include <ckcore/types.hh>
#include "ckmmc/scsidevice.hh"

namespace ckmmc
{
    /**
     * @brief Defines the SCSI driver interface.
     */
    class ScsiDriver
    {
	protected:
		bool silent_;

    public:
		ScsiDriver() : silent_(false) {};
		virtual ~ScsiDriver() {};

		/**
		 * Sets the command timeout value.
		 * @param [in] timeout The new timeout value.
		 * @return If successful true is returned, if not false is returned.
		 */
		virtual bool timeout(long timeout) = 0;

		/**
		 * Enables or disable writing to the program log or error.
		 * @param [in] enable If true no information will be written to the log.
		 * @return If successful true is returned, if not false is returned.
		 */
		bool silence(bool enable) { silent_ = enable; return true; };

		/**
		 * Scans the system for devices.
		 * @param [out] addresses Vector containing addresses of all detected
		 *						  disc devices.
		 * @return If successful true is returned, if not false is returned.
		 */
		virtual bool scan(std::vector<ScsiDevice::Address> &addresses) = 0;

		/**
		 * Transports data from or to the device using SCSI commands.
		 * @param [in] device The device to transport the command to.
		 * @param [in] cdb Buffer to command descriptor block.
		 * @param [in] cdb_len Length of the command descriptor block.
		 * @param [in] data Pointer to data buffer for either receiving or
		 *					writing data.
		 * @param [in] data_len Length of the data buffer.
		 * @param [in] mode Specifies the transport mode.
		 * @param [in] silent Set to true to prevent any information to be
		 *					  written to the program log.
		 * @return If the transport was successfully carried through true is
		 *         returned, if not false is returned.
		 */
		virtual bool transport(ScsiDevice &device,
							   unsigned char *cdb,unsigned char cdb_len,
							   unsigned char *data,unsigned long data_len,
							   ScsiDevice::TransportMode mode) = 0;

		/**
		 * Transports data from or to the device using SCSI commands. This is
		 * similar to the transport function with the exception that the sense
		 * and result is written back to the caller.
		 * @param [in] device The device to transport the command to.
		 * @param [in] cdb Buffer to command descriptor block.
		 * @param [in] cdb_len Length of the command descriptor block.
		 * @param [in] data Pointer to data buffer for either receiving or
		 *					writing data.
		 * @param [in] data_len Length of the data buffer.
		 * @param [in] mode Specifies the transport mode.
		 * @param [out] sense Pointer to sense buffer.
		 * @param [out] result Contains the transport result.
		 * @param [in] silent Set to true to prevent any information to be
		 *					  written to the program log.
		 * @return If the transport was successfully carried through true is
		 *         returned, if not false is returned.
		 */
		virtual bool transport_with_sense(ScsiDevice &device,
										  unsigned char *cdb,unsigned char cdb_len,
										  unsigned char *data,unsigned long data_len,
										  ScsiDevice::TransportMode mode,
										  unsigned char *sense,unsigned char &result) = 0;
    };
};
