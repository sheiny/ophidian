#include "def2placement_test.h"
#include <catch.hpp>

#include <ophidian/placement/Def2Placement.h>

using namespace ophidian;

class Def2PlacementFixture
{
public:
	Def2PlacementFixture() : placement(netlist){
		parser::DefParser parser;
		def = std::move(parser.readFile("./input_files/simple.def"));
	}

	std::unique_ptr<parser::Def> def;
	circuit::Netlist netlist;
	placement::Placement placement;
};

TEST_CASE_METHOD(Def2PlacementFixture, "Def2Placement: Test if def2placement mapped cells's position correctly according .def file.", "[placement][Def]")
{
	ophidian::placement::def2placement(*def, placement, netlist);

	REQUIRE(placement.location(netlist.find(circuit::Cell(), "u1")) == util::LocationMicron(3420, 6840));
	REQUIRE(placement.location(netlist.find(circuit::Cell(), "u2")) == util::LocationMicron(3420, 3420));
	REQUIRE(placement.location(netlist.find(circuit::Cell(), "u3")) == util::LocationMicron(6840, 3420));
	REQUIRE(placement.location(netlist.find(circuit::Cell(), "u4")) == util::LocationMicron(12160, 6840));
	REQUIRE(placement.location(netlist.find(circuit::Cell(), "lcb1")) == util::LocationMicron(0, 10260));
}

