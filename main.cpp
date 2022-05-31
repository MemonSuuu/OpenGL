//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//shader headers
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

//glew and glfw libraries
#include <GL/glew.h>
#include <glfw/glfw3.h>

//matrices
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

class Buffer {
    protected:
        GLuint bufferID;
        const GLenum mode;
    public:
        Buffer(GLenum mode, std::size_t size,const void *data):mode(mode){
            glGenBuffers(1, &bufferID);
            bind();
            glBufferData(mode, size, data, GL_STATIC_DRAW);
        }
        ~Buffer(){
            glDeleteBuffers(1, &bufferID);
        }
        void bind(){
            glBindBuffer(mode, bufferID);
        }
        void unBind(){
            glBindBuffer(mode, 0);
        }
        GLuint getBuffer(){
            return bufferID;
        }
};

class PosBuffer : public Buffer{
    public:
        PosBuffer(const std::vector<float> &vertices):Buffer(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), static_cast<const void*>(vertices.data())){}
        void setAttributes(){
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                3*sizeof(float),
                static_cast<const void*>(0)
            );
        }
};

class ColorBuffer : public Buffer{
    public:
        ColorBuffer(const std::vector<float> &vertices):Buffer(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), static_cast<const void*>(vertices.data())){}
        void setAttributes(){
                glEnableVertexAttribArray(1);
            glVertexAttribPointer(
                1,
                3,
                GL_FLOAT,
                GL_FALSE,
                3*sizeof(float),
                static_cast<const void*>(0)
            );
        }
};

class IndexBuffer : public Buffer{
    public:
        IndexBuffer(const std::vector<GLuint> &indices):Buffer(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), static_cast<const void*>(indices.data())){}
};

class VAO{
    private:
        GLuint VertexArrayID;
    public:
        VAO(){
            //create a VAO
            glGenVertexArrays(1, &VertexArrayID);
            glBindVertexArray(VertexArrayID);
        }
        GLuint getID(){
            return VertexArrayID;
        }
};

class MVP{
    private:
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    public:
        MVP(int width, int height){
            // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
            auto ratio = static_cast<float>(width)/static_cast<float>(height);
            projection = glm::perspective(
                glm::radians(45.0f),    //vertical fieald of view in radians, from 90 (very wide) to 30 (very zoomed in) 
                ratio,                  //window width/height
                0.1f,                   //near clipping plane, keep big (0.1)
                100.0f                  //far clipping plane, keep little (100)
            );  

            // Camera matrix
            view = glm::lookAt(
                glm::vec3(3,3,3), // where Camera is at in World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );

            // view = glm::mat4(1.0f);
  
            // Model matrix : an identity matrix (model will be at the origin)
            model = glm::mat4(1.0f);

        }
        glm::mat4 getMVP(){
            return projection*view*model;
        }
};

int main(){
    //initialize glfw
    glewExperimental = true; //Needed for core profile
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // don't want the old OpenGL 

    //create a window
    GLFWwindow* window;
    window = glfwCreateWindow(1040, 780, "Triangle", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }

    //initialize glew
    glfwMakeContextCurrent(window); 
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    VAO vao{};

    static const std::vector<float> posVert{
        0.0f, 0.0f, 0.0f, 
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 
    };

    static const std::vector<float> colVert{
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        0.5f, 1.0f, 1.0f,
        1.0f, 0.5f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 1.0f
    };

    static const std::vector<GLuint> indices {
        0, 1, 2, 2, 1, 3, //1 face
        2, 3, 4, 4, 2, 5,
        3, 1, 4, 4, 1, 6,
        1, 6, 0, 0, 6, 7,
        0, 2, 5, 5, 0, 7, 
        4, 5, 6, 6, 5, 7
    };

    PosBuffer pB(posVert);
    pB.setAttributes();

    ColorBuffer cB(colVert);
    cB.setAttributes();

    IndexBuffer iB(indices);

    MVP mvp{1040, 780};
    
    GLuint programID = LoadShaders("shaders/VertexShader.glsl", "shaders/FragmentShader.glsl");

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do{
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mvp.getMVP()));

        //use the shader
        glUseProgram(programID);

        // 1st attribute buffer : vertices
        

        // Draw the triangle !
        // glBindVertexArray(vao.getID());
        // glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; number of vertices (polygons*triangles*vertices)
        glDrawElements(
            GL_TRIANGLES,
            indices.size(),
            GL_UNSIGNED_INT,
            nullptr
        );
        // glDisableVertexAttribArray(0);
        // glDisableVertexAttribArray(1);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

}