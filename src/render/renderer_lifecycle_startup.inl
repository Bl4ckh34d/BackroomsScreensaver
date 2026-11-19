    // Renderer startup, shutdown, settings bootstrap, and audio preparation.

#include "renderer_lifecycle_bootstrap.inl"
#include "renderer_initialize.inl"
#include "renderer_startup_audio.inl"

#if !defined(BACKROOMS_GAME_EXE)
#include "renderer_self_test.inl"
#endif
