#include "footprint_generator_single.hpp"

namespace horizon {
	FootprintGeneratorSingle::FootprintGeneratorSingle(CorePackage *c): Glib::ObjectBase (typeid(FootprintGeneratorSingle)), FootprintGeneratorBase("/net/carrotIndustries/horizon/imp/footprint_generator/single.svg", c) {

			{
				auto tbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 4));
				auto la = Gtk::manage(new Gtk::Label("Count:"));
				tbox->pack_start(*la, false, false, 0);

				sp_count = Gtk::manage(new Gtk::SpinButton());
				sp_count->set_range(2, 512);
				sp_count->set_increments(2, 2);
				tbox->pack_start(*sp_count, false, false, 0);

				box_top->pack_start(*tbox, false, false, 0);
			}

			update_preview();

			sp_pitch = Gtk::manage(new SpinButtonDim());
			sp_pitch->set_range(0, 50_mm);
			sp_pitch->set_valign(Gtk::ALIGN_CENTER);
			sp_pitch->set_halign(Gtk::ALIGN_START);
			sp_pitch->set_value(1_mm);
			overlay->add_at_sub(*sp_pitch, "#pitch");
			sp_pitch->show();

			sp_count->signal_value_changed().connect([this]{pad_count = sp_count->get_value_as_int(); update_preview();});
			sp_count->set_value(4);
			sp_count->set_range(1, 512);
			sp_count->set_increments(1, 1);

		}

		bool FootprintGeneratorSingle::generate() {
			if(!property_can_generate())
				return false;
			auto pkg = core->get_package();
			int64_t pitch = sp_pitch->get_value_as_int();
			int64_t y0 = (pad_count-1)*(pitch/2);
			for(unsigned int i = 0; i<pad_count; i++) {
				auto uu = UUID::random();
				auto padstack = core->m_pool->get_padstack(padstack_button->property_selected_uuid());
				auto &pad = pkg->pads.emplace(uu, Pad(uu, padstack)).first->second;
				pad.placement.shift = {0, y0-pitch*i};
				pad.placement.set_angle_deg(270);
				pad.name = std::to_string(i+1);
			}

			core->commit();
			return true;
		}

		void FootprintGeneratorSingle::update_preview() {
			auto n = pad_count;
			if(n>=4) {
				overlay->sub_texts["#pad1"] = "1";
				overlay->sub_texts["#pad2"] = "2";
				overlay->sub_texts["#pad3"] = std::to_string(n-1);
				overlay->sub_texts["#pad4"] = std::to_string(n);
			}
			else if(n==3) {
				overlay->sub_texts["#pad1"] = "1";
				overlay->sub_texts["#pad2"] = "2";
				overlay->sub_texts["#pad3"] = "3";
				overlay->sub_texts["#pad4"] = "X";
			}
			else if(n==2) {
				overlay->sub_texts["#pad1"] = "1";
				overlay->sub_texts["#pad2"] = "2";
				overlay->sub_texts["#pad3"] = "X";
				overlay->sub_texts["#pad4"] = "X";
			}
			else if(n==1) {
				overlay->sub_texts["#pad1"] = "1";
				overlay->sub_texts["#pad2"] = "X";
				overlay->sub_texts["#pad3"] = "X";
				overlay->sub_texts["#pad4"] = "X";
			}
			overlay->queue_draw();
		}
}
