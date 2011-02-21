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
 * @file include/ckmmc/scsisilencer.hh
 * @brief Defines the SCSI device silencer class.
 */

#pragma once
#include <ckcore/types.hh>
#include "ckmmc/scsidevice.hh"

namespace ckmmc
{
    /**
     * @brief Class for temporarilly silencing a SCSI device.
     */
	class ScsiSilencer
    {
	private:
		ScsiDevice &device_;

    public:
		ScsiSilencer(ScsiDevice &device);
		~ScsiSilencer();
    };
};
