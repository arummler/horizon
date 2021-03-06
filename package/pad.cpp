#include "pad.hpp"
#include "json.hpp"

namespace horizon {

	Pad::Pad(const UUID &uu, const json &j, Pool &pool):
			uuid(uu),
			pool_padstack(pool.get_padstack(j.at("padstack").get<std::string>())),
			padstack(*pool_padstack),
			placement(j.at("placement")),
			name(j.at("name").get<std::string>())
		{
		}
	Pad::Pad(const UUID &uu, Padstack *ps): uuid(uu), pool_padstack(ps), padstack(*ps) {}

	json Pad::serialize() const {
			json j;
			j["padstack"] = (std::string)pool_padstack->uuid;
			j["placement"] = placement.serialize();
			j["name"] = name;


			return j;
		}

	UUID Pad::get_uuid() const {
		return uuid;
	}
}
