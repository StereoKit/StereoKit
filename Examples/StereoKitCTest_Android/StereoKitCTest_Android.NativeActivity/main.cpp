#include <stereokit.h>
#include <stereokit_ui.h>
#include <stdlib.h>
using namespace sk;

void android_request_permission(struct android_app *app, const char *permission);
bool android_has_permission(struct android_app *app, const char *perm_name);

android_app* app;
bool app_visible = false;
bool app_run = true;

mesh_t mesh;
material_t mat;

static void engine_handle_cmd(android_app*evt_app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:    sk_set_window(evt_app->window); break;
	case APP_CMD_WINDOW_RESIZED:
	case APP_CMD_CONFIG_CHANGED: sk_set_window(evt_app->window); break;
	case APP_CMD_TERM_WINDOW:    sk_set_window(nullptr        ); break;
	case APP_CMD_GAINED_FOCUS:   app_visible = true; break;
	case APP_CMD_LOST_FOCUS:     app_visible = false; break;
	}
}

pose_t pose = { vec3{0,0.5f,0}, quat_identity };
void android_main(struct android_app* state) {
	app = state;
	app->onAppCmd = engine_handle_cmd;

	log_set_filter(log_diagnostic);
	if (!android_has_permission(app, "RECORD_AUDIO")) {
		android_request_permission(app, "RECORD_AUDIO");
	}

	sk_settings_t settings = {};
	settings.app_name           = "StereoKitCTest_Android";
	settings.android_activity   = state->activity->clazz;
	settings.android_java_vm    = state->activity->vm;
	settings.mode               = app_mode_xr;
	if (!sk_init(settings))
		return;

	mesh = mesh_gen_rounded_cube(vec3_one*.4f, 0.05f, 4);
	mat  = material_find(default_id_material);
	render_enable_skytex(false);

	while (app_run) {
		int events;
		struct android_poll_source* source;
		while (ALooper_pollAll(app_visible ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
			if (source                  != nullptr) source->process(state, source);
			if (state->destroyRequested != 0      ) app_run = false;
		}

		if (app_visible) sk_step([]() {
			render_add_mesh(mesh, mat, matrix_trs(vec3_zero));

			if (device_display_get_type() == display_type_flatscreen) {
				vec3 pos = { cosf(time_totalf())*.6f, 0.5f, sinf(time_totalf())*.6f };
				render_set_cam_root(matrix_trs(pos, quat_lookat(pos, vec3{0,0.25f,0})));
			}

			ui_window_begin("Hello!", pose);
			ui_button("play");
			ui_window_end();
		});
	}

	sk_shutdown();
}

/**
* \brief Gets the internal name for an android permission.
* \param[in] lJNIEnv a pointer to the JNI environment
* \param[in] perm_name the name of the permission, e.g.,
*   "READ_EXTERNAL_STORAGE", "WRITE_EXTERNAL_STORAGE".
* \return a jstring with the internal name of the permission,
*   to be used with android Java functions 
*   Context.checkSelfPermission() or Activity.requestPermissions()
*/
jstring android_permission_name(JNIEnv* lJNIEnv, const char* perm_name) {
	// nested class permission in class android.Manifest,
	// hence android 'slash' Manifest 'dollar' permission
	jclass   ClassManifestpermission = lJNIEnv->FindClass( "android/Manifest$permission" );
	jfieldID lid_PERM                = lJNIEnv->GetStaticFieldID( ClassManifestpermission, perm_name, "Ljava/lang/String;" );
	jstring  ls_PERM                 = (jstring)(lJNIEnv->GetStaticObjectField( ClassManifestpermission, lid_PERM )); 
	return ls_PERM;
}

bool android_has_permission(struct android_app* app, const char* perm_name) {
	JavaVM* lJavaVM = app->activity->vm;
	JNIEnv* lJNIEnv = nullptr; 
	bool    lThreadAttached = false;

	// Get JNIEnv from lJavaVM using GetEnv to test whether
	// thread is attached or not to the VM. If not, attach it
	// (and note that it will need to be detached at the end
	//  of the function).
	switch (lJavaVM->GetEnv((void**)&lJNIEnv, JNI_VERSION_1_6)) {
	case JNI_OK: break;
	case JNI_EVERSION: return false;
	case JNI_EDETACHED: {
		jint lResult = lJavaVM->AttachCurrentThread(&lJNIEnv, nullptr);
		if(lResult == JNI_ERR) {
			return false;
		}
		lThreadAttached = true;
	} break;
	}

	bool result = false;

	jstring ls_PERM = android_permission_name(lJNIEnv, perm_name);

	jint PERMISSION_GRANTED = jint(-1);
	{
		jclass ClassPackageManager = lJNIEnv->FindClass( "android/content/pm/PackageManager" );
		jfieldID lid_PERMISSION_GRANTED = lJNIEnv->GetStaticFieldID( ClassPackageManager, "PERMISSION_GRANTED", "I" );
		PERMISSION_GRANTED = lJNIEnv->GetStaticIntField( ClassPackageManager, lid_PERMISSION_GRANTED );
	}
	{
		jobject   activity = app->activity->clazz;
		jclass    ClassContext = lJNIEnv->FindClass( "android/content/Context" );
		jmethodID MethodcheckSelfPermission = lJNIEnv->GetMethodID( ClassContext, "checkSelfPermission", "(Ljava/lang/String;)I" );
		jint      int_result = lJNIEnv->CallIntMethod( activity, MethodcheckSelfPermission, ls_PERM );
		result = (int_result == PERMISSION_GRANTED);
	}

	if(lThreadAttached) {
		lJavaVM->DetachCurrentThread();
	}

	return result;
}

/**
* \brief Query file permissions.
* \details This opens the system dialog that lets the user
*  grant (or deny) the permission.
* \param[in] app a pointer to the android app.
* \note Requires Android API level 23 (Marshmallow, May 2015)
*/
void android_request_permission(struct android_app* app, const char *permission) {
	JavaVM* lJavaVM = app->activity->vm;
	JNIEnv* lJNIEnv = nullptr; 
	bool lThreadAttached = false;

	// Get JNIEnv from lJavaVM using GetEnv to test whether
	// thread is attached or not to the VM. If not, attach it
	// (and note that it will need to be detached at the end
	//  of the function).
	switch (lJavaVM->GetEnv((void**)&lJNIEnv, JNI_VERSION_1_6)) {
	case JNI_OK: break;
	case JNI_EVERSION: return;
	case JNI_EDETACHED: {
		jint lResult = lJavaVM->AttachCurrentThread(&lJNIEnv, nullptr);
		if(lResult == JNI_ERR) {
			return;
		}
		lThreadAttached = true;
	} break;
	}

	jobjectArray perm_array = lJNIEnv->NewObjectArray( 1, lJNIEnv->FindClass("java/lang/String"), lJNIEnv->NewStringUTF("") );

	lJNIEnv->SetObjectArrayElement( perm_array, 0, android_permission_name(lJNIEnv, permission) );
	//lJNIEnv->SetObjectArrayElement( perm_array, 1, android_permission_name(lJNIEnv, "WRITE_EXTERNAL_STORAGE") );

	jobject   activity = app->activity->clazz;
	jclass    ClassActivity = lJNIEnv->FindClass("android/app/Activity");
	jmethodID MethodrequestPermissions = lJNIEnv->GetMethodID(ClassActivity, "requestPermissions", "([Ljava/lang/String;I)V");

	// Last arg (0) is just for the callback (that I do not use)
	lJNIEnv->CallVoidMethod(activity, MethodrequestPermissions, perm_array, 0);

	if(lThreadAttached) {
		lJavaVM->DetachCurrentThread();
	}
}