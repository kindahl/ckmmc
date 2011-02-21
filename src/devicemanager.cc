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
#include "ckmmc/scsidriverselector.hh"
#include "ckmmc/devicemanager.hh"

namespace ckmmc
{
    /**
     * Constructs an DeviceManager object.
     */
	DeviceManager::DeviceManager() :
		driver_(ScsiDriverSelector::driver())
    {
    }

    /**
     * Destructs the DeviceManager object.
     */
    DeviceManager::~DeviceManager()
    {
		clear();
    }

	/**
	 * Clear the list of known devices.
	 */
	void DeviceManager::clear()
	{
		// Free all allocated devices.
		std::vector<Device *>::iterator it;
		for (it = devices_.begin(); it != devices_.end(); it++)
			delete *it;

		devices_.clear();
	}

	/**
	 * Scans the system for devices.
	 * @param [in] callback Optional pointer to a callback object that will be
	 *                      notified how the scanning progresses.
	 * @return If successfull true is returned, otherwise false is returned.
	 */
	bool DeviceManager::scan(ScanCallback *callback)
	{
		// Remove any previous devices.
		clear();

		if (callback != NULL)
			callback->event_status(ScanCallback::ckEVENT_DEV_SCAN);

		// Scan system for devices.
		std::vector<ScsiDevice::Address> addresses;
		if (!driver_.scan(addresses))
			return false;

		// Add all devices.
		std::vector<ScsiDevice::Address>::iterator it_addr;
		for (it_addr = addresses.begin(); it_addr != addresses.end(); it_addr++)
		{
			devices_.push_back(new Device(*it_addr));

			if (callback != NULL)
			{
				// See if we should keep the device.
				if (!callback->event_device(*it_addr))
				{
					delete devices_.back();
					devices_.pop_back();
				}
			}
		}

		if (callback != NULL)
			callback->event_status(ScanCallback::ckEVENT_DEV_CAP);

		// Refresh the devices.
		std::vector<Device *>::iterator it;
		for (it = devices_.begin(); it != devices_.end(); it++)
		{
			if (!(*it)->refresh())
			{
				ckcore::log::print_line(ckT("[device]: unable to refresh device capabilities."));
			}
		}

		return true;
	}

	/**
	 * Returns a vector containing all known devices.
	 * @return A vector containing all known devices.
	 */
	const std::vector<Device *> &DeviceManager::devices() const
	{
		return devices_;
	}
};
