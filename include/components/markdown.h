#ifndef ROCKS_MARKDOWN_H
#define ROCKS_MARKDOWN_H

#include <cmark.h>
#include "rocks.h"
#include "rocks_clay.h"

typedef struct {
    // Stores the current markdown content
    char* markdown_text;
    
    // Configuration options for markdown rendering
    struct {
        bool highlight_code;
        bool use_custom_styling;
        uint16_t base_font_id;
        uint16_t code_font_id;
    } config;

    // Optional callback for custom rendering of specific elements
    void (*custom_renderer)(cmark_node* node, void* user_data);
    void* custom_renderer_data;
} Rocks_Markdown;

// Create and initialize a Markdown viewer
Rocks_Markdown* Rocks_CreateMarkdownViewer(
    uint16_t base_font_id, 
    uint16_t code_font_id
);

// Load markdown from a file
bool Rocks_LoadMarkdownFromFile(
    Rocks_Markdown* viewer, 
    const char* filepath
);

// Load markdown from a string
bool Rocks_LoadMarkdownFromString(
    Rocks_Markdown* viewer, 
    const char* markdown_text
);

// Render markdown to Clay render commands
void Rocks_RenderMarkdown(
    Rocks_Markdown* viewer
);

// Free resources associated with the markdown viewer
void Rocks_DestroyMarkdownViewer(Rocks_Markdown* viewer);

// Optional: Set custom rendering callback
void Rocks_SetMarkdownCustomRenderer(
    Rocks_Markdown* viewer,
    void (*custom_renderer)(cmark_node* node, void* user_data),
    void* user_data
);

#endif // ROCKS_MARKDOWN_H
