#pragma once
#include "uuid.hpp"
#include "json_fwd.hpp"
#include "common.hpp"
#include "object.hpp"
#include "position_provider.hpp"
#include "uuid_provider.hpp"
#include "block.hpp"
#include "uuid_ptr.hpp"
#include <vector>
#include <map>
#include <fstream>


namespace horizon {
	using json = nlohmann::json;


	class PowerSymbol: UUIDProvider{
		public :
			PowerSymbol(const UUID &uu, const json &j, class Sheet &sheet, class Block &block);
			PowerSymbol(const UUID &uu);

			const UUID uuid;
			uuid_ptr<Junction> junction;
			uuid_ptr<Net> net;
			bool mirror = false;


			virtual UUID get_uuid() const ;
			void update_refs(Sheet &sheet, Block &block);

			//not stored
			bool temp;

			json serialize() const;
	};
}
