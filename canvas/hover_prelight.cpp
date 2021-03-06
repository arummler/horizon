#include "canvas.hpp"
#include <iostream>
#include <algorithm>
#include <epoxy/gl.h>

namespace horizon {

	void CanvasGL::hover_prelight_update(GdkEventMotion *motion_event) {
		if(!selection_allowed)
			return;
		if(selection_mode != CanvasGL::SelectionMode::HOVER)
			return;
		gdouble x,y;
		gdk_event_get_coords((GdkEvent*)motion_event, &x, &y);
		auto c = screen2canvas({(float)x,(float)y});
		float area_min = 1e99;
		int area_min_i = -1;
		unsigned int i = 0;
		for(auto &it: selectables.items) {
			it.flags=0 ;
			if(it.inside(c, 10/scale) && selection_filter.can_select(selectables.items_ref[i])) {
				if(it.area()<area_min) {
					area_min = it.area();
					area_min_i = i;
				}
			}
			i++;
		}
		if(area_min_i != -1) {
			selectables.items[area_min_i].flags = 1;
		}
		request_push();
	}
}
