#include <iostream>
#include <ophidian/design/DesignFactory.h>
#include <ophidian/routing/ILPRouting.h>
#include <ophidian/routing/AStarRouting.h>
#include <ophidian/parser/ICCAD2020Writer.h>
#include <ophidian/util/log.h>
#include <ophidian/routing/MCFMultiThreading.h>

using namespace std;
using namespace ophidian::util;
// int THREADS_DEFAULT_VALUE = 1;

void greetings(){
    using std::endl;

    printlog("===================================================");
    printlog("              ICCAD 2020 CAD Contest               ");
    printlog("      Problem B : Routing with Cell Movement       ");
    printlog("            Team Number:   cada0194                ");
    printlog("              Team Name:      CABRA                ");
    printlog("                    Members:                       ");
    printlog("Affiliation:  University of Calgary                ");
    printlog("    Erfan Aghaeekiasaraee                          ");
    printlog("    Upma Gandhi                                    ");
    printlog("    Laleh Behjat                                   ");
    printlog("Affiliation:  Federal University of Santa Catarina ");
    printlog("    Arthur Philippi Bianco                         ");
    printlog("    Renan Oliveira Netto                           ");
    printlog("    Sheiny Fabre Almeida                           ");
    printlog("    Tiago Augusto Fontana                          ");
    printlog("    Vinicius Livramento                            ");
    printlog("    Cristina Meinhardt                             ");
    printlog("    Jose Luis Guntzel                              ");
    printlog("===================================================");
};

void run_mcf_for_circuit(ophidian::design::Design & design, std::string circuit_name){
    /*ophidian::routing::AStarRouting astar_routing{design};
    auto& netlist = design.netlist();
    for(auto net_it = netlist.begin_net(); net_it != netlist.end_net(); net_it++) {
        auto net = *net_it;
        auto net_name = design.netlist().name(net);
        std::cout << "net " << net_name << std::endl;
        design.global_routing().unroute(net);
        std::vector<ophidian::routing::AStarSegment> segments;    
        astar_routing.route_net(net, segments);
    }*/

    // UCal::MCFRouting mcf_routing(design,circuit_name);
    ophidian::parser::ICCAD2020Writer iccad_output_writer(design, circuit_name);
    std::vector<ophidian::circuit::Net> nets(design.netlist().begin_net(), design.netlist().end_net());
    std::vector<ophidian::circuit::Net> fixed_nets;
    std::vector<ophidian::circuit::Net> routed_nets;

    UCal::MCFMultiThreading mcf_multi_threading(design); 
    mcf_multi_threading.run();

    
}//end run_mcf_for_circuit

