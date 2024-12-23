#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include "Header.h"

Shape shape;
Fill fill;
std::vector<Vertex> verteces;
Color u_color = { 1, 0, 0, 1 };
std::vector<Color> gradient_colors;

const GLfloat Pi = 3.14159274101257324219f;

void changeParams()
{
    switch (shape)
    {
    case Triangle:
        verteces = {
            { -1.0f, -1.0f },
            { 0.0f, 1.0f },
            { 1.0f, -1.0f }
        };
        if (fill == Gradient) gradient_colors = GenerateColors(3);
        break;
    case Fan:
        verteces.clear();
        verteces.push_back({ 0.0f, 0.0f });
        for (GLfloat angle = 15.0f; angle <= 165.0f; angle += 30.0f)
            verteces.push_back({ cos(angle / 180.0f * Pi), sin(angle / 180.0f * Pi) });
        if (fill == Gradient) gradient_colors = GenerateColors(7);
        break;
    case Quadrangle:
        verteces = {
            { -0.5f, -0.5f },
            { -0.5f, 0.5f },
            { 0.5f, 0.5f },
            { 0.5f, -0.5f }
        };
        if (fill == Gradient) gradient_colors = GenerateColors(3);
        break;
    case Pentagon: {
        verteces.clear();
        GLfloat startAngle = 0.0f;
        for (GLfloat angle = startAngle; angle < 360.0f + startAngle; angle += 72.0f)
            verteces.push_back({ cos(angle / 180.0f * Pi), sin(angle / 180.0f * Pi) });

        if (fill == Gradient) gradient_colors = GenerateColors(5);
        break;
    }
    default: break;
    }
}

int main() {
    shape = Pentagon;
    fill = Hardcode;

    u_color = { 1, 0, 1, 1 };

    changeParams();

    sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);
    glewInit();

    Init();
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); break; }
            else if (event.type == sf::Event::Resized) { glViewport(0, 0, event.size.width, event.size.height); }
            // ��������� ����� � ����������
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num1) {
                    shape = Pentagon;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::Num2) {
                    shape = Triangle;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::Num3) {
                    shape = Quadrangle;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::Num4) {
                    shape = Fan;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::Q) {
                    fill = Hardcode;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::W) {
                    fill = Uniform;
                    changeParams();
                    Init();
                }
                if (event.key.code == sf::Keyboard::E) {
                    fill = Gradient;
                    changeParams();
                    Init();
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    std::cout << "����� �� ���������\n";
                    window.close();
                }
            }
        }
        if (!window.isOpen()) continue;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Draw();
        window.display();
    }
    Release();
    return 0;
}


void InitVBO() {
	glGenBuffers(1, &VBO);
	// �������� ������� � �����
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verteces.size() * 2 * sizeof(GLfloat), verteces.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &CBO);
	glBindBuffer(GL_ARRAY_BUFFER, CBO);
	glBufferData(GL_ARRAY_BUFFER, gradient_colors.size() * 4 * sizeof(GLfloat), gradient_colors.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	checkOpenGLerror(); //������ ������� ���� � ������������
	// �������� ������ OpenGL, ���� ����, �� ����� � ������� ��� ������
}

void InitShader() {
    // ������� ��������� ������
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // �������� �������� ���

    if (fill == Gradient)
        glShaderSource(vShader, 1, &GradientVertexShaderSource, NULL);
    else
        glShaderSource(vShader, 1, &VertexShaderSource, NULL);

    // ����������� ������
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    // ������� ������ ���� �������
    ShaderLog(vShader); //������ ������� ���� � ������������

    // ������� ����������� ������
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // �������� �������� ���

    switch (fill) {
    case Hardcode: glShaderSource(fShader, 1, &FragShaderSource, NULL); break;
    case Uniform: glShaderSource(fShader, 1, &UniformFragShaderSource, NULL); break;
    case Gradient: glShaderSource(fShader, 1, &GradientFragShaderSource, NULL); break;
    default:break;
    }

    // ����������� ������
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    // ������� ������ ���� �������
    ShaderLog(fShader);

    // ������� ��������� � ����������� ������� � ���
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    // ������� ��������� ���������
    glLinkProgram(Program);
    // ��������� ������ ������
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cout << "error attach shaders \n";
        return;
    }

    // ���������� ID �������� �� ��������� ���������
    const char* attr_name = "coord"; //��� � �������
    Attrib_vertex = glGetAttribLocation(Program, attr_name);
    if (Attrib_vertex == -1) {
        std::cout << "could not bind attrib " << attr_name << std::endl;
        return;
    }

    checkOpenGLerror();
}

void Init() {
    // �������
    InitShader();
    // ��������� �����
    InitVBO();
}

void Draw() {
    glUseProgram(Program); // ������������� ��������� ��������� �������

    if (fill == Uniform) {
        GLuint u_color_id = glGetUniformLocation(Program, "u_color");
        glUniform4f(u_color_id, u_color.r, u_color.g, u_color.b, u_color.a);
    }

    glEnableVertexAttribArray(Attrib_vertex); // �������� ������ ���������
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // ���������� VBO
    // �������� OpenGL ��� �� ������ ���������������� ��������� ������.
    glVertexAttribPointer(Attrib_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // ��������� VBO

    if (fill == Gradient) {
        v_color_id = glGetAttribLocation(Program, "vertexColor");
        glEnableVertexAttribArray(v_color_id);
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glVertexAttribPointer(v_color_id, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // �������� ������ �� ����������(������)
    switch (shape) {
    case Triangle: glDrawArrays(GL_TRIANGLES, 0, verteces.size()); break;
    case Fan: glDrawArrays(GL_TRIANGLE_FAN, 0, verteces.size()); break;
    case Quadrangle: glDrawArrays(GL_QUADS, 0, verteces.size()); break;
    case Pentagon: glDrawArrays(GL_TRIANGLE_FAN, 0, verteces.size()); break;
    default: break;
    }

    glDisableVertexAttribArray(Attrib_vertex); // ��������� ������ ���������
    if (fill == Gradient)
        glDisableVertexAttribArray(v_color_id);
    glUseProgram(0); // ��������� ��������� ���������

    checkOpenGLerror();
}

// ������������ ������
void ReleaseVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
}

// ������������ ��������
void ReleaseShader() {
    // ��������� ����, �� ��������� ��������� ���������
    glUseProgram(0);
    // ������� ��������� ���������
    glDeleteProgram(Program);
}

void Release() {
    // �������
    ReleaseShader();
    // ��������� �����
    ReleaseVBO();
}

std::vector<Color> GenerateColors(int n) {
    std::vector<Color> res;
    for (int i = 0; i < n; i++)
        res.push_back({ (GLfloat)std::rand() / RAND_MAX, (GLfloat)std::rand() / RAND_MAX, (GLfloat)std::rand() / RAND_MAX, 1 });

    return res;
}

void checkOpenGLerror()
{
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cerr << "OpenGL Error Code: " << std::hex << err << std::dec << std::endl;
	}
}

void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
		std::cout << "InfoLog: " << infoLog.data() << std::endl;
	}
}