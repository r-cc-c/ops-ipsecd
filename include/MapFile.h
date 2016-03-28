
/*
 *Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
 *All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */

#ifndef MAPFILE_H
#define MAPFILE_H

/**********************************
*System Includes
**********************************/

/**********************************
*Local Includes
**********************************/
#include "IMapFile.h"
#include "ISystemCalls.h"

/**
 * Class for to Map the Files
 */
class MapFile : public IMapFile
{
    protected:

        /**
         * System Calls Interface
         */
        ISystemCalls& m_SystemCalls;

        /**
         * Size of the file that was map
         */
        uint32_t m_size = 0;

        /**
         * Map File
         */
        void* m_map_file = nullptr;

        /**
         * @copydoc IMapFile::unmap_file
         */
        void unmap_file() override;

    public:

        /**
         * Default Constructor
         *
         * @param systemCalls SystemCall Interface
         */
        MapFile(ISystemCalls& systemCalls);

        /**
         * Default Destructor
         */
        virtual ~MapFile();

        /**
         * @copydoc IMapFile::map_file
         */
        ipsec_ret map_file(const std::string& filepath) override;

        /**
         * @copydoc IMapFile::get_map_file
         */
        inline const void* get_map_file() const override
        {
            return m_map_file;
        }

        /**
         * @copydoc IMapFile::get_size
         */
        inline uint32_t get_size() const override
        {
            return m_size;
        }
};

#endif /* MAPFILE_H */