#pragma once
#include "core.hpp"
#include "tool_place_junction.hpp"
#include "power_symbol.hpp"
#include <forward_list>

namespace horizon {

	class ToolPlacePowerSymbol : public ToolPlaceJunction {
		public :
			ToolPlacePowerSymbol(Core *c, ToolID tid);
			bool can_begin() override;

		protected:
			void create_attached();
			void delete_attached();
			bool begin_attached();
			bool update_attached(const ToolArgs &args);
			bool check_line(LineNet *li);
			PowerSymbol *sym = nullptr;
			std::forward_list<PowerSymbol*> symbols_placed;
			Net *net = nullptr;

		private:
			 bool do_merge(Net *other);

	};
}
