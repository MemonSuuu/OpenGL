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

class VertexBuffer {
    private:
    //identify vertex buffer
        GLuint vertexbuffer;
    public:
        VertexBuffer(const std::vector<float> &vertices){
            // Generate 1 buffer, put the resulting identifier in vertexbuffer
            glGenBuffers(1, &vertexbuffer);
            // The following commands will talk about our 'vertexbuffer' buffer
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            // Give our vertices to OpenGL.
            glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
        }
        GLuint getBuffer(){
            return vertexbuffer;
        }
        ~VertexBuffer(){
            glDeleteBuffers(1, &vertexbuffer);
        }
        void setAttributes(){
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride (for other data)
                (void*)0            // array buffer offset (other data)
            );
        }
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

class Projection{
    private:
        glm::mat4 projection;
    public:
        Projection(int width, int height){
            // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
            auto ratio = static_cast<float>(width)/static_cast<float>(height);
            projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
        }
        glm::mat4 getProj(){
            return projection;
        }
};

class View{
    private:
        glm::mat4 view;
    public:
        View(){
            // Camera matrix
            view = glm::lookAt(
                glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );
        }
        glm::mat4 getView(){
            return view;
        }
};

class Model{
    private:
        glm::mat4 model;
    public: 
        Model(){
            // Model matrix : an identity matrix (model will be at the origin)
            glm::mat4 Model = glm::mat4(1.0f);
        }
        glm::mat4 getModel(){
            return model;
        }
};

class MVP{
    private:
        glm::mat4 mvp;
    public:
        MVP(int width, int height){
            Projection projection{width, height};
            
            View view{};
  
            Model model{};

            // Our ModelViewProjection : multiplication of our 3 matrices
            mvp = projection.getProj() * view.getView() * model.getModel(); // Remember, matrix multiplication is the other way around
        }
        glm::mat4 getMVP(){
            return mvp;
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
    //FORLATER: checker.glewCheck();

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // don't want the old OpenGL 

    //create a window
    GLFWwindow* window;
    window = glfwCreateWindow(1040, 640, "Triangle", NULL, NULL);
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

    VAO vao{};

    static const std::vector<float> vertices {
            -.50f, -1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.50f,  1.0f, 0.0f
    };
    
    VertexBuffer vB(vertices);

    MVP mvp{1040, 640};
    
    GLuint programID = LoadShaders("shaders/VertexShader.glsl", "shaders/FragmentShader.glsl");

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do{
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //use the shader
        glUseProgram(programID);

        // 1st attribute buffer : vertices
        vB.setAttributes();

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp.getMVP()[0][0]);

        // Draw the triangle !
        glBindVertexArray(vao.getID());
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

}