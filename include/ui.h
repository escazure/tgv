#pragma once 
#include "state.h"
#include "helper.h"

namespace UI {
	void drawMenuBar(AppState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
        if (ImGui::BeginMainMenuBar()) {
            
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save UDF Preset")) { /* Save logic */ }
                if (ImGui::MenuItem("Export Mesh (.obj)")) { /* Export logic */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Camera Configuration", nullptr, state.show_camera_settings_window)) {
                    state.show_camera_settings_window = !state.show_camera_settings_window;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help & Docs")) {
                if (ImGui::MenuItem("Noise & Utility API", nullptr, state.show_noise_window)) {
                    state.show_noise_window = !state.show_noise_window;
                }
                if (ImGui::MenuItem("Keyboard Controls", nullptr, state.show_keybinds_window)) {
                    state.show_keybinds_window = !state.show_keybinds_window;
                }
                ImGui::EndMenu();
            }

            float fps = ImGui::GetIO().Framerate;
            char fps_buf[32];
            snprintf(fps_buf, sizeof(fps_buf), "%.1f FPS", fps);
            float text_width = ImGui::CalcTextSize(fps_buf).x;
            
            ImGui::SameLine(ImGui::GetWindowWidth() - text_width - 20.0f);
            ImGui::TextDisabled("%s", fps_buf);

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();


        if (state.show_noise_window) {
            ImGui::SetNextWindowSize(ImVec2(680.0f, 380.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(state.window_width / 2.0f - 340.0f, state.window_height / 2.0f - 190.0f), ImGuiCond_FirstUseEver);
            
            if (ImGui::Begin("Noise & Utility Functions API", &state.show_noise_window)) {
                ImGui::TextDisabled("Hand-written Utility Library Functions Available in UDF");
                ImGui::Spacing();

                if (ImGui::BeginTable("ApiDocsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableSetupColumn("Function Signature", ImGuiTableColumnFlags_WidthFixed, 280.0f);
                    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    auto AddRow = [](const char* sig, const char* desc) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(sig);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextWrapped("%s", desc);
                    };

                    AddRow("random(x, z, seed)", "Returns semi-random deterministic float in range [0.0, 1.0].");
                    AddRow("lerp(a, b, t)", "Linear interpolation between value a and b by factor t.");
                    AddRow("smooth(t)", "Smoothstep S-curve function (3t² - 2t³) for continuous blending.");
                    AddRow("bell_curve(x, z, r, amp)", "Generates a radial bell curve with specified radius and height.");
                    AddRow("value_noise(x, z)", "2D value noise interpolation map.");
                    AddRow("fbm(x, z, octaves=4)", "Fractional Brownian Motion noise over multiple layered octaves.");
                    AddRow("example(x, z, seed=1000)", "Pre-built complex terrain function stacking multi-octave fBm.");

                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }

        if (state.show_camera_settings_window && state.camera) {
            ImGui::SetNextWindowSize(ImVec2(440.0f, 180.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(state.window_width / 2.0f - 220.0f, state.window_height / 2.0f - 90.0f), ImGuiCond_FirstUseEver);
            
            if (ImGui::Begin("Camera Configuration", &state.show_camera_settings_window)) {
                ImGui::TextDisabled("3D Viewport Controls");
                ImGui::Spacing();

                if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                    ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Movement Speed:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::SliderFloat("##speed", &state.camera->speed, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED, "%.1f units/s");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Far View Distance:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::SliderFloat("##viewdist", &state.camera->view_distance, MIN_CAMERA_VIEW_DISTANCE, MAX_CAMERA_VIEW_DISTANCE, "%.0f units");

                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }

        if (state.show_keybinds_window) {
            ImGui::SetNextWindowSize(ImVec2(380.0f, 300.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(state.window_width / 2.0f - 190.0f, state.window_height / 2.0f - 150.0f), ImGuiCond_FirstUseEver);
            
            if (ImGui::Begin("Keyboard Controls", &state.show_keybinds_window)) {
                ImGui::TextDisabled("Navigation & Controls");
                ImGui::Spacing();

                if (ImGui::BeginTable("KeybindsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Key / Input", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableHeadersRow();

                    auto AddKeyRow = [](const char* action, const char* key) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(action);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextDisabled("%s", key);
                    };

                    AddKeyRow("Move Forward / Backward", "W / S");
                    AddKeyRow("Strafe Left / Right", "A / D");
                    AddKeyRow("Fly Upward", "Space");
                    AddKeyRow("Fly Downward", "Left Ctrl");
                    AddKeyRow("Toggle Mouse Capture", "ESC");

                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }

    }

	void drawGenerationPanel(AppState& state) {
        ImGui::SetNextWindowPos(ImVec2(10.0f, 32.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(420.0f, 540.0f), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Generation", nullptr, ImGuiWindowFlags_None);

        if (ImGui::BeginTabBar("GenerationTabBar", ImGuiTabBarFlags_None)) {
            
            if (ImGui::BeginTabItem("Editor")) {
                ImGui::Spacing();
                
                ImGui::Text("User Defined Function (UDF):");
                ImGui::InputTextMultiline("##udf_textarea", 
                                          state.fun_buf, 
                                          IM_ARRAYSIZE(state.fun_buf), 
                                          ImVec2(-FLT_MIN, 160.0f), 
                                          ImGuiInputTextFlags_AllowTabInput);

                ImGui::Spacing();

                ImGui::Text("Function Name:");
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##fun_name", state.fun_name, IM_ARRAYSIZE(state.fun_name));

                ImGui::Spacing();

                ImGui::Text("Terrain seed:");
				float button_width = 90.0f;
				float input_width = ImGui::GetContentRegionAvail().x - button_width - ImGui::GetStyle().ItemSpacing.x;
				ImGui::SetNextItemWidth(input_width);
				ImGui::InputInt("##fun_seed_int", &state.seed, 1, 100);
				
				ImGui::SameLine();

				if (ImGui::Button("Random", ImVec2(button_width, 0.0f))) {
				    state.seed = rand() % 999999; 
				}

                ImGui::Spacing();
                
                if (ImGui::Button("Compile & Generate Terrain", ImVec2(-FLT_MIN, 36.0f))) {
					std::string udf = wrap_user_input(state.fun_buf, state.fun_name);
					unsigned int status = build_shader(udf, state.fun_name, "height_map_fragment");
					std::string result;
					switch(status){
						case 0: result = "SUCCESS"; break;
						case 1: result = "FAILED_TO_OPEN_FILE"; break;
						case 2: result = "FAILED_TO_WRITE_FILE"; break;
					}

					std::cout << "Shader build finished with status: [" << result << "]\n";
                    state.terrain = new Terrain(state.size, state.step_size, state.chunk_size);
                    state.terrain->generate();
					state.generate_terrain = true;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextDisabled("Grid & Performance Settings");
                ImGui::Spacing();

                static const int size_values[] = { 256, 512, 1024, 2048, 4096, 8192 };
                static const char* size_names[] = { "256 x 256", "512 x 512", "1024 x 1024", "2048 x 2048", "4096 x 4096", "8192 x 8192" };
                
                int current_size_idx = 2;
                for (int i = 0; i < IM_ARRAYSIZE(size_values); i++) {
                    if (size_values[i] == state.size) { current_size_idx = i; break; }
                }

                if (ImGui::Combo("Terrain Size", &current_size_idx, size_names, IM_ARRAYSIZE(size_names))) {
                    state.size = size_values[current_size_idx];
                }

                static const int chunk_values[] = { 16, 32, 64, 128, 256 };
                static const char* chunk_names[] = { "16 x 16", "32 x 32", "64 x 64", "128 x 128", "256 x 256" };

                int current_chunk_idx = 3;
                for (int i = 0; i < IM_ARRAYSIZE(chunk_values); i++) {
                    if (chunk_values[i] == state.chunk_size) { current_chunk_idx = i; break; }
                }

                if (ImGui::Combo("Chunk Size", &current_chunk_idx, chunk_names, IM_ARRAYSIZE(chunk_names))) {
                    state.chunk_size = chunk_values[current_chunk_idx];
                }

                static const int step_values[] = { 1, 2, 4, 8, 16 };
                static const char* density_names[] = {
                    "Ultra High (Step: 1 unit)",
                    "High (Step: 2 units)",
                    "Medium (Step: 4 units)",
                    "Low (Step: 8 units)",
                    "Very Low (Step: 16 units)"
                };

                int current_density_idx = 3; 
                for (int i = 0; i < IM_ARRAYSIZE(step_values); i++) {
                    if (step_values[i] == state.step_size) { current_density_idx = i; break; }
                }

                if (ImGui::Combo("Vertex Density", &current_density_idx, density_names, IM_ARRAYSIZE(density_names))) {
                    state.step_size = step_values[current_density_idx];
                }


                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Presets")) {
                ImGui::TextWrapped("Preset UDF templates will be listed here.");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
	
	void drawVisualizationPanel(AppState& state) {
        ImGui::SetNextWindowPos(ImVec2(state.window_width - 370.0f, 32.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360.0f, 540.0f), ImGuiCond_FirstUseEver);

        ImGui::Begin("Visualization & Metrics", nullptr, ImGuiWindowFlags_None);

        if (ImGui::BeginTabBar("VisualizationTabBar", ImGuiTabBarFlags_None)) {

            if (ImGui::BeginTabItem("Statistics")) {
                ImGui::Spacing();
                
                ImGui::TextDisabled("Engine Performance");
                ImGui::Spacing();

                if (ImGui::BeginTable("PerfStatsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Framerate (FPS):");
                    ImGui::TableSetColumnIndex(1);
                    float fps = ImGui::GetIO().Framerate;
                    ImGui::Text("%.1f FPS (%.2f ms)", fps, 1000.0f / (fps > 0.0f ? fps : 1.0f));

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Generation Time:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.2f ms", (state.terrain_generated && state.terrain) ? state.gen_time : 0.0f);

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextDisabled("Terrain Geometry Metrics");
                ImGui::Spacing();

                if (ImGui::BeginTable("TerrainDataTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Triangle Count:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", (state.terrain_generated && state.terrain) ? state.terrain->triangle_count : 0);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Min Height:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.2f units", (state.terrain_generated && state.terrain) ? state.terrain->min_height : 0.0f);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Max Height:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.2f units", (state.terrain_generated && state.terrain) ? state.terrain->max_height : 0.0f);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Height Range:");
                    ImGui::TableSetColumnIndex(1);
                    float height_range = (state.terrain_generated && state.terrain) ? (state.terrain->max_height - state.terrain->min_height) : 0.0f;
                    ImGui::Text("%.2f units", height_range);

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Shading & Options")) {
                ImGui::Spacing();

                ImGui::TextDisabled("Lighting & Surface Render");
                ImGui::Spacing();

                ImGui::Checkbox("Enable Realtime Lighting", &state.calculate_lighting);
                ImGui::Checkbox("Wireframe Overlay Mode", &state.is_wireframe_mode);
                ImGui::Checkbox("Backface Culling", &state.cull_backface);
                ImGui::Checkbox("Render Skybox Environment", &state.render_skybox);
                ImGui::Checkbox("Render Terrain Skirt", &state.render_terrain_skirt);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextDisabled("Debug Overlays & Helpers");
                ImGui::Spacing();

                ImGui::Checkbox("Show Surface Normals", &state.show_normals);
                ImGui::Checkbox("Show Light Frustum", &state.show_light_frustum);
                ImGui::Checkbox("Show Light Marker Gizmo", &state.show_light_marker);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextDisabled("Data Color Mapping");
                ImGui::Spacing();

                static int color_mode_idx = 0;
                static const char* color_modes[] = { 
                    "Default Surface Shading", 
                    "Height Ramp (Viridis)", 
                    "Contour Lines (Isolines)"
                };
                
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::Combo("##colormode", &color_mode_idx, color_modes, IM_ARRAYSIZE(color_modes));

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
