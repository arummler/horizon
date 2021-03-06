#pragma once
#include "imp_layer.hpp"
#include "core/core_board.hpp"
#include "cam_job_dialog.hpp"

namespace horizon {
	class ImpBoard : public ImpLayer {
		public :
			ImpBoard(const std::string &board_filename, const std::string &block_filename, const std::string &constraints_filename, const std::string &via_dir, const std::string &pool_path);

			const std::map<int, Layer> &get_layers();


		protected:
			void construct() override;
			ToolID handle_key(guint k) override;
		private:
			void canvas_update();

			CoreBoard core_board;

			CAMJobWindow *cam_job_window;

	};
}
