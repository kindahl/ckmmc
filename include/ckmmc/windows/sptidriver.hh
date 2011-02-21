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
 * @file sptidriver.hh
 * @brief Defines the SPTI driver class.
 */

#pragma once
#include <windows.h>
#include <vector>
#include <map>
#include <ckcore/types.hh>
#include "ckmmc/scsidriver.hh"

namespace ckmmc
{
    /**
     * @brief SPTI driver class.
     */
	class SptiDriver : public ScsiDriver
    {
	private:
		enum
		{
			ckSPTI_DEFAULT_TIMEOUT = 60
		};

		bool ctcm_;
		long timeout_;
		std::map<ckcore::tchar,HANDLE> handles_;

		HANDLE get_handle(ScsiDevice &device);

    public:
		SptiDriver(bool ctcm);
        ~SptiDriver();

		static bool find_device_str(ScsiDevice::Address &addr);

		/*
		 * ScsiDriver Interface.
		 */
		bool timeout(long timeout);

		bool scan(std::vector<ScsiDevice::Address> &addresses);

		bool transport(ScsiDevice &device,
					   unsigned char *cdb,unsigned char cdb_len,
					   unsigned char *data,unsigned long data_len,
					   ScsiDevice::TransportMode mode);

		bool transport_with_sense(ScsiDevice &device,
								  unsigned char *cdb,unsigned char cdb_len,
								  unsigned char *data,unsigned long data_len,
								  ScsiDevice::TransportMode mode,
								  unsigned char *sense,unsigned char &result);
    };
};
