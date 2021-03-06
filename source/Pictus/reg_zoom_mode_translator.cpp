#include "reg_zoom_mode_translator.h"

namespace Reg {
	namespace Internal {
		boost::optional<ZoomModeTranslator::external_type> ZoomModeTranslator::get_value(internal_type const sval) {
			if (sval == "0") return App::ZoomFree;
			if (sval == "1") return App::ZoomFitImage;
			if (sval == "2") return App::ZoomFullSize;

			auto it = App::ZoomModeIdentifier.left.find(sval);
			if (it != App::ZoomModeIdentifier.left.end()) {
				return it->second;
			}
			return boost::none;
		}

		boost::optional<ZoomModeTranslator::internal_type> ZoomModeTranslator::put_value(external_type const& val) {
			return App::ZoomModeIdentifier.right.find(val)->second;
		}
	}
}
