#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filePath)
{
	enum class ShaderType
	{
		None = -1,
		Vertex = 0,
		Fragment = 1,
	};
	std::ifstream ifs(filePath);
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::None;
	while (std::getline(ifs, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::Vertex;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::Fragment;
			}
		}
		else
		{
			if (type != ShaderType::None)
			{
				ss[static_cast<int>(type)] << line << "\n";
			}
		}
	}
	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int res = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE)
	{
		int len = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		char* buf = new char[len];
		glGetShaderInfoLog(id, len, &len, buf);
		std::cout << type << " failed!: ";
		std::cout << buf << std::endl;
		delete[] buf;
		glDeleteShader(id);
	}
	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs); 

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return -1;

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[] = {
		-0.5, -0.5,
		 0.5, -0.5,
		 0.5,  0.5,
		-0.5,  0.5
	};
	
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	auto var = ParseShader("res/shaders/Basic.shader");

	unsigned int bo;
	// Generate a buffer and give us its ID
	glGenBuffers(1, &bo);
	// Bind newly generated buffer
	// GL_ARRAY_BUFFER for vertex attributes
	// Binding is like selecting the buffer for opengl state machine
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	// Create and initialize buffer's data store
	// GL_ARRAY_BUFFER for vertex attributes
	// size is in bytes
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int program = CreateShader(var.VertexSource, var.FragmentSource);
	glUseProgram(program);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, nullptr);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		
		//glfwWaitEventsTimeout(10);
		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(program);
	glfwTerminate();
	return 0;
}