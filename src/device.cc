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

#include <ckcore/string.hh>
#include <ckcore/log.hh>
#include "ckmmc/device.hh"

namespace ckmmc
{
    /**
     * Constructs an Device object.
     */
    Device::Device(Address &addr) :
		MmcDevice(addr)
    {
		name_ = vendor_;
		name_ += ckT(" ");
		name_ += identifier_;
		name_ += ckT(" ");
		name_ += revision_;
    }

    /**
     * Destructs the Device object.
     */
    Device::~Device()
    {
    }

	/**
	 * Sets the value of a device property.
	 * @param [in] prop The property to update.
	 * @param [in] value The new value of the property.
	 */
	void Device::set_property(Property prop,ckcore::tuint32 value)
	{
		if (prop < Device::ckPROP_INTERNAL_COUNT)
			properties_[prop] = value;
	}

	/**
     * Returns the full device name.
     * @return The full device name.
     */
    const ckcore::tchar *Device::name() const
	{
		return name_.c_str();
	}
};
