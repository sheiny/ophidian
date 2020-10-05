#include <catch.hpp>
// #include < stdio.h >
#include <ophidian/design/DesignFactory.h>
#include <ophidian/routing/ILPRouting.h>
#include <ophidian/parser/GuideWriter.h>
#include <ophidian/parser/DefWriter.h>
#include <ophidian/routing/Engine.h>
#include "run_ilp.h"

bool check_connectivity(const ophidian::design::Design & design, const std::vector<ophidian::circuit::Net>& nets){
    log() << "Checking for disconnected nets..." << std::endl;
    bool is_nets_open = false;

    namespace bgi = boost::geometry::index;
    using box_scalar_type       = ophidian::geometry::Box<double>;
    using rtree_node_type       = std::pair<box_scalar_type, ophidian::routing::GlobalRouting::gr_segment_type>;
    using rtree_type            = boost::geometry::index::rtree<rtree_node_type, boost::geometry::index::rstar<16> >;
    std::unordered_map<int, rtree_type> rtree_layers;

    for (auto segment_it = design.global_routing().begin_segment(); segment_it != design.global_routing().end_segment(); segment_it++) {
        auto segment = *segment_it;

        auto gcell_start = design.global_routing().gcell_start(segment);
        auto gcell_end = design.global_routing().gcell_end(segment);

        auto gcell_start_box = design.global_routing().gcell_graph()->box(gcell_start);
        auto gcell_end_box = design.global_routing().gcell_graph()->box(gcell_end);

        auto gcell_start_scalar_box = box_scalar_type{{gcell_start_box.min_corner().x().value(), gcell_start_box.min_corner().y().value()}, {gcell_start_box.max_corner().x().value(), gcell_start_box.max_corner().y().value()}};
        auto gcell_end_scalar_box = box_scalar_type{{gcell_end_box.min_corner().x().value(), gcell_end_box.min_corner().y().value()}, {gcell_end_box.max_corner().x().value(), gcell_end_box.max_corner().y().value()}};

        auto layer_start = design.global_routing().gcell_graph()->layer_index(gcell_start);
        auto layer_end = design.global_routing().gcell_graph()->layer_index(gcell_end);

        auto node_start = rtree_node_type{gcell_start_scalar_box, segment};
        auto node_end = rtree_node_type{gcell_end_scalar_box, segment};

        rtree_layers[layer_start].insert(node_start);
        rtree_layers[layer_end].insert(node_end);
    }

    for (auto net : nets) {
        auto net_name = design.netlist().name(net);
        ophidian::routing::GlobalRouting::gcell_container_type pin_gcells = {};
        auto pins_connected = true;
        for (auto pin : design.netlist().pins(net)) {
            auto pin_name = design.netlist().name(pin);
            auto pin_owner = design.netlist().cell(pin);
            //auto location = (pin_owner != ophidian::circuit::CellInstance()) ? design.placement().location(pin_owner) : design.placement().location(pin);
            auto location = design.placement().location(pin);
            auto box = ophidian::routing::GCellGraph::box_type{location, location};
            auto pin_geometry = design.placement().geometry(pin);
            auto layer_name = pin_geometry.front().second;
            auto pin_layer = design.routing_library().find_layer_instance(layer_name);
            auto layer_index = design.routing_library().layerIndex(pin_layer);

            // log() << "pin " << pin_name << " layer " << layer_name << " index " << layer_index << std::endl;

            design.global_routing().gcell_graph()->intersect(pin_gcells, box, layer_index-1);

            for (auto pin_box : pin_geometry) {
                auto box_layer_name = pin_box.second;
                auto box_layer = design.routing_library().find_layer_instance(box_layer_name);
                auto box_layer_index = design.routing_library().layerIndex(box_layer);

                auto pin_box_scalar = box_scalar_type{{pin_box.first.min_corner().x().value(), pin_box.first.min_corner().y().value()}, {pin_box.first.max_corner().x().value(), pin_box.first.max_corner().y().value()}};

                std::vector<rtree_node_type> intersecting_nodes;
                rtree_layers[box_layer_index].query(boost::geometry::index::intersects(pin_box_scalar), std::back_inserter(intersecting_nodes));

                pins_connected &= !intersecting_nodes.empty();
            }
        }
        if(false){ //debug code
            for(auto gcell : pin_gcells)
            {
                auto box = design.global_routing().gcell_graph()->box(gcell);
                auto layer = design.global_routing().gcell_graph()->layer_index(gcell);
                auto layer_i = design.routing_library().layer_from_index(layer);
                auto layer_str = design.routing_library().name(layer_i);
                std::cout << box.min_corner().x().value() << " " << box.min_corner().y().value() << " " << box.max_corner().x().value() << " " << box.max_corner().y().value() << " " << layer_str << std::endl;
            }
        }

        auto connected = design.global_routing().is_connected(net, pin_gcells, net_name) && pins_connected;
        
        if(!connected)
        {
            log() << "net " << net_name << " disconnected with " << design.netlist().pins(net).size() << " pins" << std::endl;
            is_nets_open = true;
        }
    }
    if (is_nets_open) printlog("Open nets detected!"); else printlog("All nets connected!");
    return is_nets_open;
}