void run_for_circuit(ophidian::design::Design & design, std::string circuit_name, std::string output) {
    

    ophidian::routing::ILPRouting<IloBoolVar> ilpRouting(design, circuit_name);
    //ophidian::routing::ILPRouting<IloNumVar> ilpRouting(design, circuit_name);
    ophidian::routing::AStarRouting astar_routing{design};
    ophidian::parser::ICCAD2020Writer iccad_output_writer(design, circuit_name);

    auto chip_origin = design.floorplan().chip_origin();
    auto chip_upper_right_corner = design.floorplan().chip_upper_right_corner();
    auto chip_area = ophidian::placement::Placement::box_type{chip_origin, chip_upper_right_corner};

    std::vector<ophidian::circuit::Net> nets(design.netlist().begin_net(), design.netlist().end_net());
    std::vector<ophidian::circuit::CellInstance> cells(design.netlist().begin_cell_instance(), design.netlist().end_cell_instance());
    std::vector<ophidian::circuit::Net> fixed_nets;
    std::vector<ophidian::circuit::Net> routed_nets;
    std::vector<ophidian::circuit::Net> unrouted_nets;

    std::vector<std::pair<ophidian::routing::ILPRouting<IloBoolVar>::cell_type, ophidian::routing::ILPRouting<IloBoolVar>::point_type>> movements; 
    // std::log() << "routing nets" << std::endl;
    auto result = ilpRouting.route_nets(nets, cells, chip_area, fixed_nets, routed_nets, unrouted_nets, movements);

    /*std::vector<ophidian::circuit::Net> bad_nets;
    for (auto net : nets) {
        auto pins = design.netlist().pins(net);
        std::vector<ophidian::interconnection::Flute::Point> net_points;
        net_points.reserve(pins.size());
        for (auto pin : pins) {
            auto pin_location = design.placement().location(pin);
            net_points.push_back(pin_location);
        }

        auto & flute = ophidian::interconnection::Flute::instance();
        auto tree = flute.create(net_points);
        auto stwl = tree->length().value();
        stwl /= 10;
        if (stwl == 0) {
            stwl = 1;
        }

        auto routes = design.global_routing().segments(net);
        auto routed_length_no_vias = 0;
        auto via_length = 0;
        for (auto route : routes) {
            auto box = design.global_routing().box(route);
            auto start = box.min_corner();
            auto end = box.max_corner();
            routed_length_no_vias += (std::abs(start.x().value() - end.x().value()) + std::abs(start.y().value() - end.y().value()));

            auto start_layer = design.global_routing().layer_start(route);
            auto end_layer = design.global_routing().layer_end(route);
            auto start_layer_index = design.routing_library().layerIndex(start_layer);
            auto end_layer_index = design.routing_library().layerIndex(end_layer);
            via_length += std::abs(start_layer_index - end_layer_index);
        }
        routed_length_no_vias /= 10;
        if (routed_length_no_vias == 0) {
            routed_length_no_vias = 1;
        }

        auto routed_length = design.global_routing().wirelength(net);

        auto cost = routed_length / stwl;
        //auto cost = routed_length_no_vias / stwl;

        if (cost > 2) {
            bad_nets.push_back(net);
        }
    }

    std::cout << bad_nets.size() << " bad nets" << std::endl;*/
    
    /*std::cout << unrouted_nets.size() << " unrouted nets" << std::endl;

    // std::log() << "result " << result << std::endl;   
    auto& netlist = design.netlist();
    //for(auto net_it = netlist.begin_net(); net_it != netlist.end_net(); net_it++) {
    //    auto net = *net_it;
    //for(auto net : bad_nets) {
    for(auto net : unrouted_nets) {
        auto net_name = design.netlist().name(net);
        std::cout << "net " << net_name << std::endl;
        design.global_routing().unroute(net);
        std::vector<ophidian::routing::AStarSegment> segments;    
        astar_routing.route_net(net, segments);
    }*/
    //for (auto net : unrouted_nets) {
    //    astar_routing.route_net(net, segments);
    //}

    iccad_output_writer.write_ICCAD_2020_output(output, movements);
    // if(result.first){
    //     iccad_output_writer.write_ICCAD_2020_output(output, movements);
    // }

    // std::log() << "connected nets" << std::endl;
    // for (auto net : nets) {
    //     ophidian::routing::GlobalRouting::gcell_container_type pin_gcells = {};
    //     for (auto pin : design.netlist().pins(net)) {
    //         auto pin_name = design.netlist().name(pin);                
    //         auto location = design.placement().location(pin);
    //         auto box = ophidian::routing::GCellGraph::box_type{location, location};
    //         auto pin_geometry = design.placement().geometry(pin);
    //         auto layer_name = pin_geometry.front().second;
    //         auto pin_layer = design.routing_library().find_layer_instance(layer_name);
    //         auto layer_index = design.routing_library().layerIndex(pin_layer);

    //         // std::log() << "pin " << pin_name << " layer " << layer_name << " index " << layer_index << std::endl;

    //         design.global_routing().gcell_graph()->intersect(pin_gcells, box, layer_index-1);
    //     }
    //     auto connected = design.global_routing().is_connected(net, pin_gcells);

    //     auto net_name = design.netlist().name(net);
    //     if(!connected)
    //         std::log() << "net " << net_name << " is open" << std::endl;
    // }
}

bool test_input(const std::string input_file)
{
    if(input_file.find(".txt") != std::string::npos)
        return true;
    return false;
}

std::string extract_circuit_name(const std::string input_file)
{
    // Find the position of first delimiter ( last / )
    int firstDelPos = input_file.rfind("/");
    // Find the position of second delimiter
    int secondDelPos = input_file.rfind(".txt");
    auto circuit = input_file.substr(firstDelPos + 1, secondDelPos - firstDelPos - 1);
    return circuit;
}

int main(int argc, char** argv) {

    greetings();

    bool input_found{false};
    string input_file{};
    string circuit_name{};

    bool output_found{false};
    string output{};

    // HELP
    if (argc == 1 || string(argv[1]) == "-h" || string(argv[1]) == "-help" || string(argv[1]) == "--help")
    {
        log() << "usage:" << endl;
        log() << "./cell_move_router <input.txt> <output.txt>" << endl;
        return 0;
    }

    if (argc < 2)
    {
        log() << "Cannot proceed, missing inputn and/or output file name" << endl;
        log() << "For help, set --help or -help or -h" << endl;
        log() << "usage: ./cell_move_router <input.txt> <output.txt>" << endl;
        return 0;
    } 

    if (argv[1])
    {
        input_file = argv[1];
        if(test_input(input_file))
        {
            input_found = true;
            circuit_name = extract_circuit_name(input_file);
        }
    }

    if (argv[2])
    {
        output = argv[2];
        if(test_input(output))
        {
            output_found = true;
        }
    }

    // must have flags:
    if (input_file == "")
    {
        log() << "Cannot proceed, missing Input file" << endl;
        log() << "For help, set --help or -help or -h" << endl;
        return 0;
    }

    if (output == "")
    {
        log() << "Cannot proceed, missing output file name" << endl;
        log() << "For help, set --help or -help or -h" << endl;
        return 0;
    }

    auto iccad_2020 = ophidian::parser::ICCAD2020{input_file};

    auto design = ophidian::design::Design();
    ophidian::design::factory::make_design_iccad2020(design, iccad_2020);
    
    //run_for_circuit(design, circuit_name, output);
    run_mcf_for_circuit(design,circuit_name);

    return 0;
}
