#pragma once
#include "main_window.hpp"
#include "pool.hpp"
#include "core/core_symbol.hpp"
#include "core/core_schematic.hpp"
#include "core/core_padstack.hpp"
#include "core/core_package.hpp"
#include "property_panels/property_panels.hpp"
#include "widgets/warnings_box.hpp"
#include "widgets/sheet_box.hpp"
#include "widgets/layer_box.hpp"
#include "core/cores.hpp"
#include "core/clipboard.hpp"
#include "key_sequence.hpp"
#include "tool_popover.hpp"
#include "selection_filter_dialog.hpp"
#include "keyseq_dialog.hpp"
#include "widgets/spin_button_dim.hpp"

namespace horizon {
	class ImpBase {
		public :
			ImpBase(const std::string &pool_path);
			void run(int argc, char *argv[]);
			virtual void handle_tool_change(ToolID id);
			virtual void construct() = 0;
			void canvas_update_from_pp();

		protected :
			MainWindow *main_window;
			CanvasGL *canvas;
			PropertyPanels *panels;
			WarningsBox *warnings_box;
			ToolPopover *tool_popover;
			SpinButtonDim *grid_spin_button;
			std::unique_ptr<SelectionFilterDialog> selection_filter_dialog;

			Pool pool;
			Cores core;
			std::unique_ptr<ClipboardManager> clipboard=nullptr;
			std::unique_ptr<KeySequenceDialog> key_sequence_dialog=nullptr;
			Glib::RefPtr<Glib::Binding> grid_spacing_binding;
			KeySequence key_seq;
			
			virtual void canvas_update() = 0;
			virtual ToolID handle_key(guint k) = 0;
			void sc(void);
			bool handle_key_press(GdkEventKey *key_event);
			void handle_cursor_move(const Coordi &pos);
			bool handle_click(GdkEventButton *button_event);
			void tool_process(const ToolResponse &resp);
			void tool_begin(ToolID id);
			void add_tool_button(ToolID id, const std::string &label);
			void handle_warning_selected(const Coordi &pos);

			void key_seq_append_default(KeySequence &ks);
			
	};
}