TEST_CASE("run ILP for iccad19 benchmarks", "[DATE21]") {

    //iccad 2019 benchmarks
    std::vector<std::string> circuit_names = {
        // "ispd18_sample3",
        // "ispd19_sample4",

        "ispd19_test1",
        // "ispd18_test8",
        // "ispd18_test10",
        // "ispd19_test7",
        // "ispd19_test8",
        // "ispd19_test9"
    };

    // std::string benchmarks_path = "./input_files/ispd19";
    std::string benchmarks_path = "./input_files/circuits";

    for (auto circuit_name : circuit_names) {
        std::cout << "running circuit " << circuit_name << std::endl;

         std::string def_file =   benchmarks_path + "/" + circuit_name + "/" + circuit_name + ".input.def";
         std::string lef_file =   benchmarks_path + "/" + circuit_name + "/" + circuit_name + ".input.lef";
        //std::string def_file =   benchmarks_path + "/" + circuit_name + ".input.def";
        //std::string lef_file =   benchmarks_path + "/" + circuit_name + ".input.lef";
        std::string guide_file = benchmarks_path + "/cu_gr_solution/" + circuit_name + ".solution_cugr.guide";
        // std::string guide_file = "./" + circuit_name + "_astar.guide";

        ophidian::parser::Def def;
        ophidian::parser::Lef lef;
        ophidian::parser::Guide guide;
        // #pragma omp parallel
        // {
            def = ophidian::parser::Def{def_file};
            lef = ophidian::parser::Lef{lef_file};
            guide = ophidian::parser::Guide{guide_file};
        // }

        auto design = ophidian::design::Design();
        ophidian::design::factory::make_design(design, def, lef, guide);

        UCal::Engine engine(design);
        std::vector<ophidian::circuit::Net> nets(design.netlist().begin_net(), design.netlist().end_net());
        // std::vector<ophidian::circuit::Net> nets;
        // auto net2037 = design.netlist().find_net("n_2037");
        // nets.push_back(net2037);
        
        log() << "Initial wirelength = " << design.global_routing().wirelength(nets) << std::endl;
        log() << "A* for generate the initial solution" << std::endl;
        //clear the possible initial solution
        for (auto net : nets) {
            design.global_routing().unroute(net);
        }
        log() << "Cleaned wirelength = " << design.global_routing().wirelength(nets) << std::endl;


        auto & netlist = design.netlist();
        auto & placement = design.placement();
        std::vector<std::pair<ophidian::circuit::Net, double>> nets_bounding_box;
        for(auto net : nets){

            double min_x = std::numeric_limits<double>::max();
            double min_y = std::numeric_limits<double>::max();
            double max_x = std::numeric_limits<double>::min();
            double max_y = std::numeric_limits<double>::min();

            for(auto net_pin : netlist.pins(net)){
                auto location = placement.location(net_pin);
                min_x = std::min(min_x, location.x().value());
                max_x = std::max(max_x, location.x().value());
                min_y = std::min(min_y, location.y().value());
                max_y = std::max(max_y, location.y().value());
            }

            auto bounding_box = (max_x - min_x) + (max_y - min_y);
            nets_bounding_box.push_back(std::make_pair(net, bounding_box));
        }
        std::sort(nets_bounding_box.begin(), nets_bounding_box.end(), [](std::pair<ophidian::circuit::Net, double> cost_a, std::pair<ophidian::circuit::Net, double> cost_b) {return cost_a.second < cost_b.second;});
        std::vector<ophidian::circuit::Net> ordered_nets;
        for(auto pair : nets_bounding_box)
        {
            ordered_nets.push_back(pair.first);
        }
        
        auto start_time = std::chrono::steady_clock::now();
        //engine.run_astar_on_circuit(ordered_nets);
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end_time-start_time;
        log() << "A* wirelength = " << design.global_routing().wirelength(nets) << std::endl;

        log() << "Total number of vias = "<< design.global_routing().number_of_vias(nets) << std::endl;
        
        log() << "initial solution generated in " << diff.count() << " seconds" << std::endl;

        log() << "Number of vias = " << design.global_routing().number_of_vias(nets) << std::endl;

        // check_connectivity(design, nets);
        if(check_connectivity(design, nets)){
            ophidian::parser::write_guide(design, circuit_name + "_astar.guide");
        }

        std::ofstream output_file;
        output_file.open(circuit_name + ".csv", std::ios::out|std::ios::trunc);
        output_file << "net_name,wirelength,vias" << std::endl;
        for(auto net : nets){
            auto net_name = netlist.name(net);
            auto wl = design.global_routing().wirelength(net);
            auto vias = design.global_routing().number_of_vias(net);
            output_file << net_name << "," << wl << "," << vias << std::endl;
        }
        output_file.close();

        //ILP lower panels with movement 

        //A* upper panels with movement

        //A* all circuit with movements

        //write new placement in the .def file
        // ophidian::parser::write_replaced_def(design, def_file+"", circuit_name + "_out.def");
        //write GR result in guide file
        // ophidian::parser::write_guide(design, circuit_name + "_out.guide");    
        std::cout << "Memory usage in peak= " << ophidian::util::mem_use::get_peak() << " MB" << std::endl;    
    }
}
//if not specified the net name, it draws the whole circuit ((not recommended)
void draw_gcell_svg_2(ophidian::design::Design & design, std::string net_name){
    auto& netlist = design.netlist();
    auto& routing_library = design.routing_library();
    auto& global_routing = design.global_routing();
    auto gcell_graph_ptr = global_routing.gcell_graph();
    std::ofstream out_svg;
    if (net_name == "")
        out_svg.open("output.svg");
    else
        out_svg.open(net_name + ".svg");

    //layer index
    std::unordered_map<int, std::string> layer2color = {
        {1,"#0000ff"},//blue
        {2,"#ff0000"},//red
        {3,"#00d000"},//green
        {4,"#d0d000"},//yellow
        {5,"#a52a2a"},//dark red
        {6,"#ffa500"},//orange
        {7,"#d000d0"},//pink
        {8,"#00d0d0"},//light blue
        {9,"#a52a2a"},//brown
        {10,"#ffff00"},//light yellow
        {11,"#008000"},//dark green
        {12,"#ff00ff"},//purple
        {13,"#ffc0cb"},//light pink
        {14,"#00ffff"},//light blue
        {15,"#800080"},//dark purple
        {16,"#808000"},//dark yellow
    };

    out_svg<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    out_svg<<"<svg>\n";

    for(auto layer_color : layer2color)
    {
        out_svg<<"<g\n";
        out_svg<<"inkscape:label=\""<<layer_color.first<<"\"\n";
        out_svg<<"inkscape:groupmode=\"layer\"\n";
        out_svg<<"id=\""<<layer_color.first<<"\">\n";
        for(auto net_it = netlist.begin_net(); net_it != netlist.end_net(); ++net_it)
        {
            if(net_name != "" && netlist.name(*net_it) != net_name)
                continue;

            for(auto gcell : global_routing.gcells(*net_it))
            {
                if(gcell_graph_ptr->layer_index(gcell) != layer_color.first)
                    continue;

                auto box = gcell_graph_ptr->box(gcell);
                auto width = box.max_corner().x() - box.min_corner().x();
                auto height = box.max_corner().y() - box.min_corner().y();

                out_svg<<"<rect\n";
                out_svg<<"style=\"fill:"<<layer_color.second<<";fill-opacity:0.5;\"\n";
                out_svg<<"width=\""<<units::unit_cast<double>(width) / 1000<<"\"\n";
                out_svg<<"height=\""<<units::unit_cast<double>(height) / 1000<<"\"\n";
                out_svg<<"x=\""<<units::unit_cast<double>(box.min_corner().x()) / 1000<<"\"\n";
                out_svg<<"y=\""<<units::unit_cast<double>(-box.max_corner().y()) / 1000<<"\" />\n";//svg files use y axis flipped
            }
        }
        out_svg<<"</g>\n";
    }
    out_svg<<"</svg>";
    out_svg.close();
}

