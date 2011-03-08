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

#include <windows.h>
#include <ckcore/log.hh>
#include "ckmmc/windows/aspidriver.hh"

namespace ckmmc
{
    /**
     * Constructs an AspiDriver object.
     */
    AspiDriver::AspiDriver() : dll_instance_(NULL),driver_loaded_(false)
    {
    }

    /**
     * Destructs the AspiDriver object.
     */
    AspiDriver::~AspiDriver()
    {
        driver_unload();
    }

    /**
     * Loads the ASPI driver DLL into memory.
     * @return If successful true is returned, if not false is returned.
     */
    bool AspiDriver::driver_load()
    {
        dll_instance_ = LoadLibrary(ckT("wnaspi32.dll"));
        if (dll_instance_ == NULL)
        {
            ckcore::log::print_line(ckT("[aspidriver]: unable to load aspi driver, wnaspi32.dll could not be loaded."));
            return false;
        }

        GetASPI32SupportInfo = (tGetASPI32SupportInfo)GetProcAddress(dll_instance_,"GetASPI32SupportInfo");
        if (!GetASPI32SupportInfo)
            return false;

        SendASPI32Command = (tSendASPI32Command)GetProcAddress(dll_instance_,"SendASPI32Command");
        if (!SendASPI32Command)
            return false;

        unsigned long status_code = HIBYTE(LOWORD(GetASPI32SupportInfo()));
        if (status_code != SS_COMP && status_code != SS_NO_ADAPTERS)
        {
            ckcore::log::print_line(ckT("[aspidriver]: unable to load aspi driver, status code 0x%.2x."),
                                    status_code);
            return false;
        }

        return true;
    }

    /**
     * Unloads the ASPI driver DLL from memory.
     * @return If successful true is returned, if not false is returned.
     */
    bool AspiDriver::driver_unload()
    {
        if (dll_instance_ == NULL)
            return false;

        FreeLibrary(dll_instance_);
        dll_instance_ = NULL;

        GetASPI32SupportInfo = NULL;
        SendASPI32Command = NULL;

        return true;
    }

    /**
     * Sets the command timeout value.
     * @param [in] timeout The new timeout value.
     * @return If successful true is returned, if not false is returned.
     */
    bool AspiDriver::timeout(long timeout)
    {
        return true;
    }

    /**
     * Scans the system for devices.
     * @param [out] addresses Vector containing addresses of all detected
     *                        disc devices.
     * @return If successful true is returned, if not false is returned.
     */
    bool AspiDriver::scan(std::vector<ScsiDevice::Address> &addresses)
    {
        // Make sure that the driver DLL is loaded.
        if (!driver_loaded_)
        {
            driver_loaded_ = driver_load();
            if (!driver_loaded_)
                return false;
        }

        int num_adapters = LOBYTE(LOWORD(GetASPI32SupportInfo()));
        for (int adapter = 0; adapter < num_adapters; adapter++)
        {
            SRB_HaInquiry ha_inq;
            memset(&ha_inq,0,sizeof(SRB_HaInquiry));
            ha_inq.SRB_Cmd = SC_HA_INQUIRY;
            ha_inq.SRB_HaId = adapter;

            // Make sure that the bus is available.
            unsigned long status = SendASPI32Command((LPSRB)&ha_inq);
            if (ha_inq.SRB_Status != SS_COMP)
                continue;

            int num_targets = ha_inq.HA_Unique[3];
            for (int target = 0; target < num_targets; target++)
            {
                for (int lun = 0; lun < 8; lun++)
                {
                    SRB_GDEVBlock dev_block;
                    memset(&dev_block,0,sizeof(SRB_GDEVBlock));
                    dev_block.SRB_Cmd = SC_GET_DEV_TYPE;
                    dev_block.SRB_HaId  = adapter;
                    dev_block.SRB_Target = target;
                    dev_block.SRB_Lun = lun;

                    // Obtain the type of the current device. If CD-ROM, save
                    // the address.
                    unsigned long status = SendASPI32Command((LPSRB)&dev_block);
                    if (dev_block.SRB_Status == SS_COMP && dev_block.SRB_DeviceType == DT_CDROM)
                    {
                        ScsiDevice::Address addr;
                        addr.bus_ = adapter;
                        addr.target_ = target;
                        addr.lun_ = lun;

                        addresses.push_back(addr);
                    }
                }
            }
        }

        return true;
    }

