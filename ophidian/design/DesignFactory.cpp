/*
 * Copyright 2017 Ophidian
   Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
   distributed with this work for additional information
   regarding copyright ownership.  The ASF licenses this file
   to you under the Apache License, Version 2.0 (the
   "License"); you may not use this file except in compliance
   with the License.  You may obtain a copy of the License at
   http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.
 */

#include "Design.h"

#include <ophidian/floorplan/FloorplanFactory.h>
#include <ophidian/placement/PlacementFactory.h>
#include <ophidian/circuit/Def2Netlist.h>
#include <ophidian/circuit/NetlistFactory.h>
#include <ophidian/placement/Lef2Library.h>

namespace ophidian
{
namespace design
{
    namespace factory 
    {
        Design make_design_iccad2017(const parser::Def& def, const parser::Lef& lef) noexcept
        {
            auto design = Design{};

//            placement::def2placement(def, design.placement(), design.netlist());
            design.floorplan() = floorplan::factory::make_floorplan(lef, def);
            placement::lef2Library(lef, design.library(), design.standard_cells());
//            circuit::def2Netlist(
//                def,
//                design.netlist(),
//                design.standard_cells());

            return design;
        }

        Design make_design_iccad2015(const parser::Def& def, const parser::Lef& lef, const parser::Verilog& verilog) noexcept
        {
            auto design = Design{};

            placement::lef2Library(lef, design.library(), design.standard_cells());
            design.floorplan() = floorplan::factory::make_floorplan(lef, def);
//            placement::def2placement(def, design.placement(), design.netlist());
//            circuit::verilog2Netlist(verilog, design.netlist());

            return design;
        }
    }
}
}
