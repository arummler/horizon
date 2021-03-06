#include "tool_route_track.hpp"
#include <iostream>
#include "core_board.hpp"
#include "obstacle/canvas_obstacle.hpp"

namespace horizon {

	ToolRouteTrack::ToolRouteTrack(Core *c, ToolID tid):ToolBase(c, tid) {
		name = "Route Track";
	}

	bool ToolRouteTrack::can_begin() {
		return core.b;
	}

	ToolResponse ToolRouteTrack::begin(const ToolArgs &args) {
		std::cout << "tool route track\n";
		core.r->selection.clear();
		return ToolResponse();
	}

	void ToolRouteTrack::begin_track(const ToolArgs &args) {
		auto c = args.target.p;
		CanvasObstacle ca;
		ca.routing_layer = args.work_layer;
		routing_layer = args.work_layer;
		ca.routing_width = net->net_class->default_width;
		ca.routing_net = net;
		ca.set_core(core.r);
		ca.update(*core.b->get_board());
		obstacles.clear();
		ca.clipper.Execute(ClipperLib::ctUnion, obstacles, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
		core.b->get_board()->obstacles = obstacles;
		track_path.clear();
		track_path.emplace_back(c.x, c.y);
		track_path.emplace_back(c.x, c.y);
		track_path.emplace_back(c.x, c.y);
		track_path_known_good = track_path;
	}

	void ToolRouteTrack::update_track(const Coordi &c) {
		assert(track_path.size()>=3);
		auto l = track_path.end()-1;
		l->X = c.x;
		l->Y = c.y;

		auto k = l-1;
		auto b = l-2;
		auto dx = std::abs(l->X-b->X);
		auto dy = std::abs(l->Y-b->Y);
		if(dy > dx) {
			auto si = (l->Y<b->Y)?1:-1;
			if(bend_mode) {
				k->X = b->X;
				k->Y = l->Y + si*dx;
			}
			else {
				k->X = l->X;
				k->Y = b->Y - si*dx;
			}
		}
		else {
			auto si = (l->X<b->X)?1:-1;
			if(bend_mode) {
				k->Y = b->Y;
				k->X = l->X + si*dy;
			}
			else {
				k->Y = l->Y;
				k->X = b->X - si*dy;
			}
		}

		core.b->get_board()->track_path = track_path;
	}

	bool ToolRouteTrack::check_track_path(const ClipperLib::Path &p) {
		ClipperLib::Clipper clipper;
		clipper.AddPaths(obstacles, ClipperLib::ptClip, true);
		clipper.AddPath(p, ClipperLib::ptSubject, false);
		ClipperLib::PolyTree solution;
		clipper.Execute(ClipperLib::ctIntersection, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
		return solution.ChildCount() ==0;
	}

	static Coordi coordi_fron_intpt(const ClipperLib::IntPoint &p) {
		Coordi r(p.X, p.Y);
		return r;
	}

	void ToolRouteTrack::update_temp_track() {
		auto brd = core.b->get_board();
		for(auto &it: temp_tracks) {
			brd->tracks.erase(it->uuid);
		}
		temp_tracks.clear();
		for(auto &it: temp_junctions) {
			brd->junctions.erase(it->uuid);
		}
		temp_junctions.clear();
		if(track_path.size()>=3) {

			ClipperLib::Path path_simp;
			path_simp.push_back(track_path.at(0));
			path_simp.push_back(track_path.at(1));
			path_simp.push_back(track_path.at(2));

			for(auto it = track_path.cbegin()+3; it < track_path.cend(); it++) {
				if(path_simp.back() == *it)
					continue;
				path_simp.push_back(*it);
				auto p0 = coordi_fron_intpt(*(path_simp.end()-1));
				auto p1 = coordi_fron_intpt(*(path_simp.end()-2));
				auto p2 = coordi_fron_intpt(*(path_simp.end()-3));

				auto va = p2-p1;
				auto vb = p1-p0;

				if((va.dot(vb))*(va.dot(vb)) == va.mag_sq()*vb.mag_sq()) {
					path_simp.erase(path_simp.end()-2);
				}

			}



			/*auto tj = core.r->insert_junction(UUID::random());
			tj->temp = true;
			tj->net = net;
			tj->net_segment = net_segment;
			tj->position = coordi_fron_intpt(path_simp.at(1));
			temp_junctions.push_back(tj);

			auto uu = UUID::random();
			auto tt = &brd->tracks.emplace(uu, uu).first->second;
			tt->from = conn_start;
			tt->to.connect(tj);
			tt->net = net;
			tt->net_segment = net_segment;
			tt->layer = routing_layer;
			temp_tracks.push_back(tt);*/


			Junction *tj = nullptr;
			for(auto it = path_simp.cbegin()+1; it < path_simp.cend(); it++) {
				if(*it != *(it-1)) {
					auto tuu = UUID::random();
					auto tr = &brd->tracks.emplace(tuu, tuu).first->second;
					tr->net = net;
					tr->net_segment = net_segment;
					tr->layer = routing_layer;
					temp_tracks.push_back(tr);

					auto ju = core.r->insert_junction(UUID::random());
					ju->temp = true;
					ju->position = coordi_fron_intpt(*it);
					ju->net = net;
					ju->net_segment = net_segment;
					temp_junctions.push_back(ju);

					if(tj == nullptr) {
						tr->from = conn_start;
					}
					else {
						tr->from.connect(tj);
					}

					tr->to.connect(ju);
					tj = ju;
				}
			}


		}
		if(via) {
			via->junction = temp_junctions.back();
		}

		core.b->get_board()->update_airwires();
	}

	bool ToolRouteTrack::try_move_track(const ToolArgs &args) {
		bool drc_okay = false;
		update_track(args.coords);
		if(!check_track_path(track_path)) { //drc error
			std::cout << "drc error" << std::endl;
			bend_mode ^= true;
			update_track(args.coords); //flip
			if(!check_track_path(track_path)) { //still drc errror
				std::cout << "still drc error" << std::endl;
				bend_mode ^= true;
				track_path = track_path_known_good;
				assert(check_track_path(track_path));
				core.b->get_board()->track_path = track_path;

				//create new segment
				//track_path.emplace_back(args.coords.x, args.coords.y);
				//track_path.emplace_back(args.coords.x, args.coords.y);
			}
			else {
				std::cout << "no drc error after flip" << std::endl;
				track_path_known_good = track_path;
				drc_okay = true;
			}
		}
		else {
			std::cout << "no drc error" << std::endl;
			track_path_known_good = track_path;
			drc_okay = true;
		}
		update_temp_track();
		return drc_okay;
	}

	ToolResponse ToolRouteTrack::update(const ToolArgs &args) {
		if(args.type == ToolEventType::KEY) {
			if(args.key == GDK_KEY_Escape) {
				core.b->revert();
				core.b->get_board()->obstacles.clear();
				core.b->get_board()->track_path.clear();
				return ToolResponse::end();
			}
		}
		if(net == nullptr) { //begin route
			if(args.type == ToolEventType::CLICK) {
				if(args.target.type == ObjectType::PAD) {
					auto pkg = &core.b->get_board()->packages.at(args.target.path.at(0));
					auto pad = &pkg->package.pads.at(args.target.path.at(1));
					net = pad->net;
					net_segment = pad->net_segment;
					if(net) {
						ToolArgs a(args);
						if(!core.b->get_layers().at(a.work_layer).copper) {
							a.work_layer = 0;
						}
						if((pad->padstack.type == Padstack::Type::TOP) ^ pkg->flip) {
							a.work_layer = 0; //top
						}
						else if((pad->padstack.type == Padstack::Type::BOTTOM) ^ pkg->flip) {
							a.work_layer = -100;
						}
						else if(pad->padstack.type == Padstack::Type::THROUGH) {
							//it's okay
						}


						begin_track(a);
						conn_start.connect(pkg, pad);
						std::cout << "begin net" << std::endl;

						return ToolResponse::change_layer(a.work_layer);
					}
				}
				else if(args.target.type == ObjectType::JUNCTION) {
					auto junc = core.r->get_junction(args.target.path.at(0));
					net = junc->net;
					net_segment = junc->net_segment;
					if(net) {
						ToolArgs a(args);
						if(!core.b->get_layers().at(a.work_layer).copper) {
							a.work_layer = 0;
						}
						if(!junc->has_via) {
							if(junc->layer<10000) {
								a.work_layer = junc->layer;
							}
						}
						begin_track(a);
						conn_start.connect(junc);
						std::cout << "begin net" << std::endl;
						return ToolResponse::change_layer(a.work_layer);
					}
				}
			}
		}
		else {
			if(args.type == ToolEventType::MOVE) {
				try_move_track(args);
				std::cout << "temp track" << std::endl;
				for(const auto &it: track_path) {
					std::cout << it.X << " " << it.Y << std::endl;
				}
				std::cout << std::endl << std::endl;
			}
			else if(args.type == ToolEventType::CLICK) {
				if(args.button == 1) {
					if(!args.target.is_valid()) {
						track_path.emplace_back(args.coords.x, args.coords.y);
						track_path.emplace_back(args.coords.x, args.coords.y);
						bend_mode ^= true;
						try_move_track(args);
					}
					else {
						if(!try_move_track(args)) //drc not okay
							return ToolResponse();
						if(args.target.type == ObjectType::PAD) {
							auto pkg = &core.b->get_board()->packages.at(args.target.path.at(0));
							auto pad = &pkg->package.pads.at(args.target.path.at(1));
							if(pad->net != net)
								return ToolResponse();
							temp_tracks.back()->to.connect(pkg, pad);
						}
						else if(args.target.type == ObjectType::JUNCTION) {
							std::cout << "temp track ju" << std::endl;
							for(const auto &it: track_path) {
								std::cout << it.X << " " << it.Y << std::endl;
							}

							auto junc = core.r->get_junction(args.target.path.at(0));
							if(junc->net && (junc->net != net))
								return ToolResponse();
							temp_tracks.back()->to.connect(junc);
						}
						else {
							return ToolResponse();
						}

						core.r->delete_junction(temp_junctions.back()->uuid);

						core.b->get_board()->track_path.clear();
						core.b->get_board()->obstacles.clear();
						core.b->commit();
						return ToolResponse::end();

					}
				}
				else if(args.button == 3) {
					auto brd = core.b->get_board();

					if(track_path.size()>=2 && via == nullptr) {
						track_path.pop_back();
						track_path.pop_back();
						update_temp_track();
					}

					core.b->get_board()->track_path.clear();
					core.b->get_board()->obstacles.clear();
					core.b->commit();
					return ToolResponse::end();
				}
			}
			else if(args.type == ToolEventType::KEY) {
				if(args.key == GDK_KEY_slash) {
					bend_mode ^= true;
					auto &b = track_path.back();
					update_track({b.X, b.Y});
					if(!check_track_path(track_path)) { //drc error
						bend_mode ^= true;
						update_track({b.X, b.Y});
					}
					update_temp_track();
				}
				else if(args.key == GDK_KEY_v) {
					if(via == nullptr) {
						UUID padstack_uuid;
						bool r;
						std::tie(r, padstack_uuid) = core.r->dialogs.select_via_padstack(core.b->get_via_padstack_provider());
						if(r) {
							auto uu = UUID::random();
							via = &core.b->get_board()->vias.emplace(uu, Via(uu, core.b->get_via_padstack_provider()->get_padstack(padstack_uuid))).first->second;
							update_temp_track();
						}
					}
					else {
						core.b->get_board()->vias.erase(via->uuid);
						via = nullptr;
					}
				}
				else if(args.key == GDK_KEY_BackSpace) {
					if(track_path.size()>3) {
						track_path.pop_back();
						track_path.pop_back();
						update_track(args.coords);
						if(!check_track_path(track_path)) { //drc error
							std::cout << "drc error" << std::endl;
							bend_mode ^= true;
							update_track(args.coords); //flip
							if(!check_track_path(track_path)) { //still drc errror
								std::cout << "still drc error" << std::endl;
								bend_mode ^= true;
								track_path = track_path_known_good;
								assert(check_track_path(track_path));
								core.b->get_board()->track_path = track_path;
							}
							else {
								std::cout << "no drc error after flip" << std::endl;
								track_path_known_good = track_path;
							}
						}
						else {
							std::cout << "no drc error" << std::endl;
							track_path_known_good = track_path;
						}
						update_temp_track();
					}
				}
			}
		}
		return ToolResponse();
	}

}
