#include "imp_schematic.hpp"
#include "export_pdf.hpp"
#include "part.hpp"

namespace horizon {
	ImpSchematic::ImpSchematic(const std::string &schematic_filename, const std::string &block_filename, const std::string &constraints_filename, const std::string &pool_path) :ImpBase(pool_path),
			core_schematic(schematic_filename, block_filename, constraints_filename, pool)
	{
		core = &core_schematic;
		core_schematic.signal_tool_changed().connect(sigc::mem_fun(this, &ImpSchematic::handle_tool_change));
		core_schematic.signal_rebuilt().connect(sigc::mem_fun(this, &ImpSchematic::handle_core_rebuilt));

		key_seq_append_default(key_seq);
		key_seq.append_sequence({
				{{GDK_KEY_p, GDK_KEY_j}, 	ToolID::PLACE_JUNCTION},
				{{GDK_KEY_j},				ToolID::PLACE_JUNCTION},
				//{{GDK_KEY_d, GDK_KEY_l}, 	ToolID::DRAW_LINE},
				//{{GDK_KEY_l},				ToolID::DRAW_LINE},
				//{{GDK_KEY_d, GDK_KEY_a}, 	ToolID::DRAW_ARC},
				//{{GDK_KEY_a},				ToolID::DRAW_ARC},
				{{GDK_KEY_p, GDK_KEY_s},	ToolID::MAP_SYMBOL},
				{{GDK_KEY_s},				ToolID::MAP_SYMBOL},
				{{GDK_KEY_d, GDK_KEY_n}, 	ToolID::DRAW_NET},
				{{GDK_KEY_n},				ToolID::DRAW_NET},
				{{GDK_KEY_p, GDK_KEY_c},	ToolID::ADD_COMPONENT},
				{{GDK_KEY_c},				ToolID::ADD_COMPONENT},
				{{GDK_KEY_p, GDK_KEY_p},	ToolID::ADD_PART},
				{{GDK_KEY_P},				ToolID::ADD_PART},
				{{GDK_KEY_p, GDK_KEY_t},	ToolID::PLACE_TEXT},
				{{GDK_KEY_t},				ToolID::PLACE_TEXT},
				{{GDK_KEY_p, GDK_KEY_b},	ToolID::PLACE_NET_LABEL},
				{{GDK_KEY_b},				ToolID::PLACE_NET_LABEL},
				{{GDK_KEY_D},				ToolID::DISCONNECT},
				{{GDK_KEY_k},				ToolID::BEND_LINE_NET},
				{{GDK_KEY_g},				ToolID::SELECT_NET_SEGMENT},
				{{GDK_KEY_p, GDK_KEY_o},	ToolID::PLACE_POWER_SYMBOL},
				{{GDK_KEY_o},				ToolID::PLACE_POWER_SYMBOL},
				{{GDK_KEY_v},				ToolID::MOVE_NET_SEGMENT},
				{{GDK_KEY_V},				ToolID::MOVE_NET_SEGMENT_NEW},
				{{GDK_KEY_i},				ToolID::EDIT_COMPONENT_PIN_NAMES},
				{{GDK_KEY_p, GDK_KEY_u},	ToolID::PLACE_BUS_LABEL},
				{{GDK_KEY_u},				ToolID::PLACE_BUS_LABEL},
				{{GDK_KEY_p, GDK_KEY_r},	ToolID::PLACE_BUS_RIPPER},
				{{GDK_KEY_slash},			ToolID::PLACE_BUS_RIPPER},
				{{GDK_KEY_B},				ToolID::MANAGE_BUSES},
				{{GDK_KEY_h},				ToolID::SMASH},
				{{GDK_KEY_H},				ToolID::UNSMASH},
		});
		key_seq.signal_update_hint().connect([this] (const std::string &s) {main_window->tool_hint_label->set_text(s);});
	}

	void ImpSchematic::canvas_update() {
		canvas->update(*core_schematic.get_canvas_data());
		warnings_box->update(core_schematic.get_sheet()->warnings);
	}

	void ImpSchematic::handle_select_sheet(Sheet *sh) {
		if(sh == core_schematic.get_sheet())
			return;

		auto v = canvas->get_scale_and_offset();
		sheet_views[core_schematic.get_sheet()->uuid] = v;
		core_schematic.set_sheet(sh->uuid);
		canvas_update();
		if(sheet_views.count(sh->uuid)) {
			auto v2 = sheet_views.at(sh->uuid);
			canvas->set_scale_and_offset(v2.first, v2.second);
		}
	}

	void ImpSchematic::handle_remove_sheet(Sheet *sh) {
		core_schematic.delete_sheet(sh->uuid);
		canvas_update();
	}

	void ImpSchematic::construct() {
		canvas->set_core(core.r);
		sheet_box = Gtk::manage(new SheetBox(&core_schematic));
		sheet_box->show_all();
		sheet_box->signal_add_sheet().connect([this]{core_schematic.add_sheet(); std::cout<<"add sheet"<<std::endl;});
		sheet_box->signal_remove_sheet().connect(sigc::mem_fun(this, &ImpSchematic::handle_remove_sheet));
		sheet_box->signal_select_sheet().connect(sigc::mem_fun(this, &ImpSchematic::handle_select_sheet));
		main_window->left_panel->pack_start(*sheet_box, false, false, 0);

		auto print_button = Gtk::manage(new Gtk::Button("Export PDF"));
		print_button->signal_clicked().connect(sigc::mem_fun(this, &ImpSchematic::handle_export_pdf));
		print_button->show();
		main_window->top_panel->pack_start(*print_button, false, false, 0);

		add_tool_button(ToolID::ANNOTATE, "Annotate");
		add_tool_button(ToolID::MANAGE_BUSES, "Buses...");
		add_tool_button(ToolID::ADD_PART, "Place part");

		core.r->signal_tool_changed().connect([print_button](ToolID t){print_button->set_sensitive(t==ToolID::NONE);});

		grid_spin_button->set_sensitive(false);
	}

	void ImpSchematic::handle_export_pdf() {
		Gtk::FileChooserDialog fc(*main_window, "Save PDF", Gtk::FILE_CHOOSER_ACTION_SAVE);
		fc.set_do_overwrite_confirmation(true);
		if(last_pdf_filename.size()) {
			fc.set_filename(last_pdf_filename);
		}
		else {
			fc.set_current_name("schematic.pdf");
		}
		fc.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
		fc.add_button("_Save", Gtk::RESPONSE_ACCEPT);
		if(fc.run()==Gtk::RESPONSE_ACCEPT) {
			std::string fn = fc.get_filename();
			last_pdf_filename = fn;
			export_pdf(fn, *core.c->get_schematic(), core.r);
		}
	}

	void ImpSchematic::handle_core_rebuilt() {
		sheet_box->update();
	}

	void ImpSchematic::handle_tool_change(ToolID id) {
		ImpBase::handle_tool_change(id);
		sheet_box->set_sensitive(id == ToolID::NONE);
	}

	ToolID ImpSchematic::handle_key(guint k) {
		return key_seq.handle_key(k);
	}
}
