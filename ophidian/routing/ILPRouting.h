#ifndef OPHIDIAN_ILP_ROUTING
#define OPHIDIAN_ILP_ROUTING

#include <unordered_set>

#include <boost/geometry/index/rtree.hpp>

#include <ophidian/geometry/Models.h>

#include <ophidian/util/log.h>

#include <ophidian/design/Design.h>
#include <ophidian/routing/GCellGraph.h>
#include <ophidian/interconnection/Flute.h>
#include <ophidian/interconnection/SteinerTree.h>
#include <ophidian/routing/Library.h>

// Magic tricks to have CPLEX behave well:
// source https://github.com/alberto-santini/cplex-example
#ifndef IL_STD
#define IL_STD
#endif
#include <cstring>
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
// End magic tricks


namespace ophidian::routing {
    class RouteCandidate : public entity_system::EntityBase
    {
    public:
        using entity_system::EntityBase::EntityBase;
    };

    /*class NetSegment : public entity_system::EntityBase
    {
    public:
        using entity_system::EntityBase::EntityBase;
    };*/

    class NetWire : public entity_system::EntityBase
    {
    public:
        using entity_system::EntityBase::EntityBase;
    };

    class PositionCandidate : public entity_system::EntityBase
    {   
    public:
        using entity_system::EntityBase::EntityBase;
    };    

    /*
        var_type should be:
            IloNumVar   : for LP
            IloBoolVar  : for ILP
    */
    template <class var_type>
    class ILPRouting {
        public:
            enum PositionCandidateOrigin{
                NA, INITIAL, TWO_PIN_NET, CENTER_OF_MASS, MEDIAN
            };

            struct Statistics{
                int64_t model_runtime_ms;
                double model_memory = 0;
            };
            
        public:
            using scalar_type               = int;
            using unit_type                 = util::database_unit_t;
            using design_type               = design::Design;
            using net_type                  = circuit::Net;
            using cell_type                 = circuit::CellInstance;
            using pin_type                  = circuit::PinInstance;
            using pin_container_type        = std::vector<pin_type>;
            using route_candidate_type      = RouteCandidate;
            using wire_type                 = NetWire;
            using wire_container_type       = std::vector<wire_type>;
            using gcell_type                = GCell;
            using gcell_container_type      = std::vector<GCell>;
            using position_candidate_type   = PositionCandidate;
            using candidate_origin_type     = PositionCandidateOrigin;

            using point_type                = util::LocationDbu;
            using point_container_type      = std::vector<point_type>;
            using box_type                  = geometry::Box<unit_type>;

            using solver_type               = IloCplex;
            using model_type                = IloModel;
            using environment_type          = IloEnv;
            using var_container_type        = std::vector<var_type>;

            using unitless_point_type       = geometry::Point<double>;
            using unitless_box_type         = geometry::Box<double>;
            using unitless_segment_type     = geometry::Segment<unitless_point_type>;

            using layer_type                = Library::layer_type;
            using layer_direction_type      = Direction;
            
            struct segment{
                point_type start;
                point_type end;
                pin_container_type start_pins;
                pin_container_type end_pins;
            };
            using segment_type              = segment;

            ILPRouting(design_type & design, std::string circuit_name);

            std::pair<bool, Statistics> route_nets(const std::vector<net_type> & nets, const std::vector<cell_type> & cells, box_type & area, const std::vector<net_type> & fixed_nets, std::vector<net_type> & routed_nets, std::vector<net_type> & unrouted_nets, std::vector<std::pair<cell_type, point_type>> & movements, bool initial_routing = true);

            void add_extra_demand();

        private:
            bool m_extra_demand_created;

            void update_gcell_capacities(const std::vector<net_type> & fixed_nets);

            void create_all_candidates(const std::vector<net_type> & nets, model_type & model, bool initial_routing = true);

            void create_net_candidates(const net_type & net, model_type & model, bool initial_routing = true);

            void create_all_candidates_with_movements(const std::vector<net_type> & nets, const std::vector<cell_type> & cells, box_type & area, model_type & model);

            void create_center_of_mass_candidate(const cell_type cell, model_type & model);

            void create_median_candidate(const cell_type cell, box_type & area, model_type & model);

            void create_2_pin_nets_candidates_with_movements(const net_type net, model_type & model);

            void create_movement_candidate(const cell_type & cell, const candidate_origin_type type, const point_type& new_position, const std::vector<net_type>& nets, std::string variable_name, model_type & model );

            void generate_routes_of_net(const net_type & net, const position_candidate_type pos_candidate, model_type & model, std::pair<layer_type, layer_type> layer_range);
            
            void create_net_candidates_in_layers(const net_type & net, const std::vector<segment_type> & segments, const layer_type & horizontal_layer, const layer_type & vertical_layer, bool large_net, const std::set<std::pair<unit_type, unit_type>> & steiner_points, const position_candidate_type pos_candidate, model_type & model);

            void add_wires_of_splitted_segment(const segment_type & segment, const point_type & segment_start, const point_type & segment_end, const layer_type & horizontal_layer, const layer_type & vertical_layer, bool connect_on_y, unsigned branch_count, const std::vector<route_candidate_type> & candidates, bool large_net);

            void create_candidate(const net_type & net, const wire_container_type & wires, std::string variable_name, model_type & model);

