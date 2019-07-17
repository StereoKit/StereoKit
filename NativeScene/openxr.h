#pragma once

#pragma comment(lib,"openxr_loader-0_90.lib")

bool openxr_init          (const char *app_name);
void openxr_shutdown      ();
void openxr_poll_events   (bool &exit, bool &has_focus);
void openxr_render_frame  ();
void openxr_make_actions  ();
void openxr_poll_actions  ();
void openxr_poll_predicted(XrTime predicted_time);
