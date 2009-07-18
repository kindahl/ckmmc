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
 * @file include/ckmmc/device.hh
 * @brief Defines the device class.
 */

#pragma once
#include <ckcore/types.hh>
//#include "ckburn/device.hh"
#include "ckmmc/mmcdevice.hh"

namespace ckmmc
{
    /**
     * @brief Device interface implementation.
     */
    class Device : public MmcDevice
    {
	public:
		friend class CapabilitiesParser;	// The capabilities parser is allowed to
											// modify device internals.

	private:
		ckcore::tstring name_;

		void set_property(Property prop,ckcore::tuint32 value);

    public:
        Device(Address &addr);
        ~Device();

        const ckcore::tchar *name() const;
    };
};
