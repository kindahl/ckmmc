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
 * @file include/ckmmc/devicemanager.hh
 * @brief Defines the device manager class.
 */

#pragma once
#include <ckcore/types.hh>
#include <ckcore/process.hh>
#include "ckmmc/device.hh"
#include "ckmmc/scsidriver.hh"

namespace ckmmc
{
    /**
     * @brief Device manager class.
     */
    class DeviceManager
    {
	public:
		/**
		 * @brief System device scan callback interface.
		 */
		class ScanCallback
		{
		public:
			/**
			 * Defines different event types.
			 */
			enum Status
			{
				ckEVENT_DEV_SCAN,		// Scanning the system bus for new devices.
				ckEVENT_DEV_CAP			// Obtaining the capabilities of an individual device.
			};

			/**
			 * Called when the system scanning status has changed.
			 * @param [in] status The new status.
			 */
			virtual void event_status(Status status) = 0;

			/**
			 * Called when a new device has been found.
			 * @param [in] addr Address of the added device.
			 * @return If false is returned the device will be skipped. If true is
			 *         returned the device manager will keep the device.
			 */
			virtual bool event_device(Device::Address &addr) = 0;
		};

	private:
		/**
		 * Defines internal constants.
		 */
		enum
		{
			ckDM_PARSE_MAX_LINE = 1024
		};

		ScsiDriver &driver_;

		// Vector containing all devices.
		std::vector<Device *> devices_;

		void clear();

    public:
        DeviceManager();
        ~DeviceManager();

		bool scan(ScanCallback *callback);

		const std::vector<Device *> &devices() const;
    };
};
