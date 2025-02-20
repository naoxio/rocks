#include "components/markdown.h"
#include <stdlib.h>
#include <string.h>

// Helper function to read entire file
static char* read_file_contents(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, length, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    return buffer;
}
static void convert_markdown_node_to_clay(cmark_node* node, Rocks_Markdown* viewer) {
    Rocks_Theme theme = Rocks_GetTheme(GRocks);

    switch (cmark_node_get_type(node)) {
        case CMARK_NODE_DOCUMENT:
            for (cmark_node* child = cmark_node_first_child(node); child; child = cmark_node_next(child)) {
                convert_markdown_node_to_clay(child, viewer);
            }
            break;
        
        case CMARK_NODE_HEADING: {
            int level = cmark_node_get_heading_level(node);
            cmark_node* text_node = cmark_node_first_child(node);
            const char* heading_text = (text_node && cmark_node_get_type(text_node) == CMARK_NODE_TEXT) ? cmark_node_get_literal(text_node) : NULL;
            if (heading_text) {
                CLAY({
                    .layout = { .sizing = CLAY_SIZING_FIT(0), .padding = CLAY_PADDING_ALL(10) }
                }) {
                    static char text_buffer[1024];
                    size_t text_length = strlen(text_buffer);
                    const Clay_String clay_heading_text = (Clay_String){ .length = text_length, .chars = text_buffer };                
                    CLAY_TEXT(
                        clay_heading_text,
                        CLAY_TEXT_CONFIG({
                            .fontSize = 24 - (level * 3),
                            .textColor = theme.text,
                            .fontId = viewer->config.base_font_id
                        })
                    );
                }
            }
            break;
        }
        
        case CMARK_NODE_PARAGRAPH: {
            cmark_node* text_node = cmark_node_first_child(node);
            const char* paragraph_text = (text_node && cmark_node_get_type(text_node) == CMARK_NODE_TEXT) ? cmark_node_get_literal(text_node) : NULL;
            if (paragraph_text) {
                CLAY({
                    .layout = { .sizing = CLAY_SIZING_FIT(0), .padding = CLAY_PADDING_ALL(5) }
                }) {
                    static char text_buffer[1024];
                    size_t text_length = strlen(text_buffer);
                    const Clay_String clay_paragraph_text = (Clay_String){ .length = text_length, .chars = text_buffer };
                    CLAY_TEXT(
                        clay_paragraph_text,
                        CLAY_TEXT_CONFIG({
                            .fontSize = 16,
                            .textColor = theme.text,
                            .fontId = viewer->config.base_font_id
                        })
                    );
                }
            }
            break;
        }
        
        case CMARK_NODE_CODE_BLOCK: {
            const char* code_text = cmark_node_get_literal(node);
            if (code_text && strlen(code_text) > 0) {
                CLAY({
                    .layout = { .sizing = CLAY_SIZING_FIT(0), .padding = CLAY_PADDING_ALL(10) },
                    .backgroundColor = theme.secondary,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                }) {
                    static char text_buffer[1024];
                    size_t text_length = strlen(text_buffer);
                    const Clay_String clay_code_text = (Clay_String){ .length = text_length, .chars = text_buffer };
                    CLAY_TEXT(
                        clay_code_text,
                        CLAY_TEXT_CONFIG({
                            .fontSize = 14,
                            .textColor = theme.text_secondary,
                            .fontId = viewer->config.code_font_id,
                            .wrapMode = CLAY_TEXT_WRAP_NONE
                        })
                    );
                }
            }
            break;
        }
        
        case CMARK_NODE_LIST: {
            bool is_ordered = cmark_node_get_list_type(node) == CMARK_ORDERED_LIST;
            CLAY({
                .layout = { 
                    .sizing = CLAY_SIZING_FIT(0),
                    .padding = CLAY_PADDING_ALL(5),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }
            }) {
                for (cmark_node* child = cmark_node_first_child(node); child; child = cmark_node_next(child)) {
                    convert_markdown_node_to_clay(child, viewer);
                }
            }
            break;
        }
        
        case CMARK_NODE_ITEM: {
            cmark_node* text_node = cmark_node_first_child(node);
            if (text_node && cmark_node_get_type(text_node) == CMARK_NODE_PARAGRAPH) {
                text_node = cmark_node_first_child(text_node);
            }
            const char* item_text = (text_node && cmark_node_get_type(text_node) == CMARK_NODE_TEXT) ? cmark_node_get_literal(text_node) : NULL;
            if (item_text) {
                CLAY({
                    .layout = { .sizing = CLAY_SIZING_FIT(0), .padding = { .left = 20 } }
                }) {
                    CLAY_TEXT(
                        CLAY_STRING("• "),
                        CLAY_TEXT_CONFIG({
                            .textColor = theme.text,
                            .fontId = viewer->config.base_font_id
                        })
                    );
                    static char text_buffer[1024];
                    size_t text_length = strlen(text_buffer);
                    const Clay_String clay_text = (Clay_String){ .length = text_length, .chars = text_buffer };
                    CLAY_TEXT(
                        clay_text,
                        CLAY_TEXT_CONFIG({
                            .textColor = theme.text,
                            .fontId = viewer->config.base_font_id
                        })
                    );
                }
            }
            break;
        }
        
        case CMARK_NODE_STRONG:  // Bold text
        case CMARK_NODE_EMPH:    // Italic text
            for (cmark_node* child = cmark_node_first_child(node); child; child = cmark_node_next(child)) {
                if (cmark_node_get_type(child) == CMARK_NODE_TEXT) {
                    const char* text = cmark_node_get_literal(child);
                    if (text) {
                        CLAY({
                            .layout = { .sizing = CLAY_SIZING_FIT(0), .padding = CLAY_PADDING_ALL(5) }
                        }) {
                            static char text_buffer[1024];
                            size_t text_length = strlen(text_buffer);
                            const Clay_String clay_text = (Clay_String){ .length = text_length, .chars = text_buffer };
                            CLAY_TEXT(
                                clay_text,
                                CLAY_TEXT_CONFIG({
                                    .fontSize = 16,
                                    .textColor = theme.text,
                                    .fontId = viewer->config.base_font_id
                                })
                            );
                        }
                    }
                }
            }
            break;

        default:
            break;
    }

    if (viewer->custom_renderer) {
        viewer->custom_renderer(node, viewer->custom_renderer_data);
    }
}

