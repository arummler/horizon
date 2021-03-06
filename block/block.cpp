#include "block.hpp"
#include "json.hpp"
#include <set>

namespace horizon {

	Block::Block(const UUID &uu, const json &j, Pool &pool, Constraints *constr):
			uuid(uu),
			name(j.at("name").get<std::string>()),
			constraints(constr)
		{
			{
				const json &o = j["nets"];
				for (auto it = o.cbegin(); it != o.cend(); ++it) {
					auto u = UUID(it.key());
					nets.emplace(std::make_pair(u, Net(u, it.value(), *constraints)));
				}
			}
			{
				const json &o = j["buses"];
				for (auto it = o.cbegin(); it != o.cend(); ++it) {
					auto u = UUID(it.key());
					buses.emplace(std::make_pair(u, Bus(u, it.value(), *this)));
				}
			}
			{
				const json &o = j["components"];
				for (auto it = o.cbegin(); it != o.cend(); ++it) {
					auto u = UUID(it.key());
					components.emplace(std::make_pair(u, Component(u, it.value(), pool, *this)));
				}
			}
		}

	Block::Block(const UUID &uu): uuid(uu) {}

	Block Block::new_from_file(const std::string &filename, Pool &obj, Constraints *constr) {
		json j;
		std::ifstream ifs(filename);
		if(!ifs.is_open()) {
			throw std::runtime_error("file "  +filename+ " not opened");
		}
		ifs>>j;
		return Block(UUID(j["uuid"].get<std::string>()), j, obj, constr);
	}

	Net *Block::get_net(const UUID &uu) {
		return &nets.at(uu);
	}

	Net *Block::insert_net() {
		auto uu = UUID::random();
		auto n = &nets.emplace(uu, uu).first->second;
		n->net_class = constraints->default_net_class;
		return n;
	}

	void Block::merge_nets(Net *net, Net *into) {
		assert(net->uuid==nets.at(net->uuid).uuid);
		assert(into->uuid==nets.at(into->uuid).uuid);
		for(auto &it_comp: components) {
			for(auto &it_conn: it_comp.second.connections) {
				if(it_conn.second.net == net) {
					it_conn.second.net = into;
				}
			}
		}
		nets.erase(net->uuid);
	}

	void Block::update_refs() {
		for(auto &it_comp: components) {
			for(auto &it_conn: it_comp.second.connections) {
				it_conn.second.net.update(nets);
			}
		}
		for(auto &it_bus: buses) {
			it_bus.second.update_refs(*this);
		}
	}

	void Block::vacuum_nets() {
		std::set<UUID> nets_erase;
		for(const auto &it: nets) {
			if(!it.second.is_power) { //don't vacuum power nets
				nets_erase.emplace(it.first);
			}
		}
		for(const auto &it: buses) {
			for(const auto &it_mem : it.second.members) {
				nets_erase.erase(it_mem.second.net->uuid);
			}
		}
		for(const auto &it_comp: components) {
			for(const auto &it_conn: it_comp.second.connections) {
				nets_erase.erase(it_conn.second.net.uuid);
			}
		}
		for(const auto &uu: nets_erase) {
			nets.erase(uu);
		}
	}

	void Block::update_connection_count() {
		for(auto &it: nets) {
			it.second.n_pins_connected = 0;
			it.second.has_bus_rippers = false;
		}
		for(const auto &it_comp: components) {
			for(const auto &it_conn: it_comp.second.connections) {
				if(it_conn.second.net)
					it_conn.second.net->n_pins_connected++;
			}
		}

	}

	Block::Block(const Block &block):
		uuid(block.uuid),
		name(block.name),
		nets(block.nets),
		buses(block.buses),
		components(block.components),
		constraints(block.constraints)
	{
		update_refs();
	}

	void Block::operator=(const Block &block) {
		uuid = block.uuid;
		name = block.name;
		nets = block.nets;
		buses = block.buses;
		components = block.components;
		constraints = block.constraints;
		update_refs();
	}

	json Block::serialize() {
		json j;
		j["name"] = name;
		j["uuid"] = (std::string)uuid;
		j["nets"] = json::object();
		for(const auto &it : nets) {
			j["nets"][(std::string)it.first] = it.second.serialize();
		}
		j["components"] = json::object();
		for(const auto &it : components) {
			j["components"][(std::string)it.first] = it.second.serialize();
		}
		j["buses"] = json::object();
		for(const auto &it : buses) {
			j["buses"][(std::string)it.first] = it.second.serialize();
		}

		return j;
	}

	Net *Block::extract_pins(const std::set<UUIDPath<3>> &pins, Net *net) {
		if(pins.size()==0) {
			return nullptr;
		}
		if(net == nullptr) {
			net = insert_net();
		}
		for(const auto &it: pins) {
			Component &comp = components.at(it.at(0));
			UUIDPath<2> conn_path(it.at(1), it.at(2));
			if(comp.connections.count(conn_path)) {
				//the connection may have been deleted
				comp.connections.at(conn_path).net = net;
			}
		}

		return net;
	}
}
