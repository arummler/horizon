#include "selection_filter.hpp"
#include "canvas.hpp"

namespace horizon {
	bool SelectionFilter::can_select(const SelectableRef &sel) {
		if(object_filter.count(sel.type))
			if(!object_filter.at(sel.type))
				return false;
		if(sel.layer == 10000)
			return true;
		if(work_layer_only && (ca->work_layer != sel.layer))
			return false;

		return true;
	}
}
