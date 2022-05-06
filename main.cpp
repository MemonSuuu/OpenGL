//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>
#include <glfw/glfw3.h>

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

    static const std::vector<GLfloat> vertices {
            -.50f, -1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.50f,  1.0f, 0.0f
    };
    
    VertexBuffer vB(vertices);
    
    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride (for other data)
        (void*)0            // array buffer offset (other data)
    );

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do{
        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );

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