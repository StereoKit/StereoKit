#include <stereokit.h>
using namespace sk;

mesh_t cube;
material_t orange_mat;

int main() {
	// log_set_filter(log_diagnostic);
	sk_init("StereoKit Linux Test (OpenXR)", runtime_mixedreality);
	// sk_init("StereoKit Linux Test (Flatscreen)", runtime_flatscreen);

	cube = mesh_gen_rounded_cube(vec3_one, 0.1f, 4);

	orange_mat = material_copy(material_find(default_id_material));
	material_set_color(orange_mat, "color", { 1,0.5,0,1 });

	while (sk_step([]() {
		render_add_mesh(cube, orange_mat, matrix_trs(vec3_forward+(vec3_up*sin(time_getf()*1.25f)*0.25f), quat_identity, vec3_one*0.5));
	})) {}
	sk_shutdown();
}
