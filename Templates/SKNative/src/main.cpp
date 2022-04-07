#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

mesh_t     sphere_mesh;
material_t sphere_mat;
pose_t     sphere_pose = pose_identity;

int main(void) {
    sk_settings_t settings = {};
	settings.app_name           = "SKNativeTemplate";
	settings.assets_folder      = "Assets";
	settings.display_preference = display_mode_mixedreality;
	if (!sk_init(settings))
		return 1;

    sphere_mesh = mesh_find    (default_id_mesh_sphere);
    sphere_mat  = material_find(default_id_material);

    while (sk_step( []() {
        ui_handle_begin("Sphere", sphere_pose, mesh_get_bounds(sphere_mesh), false);
        render_add_mesh(sphere_mesh, sphere_mat, matrix_identity);
        ui_handle_end();
	}));

    sk_shutdown();
    return 0;
}