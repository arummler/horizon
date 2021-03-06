#pragma once
#include "canvas/canvas.hpp"

namespace horizon {
	class CanvasGerber: public Canvas {
		public :
		CanvasGerber(class GerberExporter *exp);
		void push() override {}
		void request_push() override;
		private :

			void img_net(const Net *net) override;
			void img_polygon(const Polygon &poly) override;
			void img_line(const Coordi &p0, const Coordi &p1, const uint64_t width, int layer) override;
			void img_padstack(const Padstack &ps) override;
			void img_hole(const Hole &hole) override;

			GerberExporter *exporter;
	};
}