            void add_wires_to_candidate(const route_candidate_type & candidate, const wire_container_type & wires);

            void split_segment(const point_type & segment_start, const point_type & segment_end, const layer_type & wire1_layer, const layer_type & wire2_layer, bool connect_on_y, wire_container_type & wires);

            wire_type create_wire(const point_type & wire_start, const point_type & wire_end, const layer_type & start_layer, const layer_type & end_layer);

            void add_objective_function(model_type & model);

            void add_candidate_constraints(const std::vector<net_type> & nets, model_type & model);

            void add_capacity_constraints(const std::vector<net_type> & nets, model_type & model);

            void add_movements_constraints(model_type & model);

	        void write_gcell_capacities();

	        void write_segments(const std::vector<net_type> & nets, const solver_type& solver, std::vector<net_type> & routed_nets, std::vector<net_type> & unrouted_nets);

            void save_movements(const solver_type& solver, std::vector<std::pair<cell_type, point_type>> & movements);

    	    void save_result(const solver_type& solver);

            void create_all_cell_initial_candidates(model_type & model);

            void convert_to_flute(point_container_type & converted, const point_container_type & points) const;

            void find_gcells_intersecting_with_candidate(route_candidate_type & candidate, gcell_container_type & intersecting_gcells);

            bool cell_is_only_connected_to_local_nets(cell_type & cell, const std::vector<net_type> & nets);

            point_type convert_to_design(point_type & point) const;

            design_type&    m_design;
            std::string     m_circuit_name;
            environment_type m_env;

    //SEGMENTS
            /*entity_system::EntitySystem<segment_type>                       m_segments;
            entity_system::Property<segment_type, point_type>               m_segment_starts{m_segments};
            entity_system::Property<segment_type, point_type>               m_segment_ends{m_segments};
            entity_system::Property<segment_type, pin_container_type>       m_segment_start_pin{m_segments};
            entity_system::Property<segment_type, pin_container_type>       m_segment_end_pin{m_segments};

            entity_system::Aggregation<net_type, segment_type>              m_net_segments{m_design.netlist().make_aggregation_net<segment_type>(m_segments)};*/

    //ROUTE CANDIDATES
            entity_system::EntitySystem<route_candidate_type>                     m_route_candidate;
            entity_system::Property<route_candidate_type, std::string>            m_route_candidate_names{m_route_candidate};
            entity_system::Property<route_candidate_type, var_type>               m_route_candidate_variables{m_route_candidate};
            entity_system::Property<route_candidate_type, unit_type>              m_route_candidate_wirelengths{m_route_candidate};
            entity_system::Property<route_candidate_type, net_type>               m_route_candidate_nets{m_route_candidate};
    	    //entity_system::Property<route_candidate_type, wire_container_type>    m_route_candidate_wires{m_route_candidate};

            std::unordered_map<std::string, route_candidate_type>                 m_name_to_route_candidate;
            entity_system::Aggregation<net_type, route_candidate_type>            m_net_candidates{m_design.netlist().make_aggregation_net<route_candidate_type>(m_route_candidate)};

    //WIRES
            entity_system::EntitySystem<wire_type>          m_wires;
            entity_system::Property<wire_type, point_type>  m_wire_starts{m_wires};
            entity_system::Property<wire_type, point_type>  m_wire_ends{m_wires};
            entity_system::Property<wire_type, int>  m_wire_start_layers{m_wires};
            entity_system::Property<wire_type, int>  m_wire_end_layers{m_wires};
            
            entity_system::Aggregation<route_candidate_type, wire_type>            m_route_candidate_wires{m_route_candidate, m_wires};


            entity_system::Property<gcell_type, scalar_type>    m_gcells_demand{m_design.global_routing().gcell_graph()->make_property_gcells<scalar_type>()};
            entity_system::Property<gcell_type, scalar_type>    m_gcells_extra_demand{m_design.global_routing().gcell_graph()->make_property_gcells<scalar_type>()};

    // POSITION CANDIDATES
            entity_system::EntitySystem<position_candidate_type>            m_position_candidates;
            entity_system::Property<position_candidate_type, std::string>   m_position_candidate_names{m_position_candidates};
            entity_system::Property<position_candidate_type, cell_type>     m_position_candidate_cell{m_position_candidates};
            entity_system::Property<position_candidate_type, var_type>      m_position_candidate_variables{m_position_candidates};
            entity_system::Property<position_candidate_type, point_type>    m_position_candidate_position{m_position_candidates};
            entity_system::Property<position_candidate_type, candidate_origin_type>   m_position_candidate_origin{m_position_candidates, candidate_origin_type::NA};

            entity_system::Aggregation<position_candidate_type, route_candidate_type>  m_position_candidate_to_routes{m_position_candidates, m_route_candidate};

            std::unordered_map<std::string, position_candidate_type>        m_name_to_position_candidate;
            entity_system::Property<cell_type, position_candidate_type>     m_cell_initial_candidate{m_design.netlist().make_property_cell_instance<position_candidate_type>()};
            entity_system::Aggregation<cell_type, position_candidate_type>  m_cell_position_candidates{m_design.netlist().make_aggregation_cell<position_candidate_type>(m_position_candidates)};

    };
} // end namespace ophidian::routing

#include "ILPRouting.hpp"

#endif
