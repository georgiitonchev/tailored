#include "tailored.h"

#include "../dep/include/glad/glad.h"

#define CGLTF_IMPLEMENTATION
#include "../dep/include/cgltf/cgltf.h"

t_mesh process_mesh(cgltf_mesh *cgltf_mesh) {

  t_mesh mesh = {0};

  for (cgltf_size pi = 0; pi < cgltf_mesh->primitives_count; pi++) {
    cgltf_primitive primitive = cgltf_mesh->primitives[pi];

    if (primitive.indices != NULL) {
			mesh.has_indices = true;
      	cgltf_accessor *data = primitive.indices;
      	unsigned short *indices = malloc(data->count * sizeof(unsigned short));

      	unsigned short *buffer =
          	(unsigned short *)data->buffer_view->buffer->data +
          	data->buffer_view->offset / sizeof(unsigned short) +
          	data->offset / sizeof(unsigned short);

      	int n = 0;
      	for (unsigned int i = 0; i < data->count; i++) {
        	indices[i] = buffer[n];
        	n += (int)(data->stride / sizeof(unsigned short));
      	}

      	mesh.indices_count = data->count;
      	mesh.indices = indices;
    }

		mesh.vertices_count = primitive.attributes[0].data->count;
		t_vertex* vertices = malloc(mesh.vertices_count * sizeof(t_vertex));

    for (cgltf_size ai = 0; ai < primitive.attributes_count; ai++) {

			cgltf_attribute attribute = primitive.attributes[ai];
			cgltf_accessor *data = attribute.data;

			if (attribute.type == cgltf_attribute_type_position) {

					int n = 0;
					float *buffer = (float *)data->buffer_view->buffer->data +
													data->buffer_view->offset / sizeof(float) +
													data->offset / sizeof(float);

					for (unsigned int k = 0; k < data->count; k++) {
							t_vec3 vertex = {0};
							vertex.x = buffer[n];
							vertex.y = buffer[n + 1];
							vertex.z = buffer[n + 2];

							vertices[k].position = vertex;

							n += (int)(data->stride / sizeof(float));
					}
			}
			else if (attribute.type == cgltf_attribute_type_normal) {

					int n = 0;
					float *buffer = (float *)data->buffer_view->buffer->data +
													data->buffer_view->offset / sizeof(float) +
													data->offset / sizeof(float);

					for (unsigned int k = 0; k < data->count; k++) {
							t_vec3 normal = {0};
							normal.x = buffer[n];
							normal.y = buffer[n + 1];
							normal.z = buffer[n + 2];

							vertices[k].normal = normal;

							n += (int)(data->stride / sizeof(float));
					}
			}
    }

		mesh.vertices = vertices;
  }
  return mesh;
}

void process_node(cgltf_node *node) {

  process_mesh(node->mesh);

  for (cgltf_size i = 0; i < node->children_count; i++) {
    process_node(node->children[i]);
  }
}

void process_scene(cgltf_scene *scene) {

  for (cgltf_size i = 0; i < scene->nodes_count; i++) {
    process_node(scene->nodes[i]);
  }
}

void process_gltf_file(const char* path, t_model* model) {

    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result == cgltf_result_success) {

        printf(".gltf file loaded successfuly\n");

        result = cgltf_load_buffers(&options, data, path);

				if (result == cgltf_result_success) {

					 printf(".gltf buffers loaded successfuly\n");
					 for (cgltf_size i = 0; i < data->meshes_count; i++) {
							model->meshes = malloc(sizeof(t_mesh) * data->meshes_count);
							model->meshes[i] = process_mesh(&data->meshes[i]);
        	}

						printf("meshes processed.\n");
				}
				else{
					printf("error loading .gltf buffers\n");
				}
        // process_scene(data->scene);

        cgltf_free(data);
				printf("free data.\n");
    } else {
        printf("error loading .gltf file\n");
    }
}

void setup_mesh(t_mesh* mesh)
{
    //generate arrays and buffers
    glGenVertexArrays(1, &mesh->gl_vertex_array); //VAO
    glGenBuffers(1, &mesh->gl_array_buffer); //VBO
    glGenBuffers(1, &mesh->gl_element_array_buffer); //EBO

    glBindVertexArray(mesh->gl_vertex_array); //VAO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_array_buffer); //VBO

    //bind vertices data
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(t_vertex), mesh->vertices, GL_STATIC_DRAW); // mesh->vertices might need to be something else

		if (mesh->has_indices) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gl_element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(unsigned short), mesh->indices, GL_STATIC_DRAW); //mesh->indices might need to be something else
		}

		//positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(t_vertex), 0); //may need to cast 0 to void*

		//normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(t_vertex), (void *)offsetof(t_vertex, normal));

    glBindVertexArray(0);
}

void draw_mesh(t_mesh* mesh, unsigned int shader_program) 
{
    glUseProgram(shader_program);
    glBindVertexArray(mesh->gl_vertex_array);

	if (mesh->has_indices)
		glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_SHORT, 0);
	else
		glDrawArrays(GL_TRIANGLES, 0, mesh->vertices_count);
}