    /**
     * Transports data from or to the device using SCSI commands.
     * @param [in] device The device to transport the command to.
     * @param [in] cdb Buffer to command descriptor block.
     * @param [in] cdb_len Length of the command descriptor block.
     * @param [in] data Pointer to data buffer for either receiving or
     *                  writing data.
     * @param [in] data_len Length of the data buffer.
     * @param [in] mode Specifies the transport mode.
     * @return If the transport was successfully carried through true is
     *         returned, if not false is returned.
     */
    bool AspiDriver::transport(ScsiDevice &device,
                               unsigned char *cdb,unsigned char cdb_len,
                               unsigned char *data,unsigned long data_len,
                               ScsiDevice::TransportMode mode)
    {
        // Make sure that the driver DLL is loaded.
        if (!driver_loaded_)
        {
            driver_loaded_ = driver_load();
            if (!driver_loaded_)
                return false;
        }

        if (device.address().bus_ == -1 ||
            device.address().target_ == -1 ||
            device.address().lun_ == -1)
        {
            return false;
        }

        if (cdb == NULL || cdb_len > 16)
            return false;

        // Prepare SCSI command.
        SRB_ExecSCSICmd srb_cmd;
        memset(&srb_cmd,0,sizeof(SRB_ExecSCSICmd));

        srb_cmd.SRB_Cmd = SC_EXEC_SCSI_CMD;
        srb_cmd.SRB_HaId = static_cast<BYTE>(device.address().bus_);
        srb_cmd.SRB_Target = static_cast<BYTE>(device.address().target_);
        srb_cmd.SRB_Lun = static_cast<BYTE>(device.address().lun_);
        srb_cmd.SRB_SenseLen = 24;
        srb_cmd.SRB_BufPointer = data;
        srb_cmd.SRB_BufLen = data_len;
        srb_cmd.SRB_CDBLen = cdb_len;
        memcpy(srb_cmd.CDBByte,cdb,cdb_len);

        switch (mode)
        {
            case ScsiDevice::ckTM_UNSPECIFIED:
                srb_cmd.SRB_Flags = 0;
                break;

            case ScsiDevice::ckTM_READ:
                srb_cmd.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
                break;

            case ScsiDevice::ckTM_WRITE:
                srb_cmd.SRB_Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
                break;

            default:
                return false;
        }

        // Setup wait event.
        HANDLE wait_event = CreateEvent(NULL,TRUE,FALSE,NULL);
        ResetEvent(wait_event);
        srb_cmd.SRB_PostProc = (void (__cdecl *)(void))wait_event;

        // Execute SCSI command and wait for it to finish.
        if (SendASPI32Command((LPSRB)&srb_cmd) == SS_PENDING)
            WaitForSingleObject(wait_event,INFINITE);

        CloseHandle(wait_event);

        // Verify the commnad result.
        if (srb_cmd.SRB_Status != SS_COMP)
        {
            if (!silent_)
            {
                ckcore::log::print_line(ckT("[aspidriver]: SendASPI32Command failed (0x%.2x, %d)."),
                                        srb_cmd.SRB_Status,GetLastError());
            }

            return false;
        }

        if (srb_cmd.SRB_TargStat != ScsiDevice::ckSCSISTAT_GOOD)
        {
            if (!silent_)
            {
                ckcore::log::print_line(ckT("[aspidriver]: scsi command failed (0x%.2x)."),
                                        srb_cmd.SRB_TargStat);

                // Dump CDB.
                ckcore::log::print(ckT("[aspidriver]: > cdb: "));
                for (unsigned int i = 0; i < cdb_len; i++)
                {
                    if (i == 0)
                        ckcore::log::print(ckT("0x%.2x"),cdb[i]);
                    else
                        ckcore::log::print(ckT(",0x%.2x"),cdb[i]);
                }

                ckcore::log::print_line(ckT(""));

                // Dump sense information.
                ckcore::log::print_line(ckT("[aspidriver]: > sense key: 0x%x"),srb_cmd.SenseArea[2] & 0xf);
                ckcore::log::print_line(ckT("[aspidriver]: > asc: 0x%.2x"),srb_cmd.SenseArea[12]);
                ckcore::log::print_line(ckT("[aspidriver]: > ascq: 0x%.2x"),srb_cmd.SenseArea[13]);
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
     *                  writing data.
     * @param [in] data_len Length of the data buffer.
     * @param [in] mode Specifies the transport mode.
     * @param [out] sense Pointer to sense buffer.
     * @param [out] result Contains the transport result.
     * @return If the transport was successfully carried through true is
     *         returned, if not false is returned.
     */
    bool AspiDriver::transport_with_sense(ScsiDevice &device,
                                          unsigned char *cdb,unsigned char cdb_len,
                                          unsigned char *data,unsigned long data_len,
                                          ScsiDevice::TransportMode mode,
                                          unsigned char *sense,unsigned char &result)
    {
        // Make sure that the driver DLL is loaded.
        if (!driver_loaded_)
        {
            driver_loaded_ = driver_load();
            if (!driver_loaded_)
                return false;
        }

        if (device.address().bus_ == -1 ||
            device.address().target_ == -1 ||
            device.address().lun_ == -1)
        {
            return false;
        }

        if (cdb == NULL || cdb_len > 16)
            return false;

        if (sense == NULL)
            return false;

        // Prepare SCSI command.
        SRB_ExecSCSICmd srb_cmd;
        memset(&srb_cmd,0,sizeof(SRB_ExecSCSICmd));

        srb_cmd.SRB_Cmd = SC_EXEC_SCSI_CMD;
        srb_cmd.SRB_HaId = static_cast<BYTE>(device.address().bus_);
        srb_cmd.SRB_Target = static_cast<BYTE>(device.address().target_);
        srb_cmd.SRB_Lun = static_cast<BYTE>(device.address().lun_);
        srb_cmd.SRB_SenseLen = 24;
        srb_cmd.SRB_BufPointer = data;
        srb_cmd.SRB_BufLen = data_len;
        srb_cmd.SRB_CDBLen = cdb_len;
        memcpy(srb_cmd.CDBByte,cdb,cdb_len);

        switch (mode)
        {
            case ScsiDevice::ckTM_UNSPECIFIED:
                srb_cmd.SRB_Flags = 0;
                break;

            case ScsiDevice::ckTM_READ:
                srb_cmd.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
                break;

            case ScsiDevice::ckTM_WRITE:
                srb_cmd.SRB_Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
                break;

            default:
                return false;
        }

        // Setup wait event.
        HANDLE wait_event = CreateEvent(NULL,TRUE,FALSE,NULL);
        ResetEvent(wait_event);
        srb_cmd.SRB_PostProc = (void (__cdecl *)(void))wait_event;

        // Execute SCSI command and wait for it to finish.
        if (SendASPI32Command((LPSRB)&srb_cmd) == SS_PENDING)
            WaitForSingleObject(wait_event,INFINITE);

        CloseHandle(wait_event);

        memcpy(sense,srb_cmd.SenseArea,24);
        result = srb_cmd.SRB_TargStat;

        return true;
    }
};
