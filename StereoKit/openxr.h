#pragma once

bool openxr_init          (const char *app_name);
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
void openxr_poll_events   ();
void openxr_render_frame  ();
void openxr_make_actions  ();
void openxr_poll_actions  ();
