#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm-master/glm-master/glm/glm.hpp>

#include "texture.h"
#include "shader.h"

struct character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
public:
    std::map<char, character> characters; // map of all characters
    Shader TextShader;
    
    TextRenderer(unsigned int width, unsigned int height);
    void Load(std::string font, unsigned int fontSize);
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));
    float GetTextWidth(const std::string& text, float scale);
    float GetTextHeight(const std::string& text, float scale);

private:
    unsigned int VAO, VBO;
};

#endif // !TEXT_RENDERER_H
