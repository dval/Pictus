#include <GL/glew.h>
#include "hw3d_opengl_context.h"
#include "hw3d_opengl_texture_pbo.h"
#include "hw3d_opengl_common.h"

#include <orz/exception.h>

namespace Hw3D
{
	void OpenGlContext::Activate(wxWindow *target)
	{
		if(target == nullptr)
		{
			DO_THROW(Err::CriticalError, "OpenGlContext::Activate: target was null");
		}
		m_currentTarget = dynamic_cast<wxGLCanvas*>(target);
		if(m_currentTarget == nullptr)
		{
			DO_THROW(Err::CriticalError, "OpenGlContext::Activate: target was not a wxGLCanvas object");
		}

		m_context->SetCurrent(*m_currentTarget);

		InitializeContext();
	}

	void OpenGlContext::Clear(int a, int r, int g, int b)
	{
		glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGlContext::BeginDraw()
	{
		glUseProgram(m_program);
	}

	void OpenGlContext::RenderQuad(const Vertex2D &a, const Vertex2D &b, const Vertex2D &c, const Vertex2D &d)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		auto posAttrib = glGetAttribLocation(m_program, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
		glEnableVertexAttribArray(posAttrib);
		auto texAttrib = glGetAttribLocation(m_program, "tex");
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(texAttrib);


		/*float data[] = {
			a.Position.X, a.Position.Y, 0,
			b.Position.X, b.Position.Y, 0,
			c.Position.X, c.Position.Y, 0,
			d.Position.X, d.Position.Y, 0,
		};*/
		float data[] = {
			0.0f, 1.5f, 0, 0, 0,
			0.5f, -0.5f, 0, 1, 0,
			-0.5f, -0.5f, 0, 0, 1,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		GLenum err;
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "Failed rendering quad: " + GetGlErrorString(err));
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGlContext::EndDraw()
	{
		m_currentTarget->SwapBuffers();
	}

	void OpenGlContext::SetTexture(int stage, std::shared_ptr<Texture> texture)
	{
		auto glTexture = std::dynamic_pointer_cast<OpenGlTexture>(texture);
		if(glTexture == nullptr)
		{
			DO_THROW(Err::InvalidParam, "Texture object was not an OpenGlTexture");
		}
		auto u_tex = glGetUniformLocation(m_program, "semp");
		if(glGetError() != GL_NO_ERROR)
		{
			DO_THROW(Err::InvalidParam, "No semp");
		}
		if(u_tex == GL_INVALID_VALUE)
		{
			DO_THROW(Err::CriticalError, "Lying OpenGL driver, didn't get a valid uniform location");
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTexture->GetTextureName());
		glUniform1i(u_tex, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	void OpenGlContext::SetRenderTarget(std::shared_ptr<Texture> renderTarget)
	{

	}

	void OpenGlContext::SetMatrix(TransformState state, const Matrix &m)
	{

	}

	void OpenGlContext::SendTextureRect(std::shared_ptr<Texture> sourceTexture, const Geom::RectInt &sourceRect,
										std::shared_ptr<Texture> destinationTexture,
										const Geom::PointInt &destinationTopLeft)
	{

	}

	OpenGlContext::OpenGlContext(wxWindow *win):
		m_context{std::make_shared<wxGLContext>(dynamic_cast<wxGLCanvas*>(win))},
		m_vbo(0),
		m_isInitialized(0)
	{
		glEnable(GL_DEBUG_OUTPUT);
	}

	void OpenGlContext::InitializeContext()
	{
		if(m_isInitialized)
		{
			return;
		}

		GLenum err;

		if (glewInit() != GLEW_OK)
		{
			DO_THROW(Err::CriticalError, "Failed initializing GLEW");
		}
		glGenBuffers(1, &m_vbo);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glGenBuffers failed: " + GetGlErrorString(err));
		}
		const char* vertexShaderSource = "#version 330 core\n"
			"\n"
			"in vec3 position;\n"
			"in vec2 tex;\n"
			"out vec2 texCoord;\n"
			"\n"
			"void main()\n"
			"{\n"
			"	gl_Position = vec4(position, 1.0);\n"
			"	texCoord = tex;"
			"}";
		const char* fragmentShaderSource = "#version 330 core\n"
			"\n"
			"in vec2 texCoord;\n"
			"out vec4 col;\n"
			"uniform sampler2D semp;\n"
			"\n"
			"void main()\n"
			"{\n"
			"	col = texture2D(semp, texCoord) + 0.5 * vec4(texCoord, texCoord);\n"
			"}";
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glShaderSource (vertex) failed: " + GetGlErrorString(err));
		}

		glCompileShader(vertexShader);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glCompileShader (vertex) failed: " + GetGlErrorString(err));
		}

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glShaderSource (fragment) failed: " + GetGlErrorString(err));
		}
		glCompileShader(fragmentShader);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glCompileShader (fragment) failed: " + GetGlErrorString(err));
		}

		m_program = glCreateProgram();
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glCreateProgram failed: " + GetGlErrorString(err));
		}

		glAttachShader(m_program, vertexShader);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glAttachShader (vertex) failed: " + GetGlErrorString(err));
		}
		glAttachShader(m_program, fragmentShader);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glAttachShader (fragment) failed: " + GetGlErrorString(err));
		}

		glLinkProgram(m_program);
		if((err = glGetError()) != GL_NO_ERROR)
		{
			DO_THROW(Err::CriticalError, "glLinkProgram failed: " + GetGlErrorString(err));
		}
	}
}
