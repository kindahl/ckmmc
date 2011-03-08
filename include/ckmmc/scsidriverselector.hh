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
 * @file include/ckmmc/scsidriverselector.hh
 * @brief Defines the SCSI driver selector class.
 */

#pragma once
#include <ckcore/types.hh>
#include "ckmmc/scsidriver.hh"

namespace ckmmc
{
    /**
     * @brief Class for selecting and obtaining the SCSI driver instance.
     */
    class ScsiDriverSelector
    {
    private:
        ScsiDriverSelector();
        ScsiDriverSelector(const ScsiDriverSelector &obj);
        ~ScsiDriverSelector();
        ScsiDriverSelector &operator=(const ScsiDriverSelector &rhs);

    public:
        static ScsiDriver &driver();
    };
};
