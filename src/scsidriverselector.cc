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

#ifdef _WINDOWS
#include "ckmmc/windows/aspidriver.hh"
#include "ckmmc/windows/sptidriver.hh"
#endif
#include "ckmmc/scsidriverselector.hh"

namespace ckmmc
{
	/**
	 * Constructs a ScsiDriverSelector object.
	 */
	ScsiDriverSelector::ScsiDriverSelector()
	{
	}

	/**
	 * Destructs the ScsiDriverSelector object.
	 */
	ScsiDriverSelector::~ScsiDriverSelector()
	{
	}

	/**
	 * Returns the selected SCSI driver instance.
	 * @return The selected SCSI driver instance.
	 */
	ScsiDriver& ScsiDriverSelector::driver()
	{
#ifdef _WINDOWS
		OSVERSIONINFO osvi;
		memset(&osvi,0,sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// Use SPTI on Windows 2000 and newer.
		GetVersionEx(&osvi);
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion > 4)
		{
			// Use cdrtools compatibility mode.
			static SptiDriver driver(true);
			return driver;
		}
		else
		{
			static AspiDriver driver;
			return driver;
		}
#endif
	}
};
