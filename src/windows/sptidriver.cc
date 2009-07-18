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

#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <ckcore/string.hh>
#include <ckcore/log.hh>
#include <ckcore/convert.hh>
#include "ckmmc/windows/sptidriver.hh"

namespace ckmmc
{
	typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER
	{
		SCSI_PASS_THROUGH_DIRECT spt;
		ULONG Filler;			// Realign buffer to double word boundary.
		UCHAR ucSenseBuf[32];
	} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

    /**
     * Constructs an SptiDriver object.
     */
	SptiDriver::SptiDriver() : timeout_(ckSPTI_DEFAULT_TIMEOUT)
    {
    }

	/**
     * Destructs the SptiDriver object.
     */
    SptiDriver::~SptiDriver()
    {
		// Release all device handles.
		std::map<ckcore::tchar,HANDLE>::iterator it;
		for (it = handles_.begin(); it != handles_.end(); it++)
			CloseHandle(it->second);

		handles_.clear();
    }

	/**
	 * Tries to find the handle of the specified device.
	 * @param [in] device The device to find the handle of.
	 * @return If successful the handle is returned, if not
	 *		   INVALID_HANDLE_VALUE is returned.
	 */
	HANDLE SptiDriver::get_handle(ScsiDevice &device)
	{
		if (device.address().device_.empty())
		{
			ckcore::log::print_line(ckT("[sptidriver]: invalid address."));
			return INVALID_HANDLE_VALUE;
		}

		// See if a handle already exist.
		ckcore::tchar drv_letter = device.address().device_[0];
		if (handles_.count(drv_letter) > 0)
			return handles_[drv_letter];

		// Create a new handle to the device.
		ckcore::tchar drive_str[7];
		lstrcpy(drive_str,ckT("\\\\.\\X:"));
		drive_str[4] = drv_letter;

		// Create the device connection.
		HANDLE handle = CreateFile(drive_str,GENERIC_WRITE | GENERIC_READ,
								   FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
								   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

		if (handle == INVALID_HANDLE_VALUE)
			return INVALID_HANDLE_VALUE;

		handles_[drv_letter] = handle;
		return handle;
	}

	/**
	 * Tries to find the device letter string for the specified address.
	 * @param [in,out] addr The address to update with device drive letter
	 *						information.
	 * @return If the device string could be retrieved true is returned, if not
	 *		   false is returned.
	 */
	bool SptiDriver::find_device_str(ScsiDevice::Address &addr)
	{
		unsigned long dummy;
		SCSI_ADDRESS scsi_addr;

		ckcore::tchar drive_str[7];
		lstrcpy(drive_str,ckT("\\\\.\\X:"));
		
		for (ckcore::tchar drive_letter = 'C'; drive_letter <= 'Z'; drive_letter++)
		{
			// Open the device.
			drive_str[4] = drive_letter;
			HANDLE device = CreateFile(drive_str,GENERIC_WRITE | GENERIC_READ,
									   FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
									   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (device == INVALID_HANDLE_VALUE)
				continue;

			memset(&scsi_addr,0,sizeof(SCSI_ADDRESS));
			bool result = DeviceIoControl(device,IOCTL_SCSI_GET_ADDRESS,NULL,0,
										  &scsi_addr,sizeof(SCSI_ADDRESS),&dummy,FALSE) != 0;

			// Close device handle.
			CloseHandle(device);

			// Validate address query result.
			if (!result)
				continue;

			if (scsi_addr.PortNumber == addr.bus_ &&
				scsi_addr.TargetId == addr.target_ &&
				scsi_addr.Lun == addr.lun_)
			{
				addr.device_.clear();
				addr.device_.push_back(drive_letter);
				return true;
			}
		}

		return false;
	}

	/**
	 * Sets the command timeout value.
	 * @param [in] timeout The new timeout value.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool SptiDriver::timeout(long timeout)
	{
		timeout_ = timeout < 0 ? ckSPTI_DEFAULT_TIMEOUT : timeout;
		return true;
	}

	/**
	 * Scans the system for devices.
	 * @param [out] addresses Vector containing addresses of all detected
	 *						  disc devices.
	 * @return If successful true is returned, if not false is returned.
	 */
	bool SptiDriver::scan(std::vector<ScsiDevice::Address> &addresses)
	{
		unsigned long dummy;
		SCSI_ADDRESS scsi_addr;

		ckcore::tchar drive_str[7];
		lstrcpy(drive_str,ckT("\\\\.\\X:"));
		
		for (ckcore::tchar drive_letter = 'C'; drive_letter <= 'Z'; drive_letter++)
		{
			drive_str[4] = drive_letter;

			// We're only intersted in disc devices.
			if (GetDriveType(drive_str + 4) != DRIVE_CDROM)
				continue;

			// Open the device.
			HANDLE handle = CreateFile(drive_str,GENERIC_WRITE | GENERIC_READ,
									   FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
									   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (handle == INVALID_HANDLE_VALUE)
				continue;

			// Try to obtain the device's SCSI address.
			ScsiDevice::Address addr;

			memset(&scsi_addr,0,sizeof(SCSI_ADDRESS));
			if (DeviceIoControl(handle,IOCTL_SCSI_GET_ADDRESS,NULL,0,
								&scsi_addr,sizeof(SCSI_ADDRESS),&dummy,FALSE) != 0)
			{
				addr.bus_ = scsi_addr.PortNumber;
				addr.target_ = scsi_addr.TargetId;
				addr.lun_ = scsi_addr.Lun;
			}
			else
			{
				// Some Firewire devices does not support the above operation.
				// If that is the case use the cdrtools address hack.
				if (GetLastError() == ERROR_NOT_SUPPORTED)
				{
					addr.bus_ = drive_letter - 'A';
					addr.target_ = 0;
					addr.lun_ = 0;
				}
				else
				{
					CloseHandle(handle);
					continue;
				}
			}

			addr.device_.push_back(drive_letter);

			// Remember the handle.
			handles_[drive_letter] = handle;

			// Add the address to the address vector.
			addresses.push_back(addr);
		}

		return true;
	}

	/**
	 * Transports data from or to the device using SCSI commands.
	 * @param [in] device The device to transport the command to.
	 * @param [in] cdb Buffer to command descriptor block.
	 * @param [in] cdb_len Length of the command descriptor block.
	 * @param [in] data Pointer to data buffer for either receiving or
	 *					writing data.
	 * @param [in] data_len Length of the data buffer.
	 * @param [in] mode Specifies the transport mode.
	 * @return If the transport was successfully carried through true is
	 *         returned, if not false is returned.
	 */
	bool SptiDriver::transport(ScsiDevice &device,
							   unsigned char *cdb,unsigned char cdb_len,
							   unsigned char *data,unsigned long data_len,
							   ScsiDevice::TransportMode mode)
	{
		// Try to obtain the device handle.
		HANDLE handle = get_handle(device);
		if (handle == INVALID_HANDLE_VALUE)
		{
			if (!silent_)
			{
				ckcore::log::print_line(ckT("[sptidriver]: unable to obtain device handle (%d, %d, %d, %s)."),
										device.address().bus_,device.address().target_,device.address().lun_,
										device.address().device_.c_str());
			}

			return false;
		}

		if (cdb == NULL || cdb_len > 16)
			return false;

		// Prepare SCSI command.
		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptwb;
		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
		sptwb.spt.DataTransferLength = data_len;
		sptwb.spt.DataBuffer = data;
		sptwb.spt.CdbLength = cdb_len;
		sptwb.spt.TimeOutValue = timeout_;
		memcpy(sptwb.spt.Cdb,cdb,cdb_len);
	
		switch (mode)
		{
			case ScsiDevice::ckTM_UNSPECIFIED:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
				break;

			case ScsiDevice::ckTM_READ:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
				break;

			case ScsiDevice::ckTM_WRITE:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
				break;

			default:
				return false;
		}

		// Send SCSI command.
		unsigned long returned = 0;
		if (!DeviceIoControl(handle,IOCTL_SCSI_PASS_THROUGH_DIRECT,
							 &sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
							 &sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
							 &returned,FALSE))
		{
			if (!silent_)
			{
				ckcore::log::print_line(ckT("[sptidriver]: DeviceIoControl failed (%d; 0x%p, %d, 0x%p, %d, %d)."),
										GetLastError(),cdb,cdb_len,data,data_len,static_cast<int>(mode));
			}

			return false;
		}

		// Verify command result.
		if (sptwb.spt.ScsiStatus != ScsiDevice::ckSCSISTAT_GOOD)
		{
			if (!silent_)
			{
				ckcore::log::print_line(ckT("[sptidriver]: scsi command failed (0x%.2x)."),
										sptwb.spt.ScsiStatus);

				// Dump CDB.
				ckcore::log::print(ckT("[sptidriver]: > cdb: "));
				for (unsigned int i = 0; i < cdb_len; i++)
				{
					if (i == 0)
						ckcore::log::print(ckT("0x%.2x"),cdb[i]);
					else
						ckcore::log::print(ckT(",0x%.2x"),cdb[i]);
				}

				ckcore::log::print_line(ckT(""));

				// Dump sense information.
				ckcore::log::print_line(ckT("[sptidriver]: > sense key: 0x%x"),sptwb.ucSenseBuf[2] & 0xf);
				ckcore::log::print_line(ckT("[sptidriver]: > asc: 0x%.2x"),sptwb.ucSenseBuf[12]);
				ckcore::log::print_line(ckT("[sptidriver]: > ascq: 0x%.2x"),sptwb.ucSenseBuf[13]);
			}

			return false;
		}

		return true;
	}

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
	 * @return If the transport was successfully carried through true is
	 *         returned, if not false is returned.
	 */
	bool SptiDriver::transport_with_sense(ScsiDevice &device,
										  unsigned char *cdb,unsigned char cdb_len,
										  unsigned char *data,unsigned long data_len,
										  ScsiDevice::TransportMode mode,
										  unsigned char *sense,unsigned char &result)
	{
		// Try to obtain the device handle.
		HANDLE handle = get_handle(device);
		if (handle == INVALID_HANDLE_VALUE)
		{
			if (!silent_)
			{
				ckcore::log::print_line(ckT("[sptidriver]: unable to obtain device handle (%d, %d, %d, %s)."),
										device.address().bus_,device.address().target_,device.address().lun_,
										device.address().device_.c_str());
			}

			return false;
		}

		if (cdb == NULL || cdb_len > 16)
			return false;

		if (sense == NULL)
			return false;

		// Prepare SCSI command.
		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptwb;
		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
		sptwb.spt.DataTransferLength = data_len;
		sptwb.spt.DataBuffer = data;
		sptwb.spt.CdbLength = cdb_len;
		sptwb.spt.TimeOutValue = timeout_;
		memcpy(sptwb.spt.Cdb,cdb,cdb_len);
	
		switch (mode)
		{
			case ScsiDevice::ckTM_UNSPECIFIED:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
				break;

			case ScsiDevice::ckTM_READ:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
				break;

			case ScsiDevice::ckTM_WRITE:
				sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
				break;

			default:
				return false;
		}

		// Send SCSI command.
		unsigned long returned = 0;
		if (!DeviceIoControl(handle,IOCTL_SCSI_PASS_THROUGH_DIRECT,
							 &sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
							 &sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
							 &returned,FALSE))
		{
			if (!silent_)
			{
				ckcore::log::print_line(ckT("[sptidriver]: DeviceIoControl failed (%d; 0x%p, %d, 0x%p, %d, %d)."),
										GetLastError(),cdb,cdb_len,data,data_len,static_cast<int>(mode));
			}

			return false;
		}

		// Copy sense information into buffer.
		memcpy(sense,sptwb.ucSenseBuf,24);
		result = sptwb.spt.ScsiStatus;

		return true;
	}
};
