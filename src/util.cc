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

#include <math.h>
#include "ckmmc/util.hh"

namespace ckmmc
{
	namespace util
	{
		/**
		 * Converts speed measured in sectors per second into the more human
		 * readable <speed>x format.
		 * @param [in] sec_spede The sector speed to convert.
		 * @param [in] profile The profile to use for conversion.
		 * @return The speed in human readable form.
		 */
		float sec_to_human_speed(ckcore::tuint32 sec_speed,
								 Device::Profile profile)
		{
			switch (profile)
			{
				case Device::ckPROFILE_DVDROM:
				case Device::ckPROFILE_DVDMINUSR_SEQ:
				case Device::ckPROFILE_DVDRAM:
				case Device::ckPROFILE_DVDMINUSRW_RESTOV:
				case Device::ckPROFILE_DVDMINUSRW_SEQ:
				case Device::ckPROFILE_DVDMINUSR_DL_SEQ:
				case Device::ckPROFILE_DVDMINUSR_DL_JUMP:
				case Device::ckPROFILE_DVDPLUSRW:
				case Device::ckPROFILE_DVDPLUSR:
				case Device::ckPROFILE_DVDPLUSRW_DL:
				case Device::ckPROFILE_DVDPLUSR_DL:
					return (float)sec_speed/675;

				case Device::ckPROFILE_BDROM:
				case Device::ckPROFILE_BDR_SRM:
				case Device::ckPROFILE_BDR_RRM:
				case Device::ckPROFILE_BDRE:
				case Device::ckPROFILE_HDDVDROM:
				case Device::ckPROFILE_HDDVDR:
				case Device::ckPROFILE_HDDVDRAM:
					return (float)sec_speed/2231;

				default:
					return (float)((unsigned long)floor((double)sec_speed/75 + 0.5));
			}
		}

		/**
		 * Converts speed measured in sectors per second into the more human
		 * readable <speed>x format as a string.
		 * @param [in] sec_spede The sector speed to convert.
		 * @param [in] profile The profile to use for conversion.
		 * @return The speed in human readable string form.
		 */
		ckcore::tstring sec_to_disp_speed(ckcore::tuint32 sec_speed,
										  Device::Profile profile)
		{
			ckcore::tstringstream s;
			float speed = sec_to_human_speed(sec_speed,profile);

			switch (profile)
			{
				case Device::ckPROFILE_CDROM:
				case Device::ckPROFILE_CDR:
				case Device::ckPROFILE_CDRW:
					s << static_cast<ckcore::tuint32>(speed);
					break;

				default:
					s.precision(1);
					s << std::fixed << speed;
					break;
			}

			s << ckT("x");
			return s.str();
		}
	};
};

