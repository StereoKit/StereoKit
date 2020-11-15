#include <stereokit.h>
using namespace sk;

int main() {
  log_set_filter(log_diagnostic);
  sk_init("StereoKit Linux Test", runtime_flatscreen);
  while (sk_step([]() {})) {}
  sk_shutdown();
}
//
// mesh_t     cube;
// material_t mat;
//
// int main() {
//     log_set_filter(log_diagnostic);
//     sk_init("Fabrica C", runtime_flatscreen);
// 	render_set_clear_color({ 1,1,1,1 });
// 	render_enable_skytex(false);
//
//     cube = mesh_find    (default_id_mesh_cube);
//     mat  = material_find(default_id_material);
//
//     while (sk_step([]() {
//         render_add_mesh(cube, mat, matrix_trs(vec3_zero, quat_identity, vec3_one*0.01));
//     })) {}
//     sk_shutdown();
// }
