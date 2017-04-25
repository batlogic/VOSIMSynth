/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "vosimlib/common.h"
#include <vector>
#include <unordered_map>

namespace syn
{
    class Unit;

    class VOSIMLIB_API UnitFactory
    {
        UnitFactory() {}

        struct FactoryPrototype
        {
            FactoryPrototype(std::string a_group_name, Unit* a_unit, size_t a_class_size);

            UnitTypeId classIdentifier;
            std::string group_name;
            std::string name;
            Unit* prototype;
            int build_count;
            size_t size;
        };

    public:

        UnitFactory(const UnitFactory&) = delete;
        void operator=(const UnitFactory&) = delete;

        static UnitFactory& instance() {
            static UnitFactory singleton;
            return singleton;
        }

        virtual ~UnitFactory();

        /**
         * \brief Register a prototype unit with the factory. Prototype deletion will be taken care of upon
         * factory destruction.
         */
        template <typename T, typename = std::enable_if_t<std::is_base_of<Unit, T>::value>>
        void addUnitPrototype(const std::string& a_group_name, const std::string& a_unit_name);

        const std::vector<std::string>& getGroupNames() const;

        std::vector<std::string> getPrototypeNames(const std::string& group) const;

        std::vector<std::string> getPrototypeNames() const;

        const FactoryPrototype* getFactoryPrototype(UnitTypeId a_classIdentifier) const;

        Unit* createUnit(UnitTypeId a_classIdentifier, const std::string& a_name = "");

        std::string generateUnitName(UnitTypeId a_classIdentifier) const;

        bool hasClassId(UnitTypeId a_classIdentifier) const;

        /**
         * \returns -1 on failure.
         */
        UnitTypeId getClassId(const std::string& a_groupName, const std::string& a_protoName) const;

        void resetBuildCounts();

    protected:
        int getPrototypeIdx_(const std::string& a_name) const;

        int getPrototypeIdx_(UnitTypeId a_classId) const;
        
        /**
         * Fallback for finding prototypes by 32-bit id (instead of UnitTypeId). Probably not very safe.         
         */
        int getPrototypeIdx_(uint32_t a_classId) const;

        Unit* createUnit_(int a_protoNum, const std::string& a_name);

    private:
        std::vector<FactoryPrototype> m_prototypes;
        std::vector<std::string> m_group_names;

        std::unordered_map<UnitTypeId, int> m_class_identifiers; //<! mapping from unique class IDs to prototype numbers
    };

    template <typename T, typename>
    void UnitFactory::addUnitPrototype(const std::string& a_group_name, const std::string& a_unit_name) {
        FactoryPrototype prototype{a_group_name, new T(a_unit_name), sizeof(T)};
        // Add prototype
        if (m_class_identifiers.find(prototype.classIdentifier) != m_class_identifiers.end())
            return;
        m_prototypes.push_back(prototype);
        m_class_identifiers[prototype.classIdentifier] = (int)m_prototypes.size() - 1;

        // Add group
        if (std::find(m_group_names.begin(), m_group_names.end(), a_group_name) != m_group_names.end())
            return;
        if (prototype.group_name.empty())
            return;
        m_group_names.push_back(prototype.group_name);        
    }
}