Rocks_Markdown* Rocks_CreateMarkdownViewer(
    uint16_t base_font_id, 
    uint16_t code_font_id
) {
    Rocks_Markdown* viewer = calloc(1, sizeof(Rocks_Markdown));
    if (!viewer) return NULL;

    viewer->config.base_font_id = base_font_id;
    viewer->config.code_font_id = code_font_id;
    viewer->config.highlight_code = true;

    return viewer;
}

bool Rocks_LoadMarkdownFromFile(
    Rocks_Markdown* viewer, 
    const char* filepath
) {
    if (!viewer) return false;

    // Free any existing markdown text
    free(viewer->markdown_text);

    viewer->markdown_text = read_file_contents(filepath);
    return viewer->markdown_text != NULL;
}

bool Rocks_LoadMarkdownFromString(
    Rocks_Markdown* viewer, 
    const char* markdown_text
) {
    if (!viewer || !markdown_text) return false;

    // Free any existing markdown text
    free(viewer->markdown_text);

    // Create a copy of the markdown text
    viewer->markdown_text = strdup(markdown_text);
    return viewer->markdown_text != NULL;
}

void Rocks_RenderMarkdown(Rocks_Markdown* viewer) {
    if (!viewer || !viewer->markdown_text) {
        return;
    }

    cmark_node* root = cmark_parse_document(
        viewer->markdown_text, 
        strlen(viewer->markdown_text), 
        CMARK_OPT_DEFAULT
    );
    if (!root) {
        return;
    }

    // Wrap all Markdown elements in a vertical layout
    CLAY({
        .layout = { 
            .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 10  // Add spacing between elements
        }
    }) {
        convert_markdown_node_to_clay(root, viewer);
    }
    
    cmark_node_free(root);
}

void Rocks_DestroyMarkdownViewer(Rocks_Markdown* viewer) {
    if (!viewer) return;

    free(viewer->markdown_text);
    free(viewer);
}

void Rocks_SetMarkdownCustomRenderer(
    Rocks_Markdown* viewer,
    void (*custom_renderer)(cmark_node* node, void* user_data),
    void* user_data
) {
    if (!viewer) return;

    viewer->custom_renderer = custom_renderer;
    viewer->custom_renderer_data = user_data;
}
