#pragma once
#include "uuid.hpp"
#include "json_fwd.hpp"
#include "object.hpp"
#include "unit.hpp"
#include "block.hpp"
#include "schematic_symbol.hpp"
#include "line_net.hpp"
#include "text.hpp"
#include "net_label.hpp"
#include "bus_label.hpp"
#include "bus_ripper.hpp"
#include "power_symbol.hpp"
#include "frame.hpp"
#include "warning.hpp"
#include <vector>
#include <map>
#include <fstream>

namespace horizon {
	using json = nlohmann::json;

	class NetSegmentInfo {
		public:
		NetSegmentInfo(LineNet *li);
		NetSegmentInfo(Junction *ju);
		bool has_label = false;
		bool has_power_sym = false;
		Coordi position;
		Net *net = nullptr;
		Bus *bus= nullptr;
		bool is_bus() const;
	};

	class Sheet {
		public :
			Sheet(const UUID &uu, const json &, Block &Block, Object &pool);
			Sheet(const UUID &uu);
			UUID uuid;
			std::string name;
			unsigned int index;

			std::map<const UUID, Junction> junctions;
			std::map<const UUID, SchematicSymbol> symbols;
			//std::map<const UUID, class JunctionPin> junction_pins;
			std::map<const UUID, class LineNet> net_lines;
			std::map<const UUID, class Text> texts;
			std::map<const UUID, NetLabel> net_labels;
			std::map<const UUID, PowerSymbol> power_symbols;
			std::map<const UUID, BusLabel> bus_labels;
			std::map<const UUID, BusRipper> bus_rippers;
			std::vector<Warning> warnings;

			LineNet *split_line_net(LineNet *it, Junction *ju);
			void merge_net_lines(LineNet *a, LineNet *b, Junction *ju);
			void expand_symbols();
			void simplify_net_lines();
			void delete_dependants();
			void propagate_net_segments();
			std::map<const UUID, NetSegmentInfo> analyze_net_segments(bool place_warnings=false);
			std::set<UUIDPath<3>> get_pins_connected_to_net_segment(const UUID &uu_segment);

			void replace_junction(Junction *j, SchematicSymbol *sym, SymbolPin *pin);
			Junction *replace_bus_ripper(BusRipper *rip);
			//void replace_junction(Junction *j, PowerSymbol *sym);
			//void replace_power_symbol(PowerSymbol *sym, Junction *j);
			//void connect(SchematicSymbol *sym, SymbolPin *pin, PowerSymbol *power_sym);

			Frame frame;

			json serialize() const;
	};

}