TEST_CASE("run iccad19 benchmarks", "[connectivity]") {

    //iccad 2019 benchmarks
    std::vector<std::string> circuit_names = {
        // "ispd18_sample3",
        // "ispd19_sample4",
        // "ispd19_test4",
        // "ispd19_test5",


        "ispd19_test1",
        // "ispd18_test8",
        // "ispd18_test10",
        // "ispd19_test7",
        // "ispd19_test8",
        // "ispd19_test9"
    };

    //std::string benchmarks_path = "./input_files/ispd19";
    std::string benchmarks_path = "./input_files/circuits";

    for (auto circuit_name : circuit_names) {
        std::cout << "running circuit " << circuit_name << std::endl;

         std::string def_file =   benchmarks_path + "/" + circuit_name + "/" + circuit_name + ".input.def";
         std::string lef_file =   benchmarks_path + "/" + circuit_name + "/" + circuit_name + ".input.lef";
        //std::string def_file =   benchmarks_path + "/" + circuit_name + ".input.def";
        //std::string lef_file =   benchmarks_path + "/" + circuit_name + ".input.lef";
         std::string guide_file = benchmarks_path + "/cu_gr_solution/" + circuit_name + ".solution_cugr.guide";
        // std::string guide_file = "./" + circuit_name + "_astar.guide";
        // std::string guide_file = "./" + circuit_name + ".solution_cugr.guide";



        ophidian::parser::Def def;
        ophidian::parser::Lef lef;
        ophidian::parser::Guide guide;
        def = ophidian::parser::Def{def_file};
        lef = ophidian::parser::Lef{lef_file};
        guide = ophidian::parser::Guide{guide_file};

        auto design = ophidian::design::Design();
        ophidian::design::factory::make_design(design, def, lef, guide);

        std::vector<ophidian::circuit::Net> nets(design.netlist().begin_net(), design.netlist().end_net());
        /*std::vector<ophidian::circuit::Net> nets;
        auto net_to_debug = design.netlist().find_net("net2958");
        nets.push_back(net_to_debug);*/
        
        log() << "Initial wirelength = " << design.global_routing().wirelength(nets) << std::endl;

        log() << "Total number of vias = "<< design.global_routing().number_of_vias(nets) << std::endl;

        check_connectivity(design, nets);
        /*for(auto net : nets){
            auto connected = design.global_routing().is_connected(net);
            if(connected == false)
            {
                auto net_name = design.netlist().name(net);
                log() << net_name << " is not connected" << std::endl;
                draw_gcell_svg_2(design, net_name);
            }
        }*/

        std::cout << "Memory usage in peak= " << ophidian::util::mem_use::get_peak() << " MB" << std::endl;    
    }
}

