#include "rasterizer_renderer.h"

#include "utils/resource_utils.h"


void cg::renderer::rasterization_renderer::init()
{
	rasterizer = std::make_shared<cg::renderer::rasterizer<cg::vertex, cg::unsigned_color>>();
	rasterizer->set_viewport(settings->width, settings->height);

	render_target = std::make_shared<cg::resource<cg::unsigned_color>>(settings->width, settings->height);
	depth_buffer = std::make_shared<cg::resource<float>>(settings->width, settings->height);
	rasterizer->set_render_target(render_target, depth_buffer);

	load_model();
	load_camera();

}
void cg::renderer::rasterization_renderer::render()
{
	auto start = std::chrono::high_resolution_clock::now();
	rasterizer->clear_render_target({0, 0, 0});

	float4x4 matrix = mul(
			camera->get_projection_matrix(),
			camera->get_view_matrix(),
			model->get_world_matrix()
			);

	rasterizer->vertex_shader = [&](float4 vertex, cg::vertex vertex_data) {
		auto processed = mul(matrix, vertex);
		return std::make_pair(processed, vertex_data);
	};

	rasterizer->pixel_shader = [](cg::vertex vertex_data, float x) {
		return cg::color{vertex_data.ambient_r, vertex_data.ambient_g, vertex_data.ambient_b};
	};

	for (size_t shape_id = 0; shape_id < model->get_index_buffers().size(); shape_id++) {
		rasterizer->set_vertex_buffer(model->get_vertex_buffers()[shape_id]);
		rasterizer->set_index_buffer(model->get_index_buffers()[shape_id]);
		rasterizer->draw(model->get_index_buffers()[shape_id]->get_number_of_elements(), 0);
	}

	auto stop =  std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> duration = stop - start;
	std::cout << "Rasterization took " << duration.count() << " ms\n";

	cg::utils::save_resource(*render_target, settings->result_path);

}

void cg::renderer::rasterization_renderer::destroy() {}

void cg::renderer::rasterization_renderer::update() {}
