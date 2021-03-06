#pragma once
#include "package.hpp"
#include "pool.hpp"
#include "core.hpp"
#include "layer.hpp"
#include <memory>
#include <iostream>
#include <deque>

namespace horizon {
	class CorePackage: public Core {
		public:
			CorePackage(const std::string &filename, Pool &pool);
			bool has_object_type(ObjectType ty) override;

			Package *get_package(bool work=true);

			/*Polygon *insert_polygon(const UUID &uu, bool work = true);
			Polygon *get_polygon(const UUID &uu, bool work=true);
			void delete_polygon(const UUID &uu, bool work = true);
			Hole *insert_hole(const UUID &uu, bool work = true);
			Hole *get_hole(const UUID &uu, bool work=true);
			void delete_hole(const UUID &uu, bool work = true);*/

			const std::map<int, Layer> &get_layers();

			virtual std::string get_property_string(const UUID &uu, ObjectType type, ObjectProperty::ID property, bool *handled=nullptr);
			virtual void set_property_string(const UUID &uu, ObjectType type, ObjectProperty::ID property, const std::string &value, bool *handled=nullptr);

			void rebuild(bool from_undo=false);
			void commit();
			void revert();
			void save();


			const Package *get_canvas_data();
			std::pair<Coordi,Coordi> get_bbox() override;
			json get_meta() override;

		private:
			std::map<const UUID, Junction> *get_junction_map(bool work=true) override;
			std::map<const UUID, Line> *get_line_map(bool work=true) override;
			std::map<const UUID, Arc> *get_arc_map(bool work=true) override;
			std::map<const UUID, Text> *get_text_map(bool work=true) override;
			std::map<const UUID, Polygon> *get_polygon_map(bool work=true) override;
			std::map<const UUID, Hole> *get_hole_map(bool work=true) override;



			Package package;
			Package package_work;
			std::string m_filename;

			class HistoryItem: public Core::HistoryItem {
				public:
				HistoryItem(const Package &k):package(k) {}
				Package package;
			};
			void history_push();
			void history_load(unsigned int i);
	};
}
