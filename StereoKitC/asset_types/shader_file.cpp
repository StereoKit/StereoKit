#include "shader_file.h"
#include "shader.h"

#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../systems/defaults.h"

namespace sk {

tex_t shader_stref_to_tex(const stref_t &tex_name) {
	if      (stref_equals(tex_name, "white")) return tex_find(default_id_tex);
	else if (stref_equals(tex_name, "black")) return tex_find(default_id_tex_black);
	else if (stref_equals(tex_name, "gray" )) return tex_find(default_id_tex_gray);
	else if (stref_equals(tex_name, "flat" )) return tex_find(default_id_tex_flat);
	else if (stref_equals(tex_name, "rough")) return tex_find(default_id_tex_rough);
	else                                      return tex_find(default_id_tex);
}

} // namespace sk