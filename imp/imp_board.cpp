#include "imp_board.hpp"
#include "json.hpp"
#include "part.hpp"

namespace horizon {
	ImpBoard::ImpBoard(const std::string &board_filename, const std::string &block_filename, const std::string &constraints_filename, const std::string &via_dir, const std::string &pool_path):
			ImpLayer(pool_path),
			core_board(board_filename, block_filename, constraints_filename, via_dir, pool) {
		core = &core_board;
		core_board.signal_tool_changed().connect(sigc::mem_fun(this, &ImpBase::handle_tool_change));


		key_seq_append_default(key_seq);
		key_seq.append_sequence({
				{{GDK_KEY_p, GDK_KEY_j}, 	ToolID::PLACE_JUNCTION},
				{{GDK_KEY_j},				ToolID::PLACE_JUNCTION},
				{{GDK_KEY_d, GDK_KEY_l}, 	ToolID::DRAW_LINE},
				{{GDK_KEY_l},				ToolID::DRAW_LINE},
				{{GDK_KEY_d, GDK_KEY_a}, 	ToolID::DRAW_ARC},
				{{GDK_KEY_a},				ToolID::DRAW_ARC},
				{{GDK_KEY_d, GDK_KEY_y}, 	ToolID::DRAW_POLYGON},
				{{GDK_KEY_y}, 				ToolID::DRAW_POLYGON},
				{{GDK_KEY_p, GDK_KEY_t},	ToolID::PLACE_TEXT},
				{{GDK_KEY_t},				ToolID::PLACE_TEXT},
				{{GDK_KEY_p, GDK_KEY_p},	ToolID::MAP_PACKAGE},
				{{GDK_KEY_P},				ToolID::MAP_PACKAGE},
				{{GDK_KEY_d, GDK_KEY_t},	ToolID::DRAW_TRACK},
				{{GDK_KEY_p, GDK_KEY_v},	ToolID::PLACE_VIA},
				{{GDK_KEY_v},				ToolID::PLACE_VIA},
				{{GDK_KEY_x},				ToolID::ROUTE_TRACK},
				{{GDK_KEY_g},				ToolID::DRAG_KEEP_SLOPE},
		});
		key_seq.signal_update_hint().connect([this] (const std::string &s) {main_window->tool_hint_label->set_text(s);});

	}

	void ImpBoard::canvas_update() {
		canvas->update(*core_board.get_canvas_data());
		warnings_box->update(core_board.get_board()->warnings);
	}



	void ImpBoard::construct() {
		canvas->set_core(core.r);
		ImpLayer::construct();


		auto cam_job_button = Gtk::manage(new Gtk::Button("CAM Job"));
		main_window->top_panel->pack_start(*cam_job_button, false, false, 0);
		cam_job_button->show();
		cam_job_button->signal_clicked().connect([this]{cam_job_window->show_all();});
		core.r->signal_tool_changed().connect([cam_job_button](ToolID t){cam_job_button->set_sensitive(t==ToolID::NONE);});

		auto reload_netlist_button = Gtk::manage(new Gtk::Button("Reload netlist"));
		main_window->top_panel->pack_start(*reload_netlist_button, false, false, 0);
		reload_netlist_button->show();
		reload_netlist_button->signal_clicked().connect([this]{core_board.reload_netlist();canvas_update();});
		core.r->signal_tool_changed().connect([reload_netlist_button](ToolID t){reload_netlist_button->set_sensitive(t==ToolID::NONE);});


		cam_job_window = CAMJobWindow::create(main_window, core.b);
	}

	ToolID ImpBoard::handle_key(guint k) {
		return key_seq.handle_key(k);
	}
}
