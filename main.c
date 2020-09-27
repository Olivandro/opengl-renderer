#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Defining compiler directives to identify with OS is Apple. This is because Apple has its own subset 
// functions for glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays.
#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif

// GL Error handing functions plus compiler directives
#define assert(x) if(!(x)) return -1;
#define GLCall(x) GLClearError();\
    x;\
    assert(GLLogCall(#x, __FILE__, __LINE__))\

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char *function, const char *file, const int line)
{
    // The breakdown of this compared to tutorial is 
    // quite different. Within the tutorial Cherno init and assigns GLenum error
    // within the while statement clause. This causes all sorts of problems
    // for the IDE, compiler etc. 
    int error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        printf("OpenGL Error code: %d\n", error);
        printf("Error occurred at function: %s\n", function);
        printf("File: %s\n", file);
        printf("Line: %d\n\n", line);
        return false;
    }
    return true;
}

// GLFW Error handing
void glfwErrorCallback(int error, const char description[]) {
    printf("\nGLFW Error: %s\n\n", description);
}


// To improve through use of malloc and free functions, to improve 
// memory allocation, and insure that struct ShaderSource, buffer and src 
// all have flexible but correct memory allocation. 

// Shader parsing and struct. parseShader would be considered a public function.
struct ShaderSource
{
    char VertexShader[512];
    char FragmentShader[512];
};

struct ShaderSource parseShader(const char filepath[])
{
    // instantiate a temp version of our struct
    struct ShaderSource tmp;

    // Open our target file for parsing. This is the shader source
    FILE *fp = fopen(filepath, "r");

    // Establish a buffer array for the file stream 
    char buffer[512];

    // unsigned char ptr array that has the same size array as the buffer and 
    // struct variables. This is probably not needed... Checking in on 
    // that.
    const char (*src)[512];

    // Determiners... These values determine when parsing if source code pertains to
    // a vertex or a fragment shader.
    char divider[] = "#shader";
    char version[] = "#version";
    char v[] = "vertex";
    char f[] = "fragment";

    while(fgets(buffer, 512, fp))
    {
        if (strstr(buffer, divider))
        {
            if (strstr(buffer, v))
            {
                src = &tmp.VertexShader;
            }
            else if (strstr(buffer, f))
            {
                src = &tmp.FragmentShader;
            }
        }
        else if (strstr(buffer, version))
        {
            strcpy(src, buffer);
        }
        else
        {
            strcat(src, buffer);
        }
    }
    // Lets close that file as we don't need it.
    fclose(fp);

    // Return our parsed shader sources.
    return tmp;
}

// Shader compiler. Embedded function, used within createShader
static unsigned int compileShader(unsigned int type, const char source[]) 
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    // Syntax error checking needing for shader files
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char message[len];
        glGetShaderInfoLog(id, len, &len, message);
        printf("%s\n", message);
        printf("FILE: %s\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
        glDeleteShader(id);
        return 0;
    }
    
    return id;
};

// Public function createShader. Call to create shaders and attach shaders to program
unsigned int createShader(const char vertexShader[], const char fragmentShader[]) 
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
};

// Rendering functions for abstractions and easier implimentation...

// 1. Generating vertex buffer and quick binding and unbinding functions
// vertexBuffer function generates a buffer, binds it and assigns it data
// On successful pass return 0, otherwise GLCall will return -1 with corresponding
// error code.
int vertexBuffer(unsigned int buffer, const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    return 0;
} 
int vertexBufferBind(unsigned int buffer)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    return 0;
}

// Unbind vertext buffer.
int vertexBufferUnbind(void)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    return 0;
}

// 2. Index buffer abstracted functions.... Here we only bind the index buffer
// and bind the corresponding data to the ib object. We do not gen a buffer
// for the ibo, because this causes a program crash with no error returned.
// This function must be called after glGenBuffers(GLsizei n, GLuint* buffers)
int indexBuffer(unsigned int ibo, const unsigned int* data, unsigned int count)
{
    assert(sizeof(unsigned int) == sizeof(GLuint));
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count, data, GL_STATIC_DRAW));
    return 0;
}

int indexBufferBind(unsigned int ibo)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    return 0;
}

int indexBufferUnbind(void)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    return 0;
}

// 3. Buffer Array signifies vertex buffer arrays or objects.
// TO DO:
// 2. attribute vertex buffers to init array, possily use for loop
// 3. Remane..
int bufferArray(unsigned int vao)
{
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));
    return vao;
}

int bindBufferArray(unsigned int vao)
{
    GLCall(glBindVertexArray(vao));
    return 0;
}

int unbindBufferArray(void)
{
    GLCall(glBindVertexArray(0));
    return 0;
}



// Vertex buffer and index structs and functions.
// 1. basic positions and position index struct
// 2. to contain index count
// 3. unsigned int ibo
// 4.
struct elementToDraw
{
    float positions[10];
    unsigned int index[8];
};



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Due to version limitation of Mac OSX openGL version 
    // GLFW_OPENGL_ANY_PROFILE is currently the only profile that is usable
    // Otehrwise you'll received... GLFW Error: Context profiles are only defined for OpenGL version 3.2 and above
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    glfwSetErrorCallback(glfwErrorCallback);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {   
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // GLEW implementation
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1; 
    }
    // Let's print the GLEW Version
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    // Let's print the target processor (your computer's integrated or external GPU) OpenGL Version
    fprintf(stdout, "Status: Using OpenGL %s\n", glGetString(GL_VERSION));

    fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


    /* 
    *   single element draw set. 
    *   1. establish data for drawing.
    *   2. establish a vao for housing single 
    *   3. bind vertex buffer
    *   4. bind index buffer
    *   5. bind everything to vao
    *   6. unbind.
    */
    // Define data Buffer for drawing to draw a square
    struct elementToDraw square = {
        // positions to draw
        {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f
        },
        // index draw sequence
        {
        0, 1, 2,
        2, 3, 0
        }
    };

    // Vertex array elements
    // We have to assign vao's value immediately, as glGenVertexArrays, which is nested in bufferArray, 
    // returns x number of vertex arrays. 
    unsigned int vao = bufferArray(vao);

    // Binding for vertex Buffer
    unsigned int buffer;
    vertexBuffer(buffer, square.positions, 4 * 2 * sizeof(float));
    
    // Binding for index buffer
    unsigned int ibo;
    indexBuffer(ibo, square.index, 6 * sizeof(unsigned int));

    // bind to vertex array object
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
    // Unbind current vertex components to draw... i.e. square
    vertexBufferUnbind();
    indexBufferUnbind();
    unbindBufferArray();
    /*
    * END OF SQUARE DRAW SET UP
    */



    /* 
    *   single element draw set. 
    *   1. establish data for drawing.
    *   2. establish a vao for housing single 
    *   3. bind vertex buffer
    *   4. bind index buffer
    *   5. bind everything to vao
    *   6. unbind.
    */
    // Draw a triangle!!
    struct elementToDraw triangle = {
        {
            1.0f, -1.0f
            -1.0f, -1.0f,
            -1.0f, 1.0f
        },
        {
            0, 1, 2
        }
    };

    unsigned int vao1 = bufferArray(vao1);
    vertexBuffer(buffer, triangle.positions, 3 * 2 * sizeof(float));
    unsigned int ibo1;
    indexBuffer(ibo1, triangle.index, 3 * sizeof(int));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    unbindBufferArray();
    vertexBufferUnbind();
    indexBufferUnbind();
    /*
    * END OF triangle DRAW SET UP
    */


    /*
    *   Set up for first shader program.
    */
    char filepath[] = "shaders/basic.shader";
    struct ShaderSource Source = parseShader(filepath);

    unsigned int shader = createShader(Source.VertexShader, Source.FragmentShader);
    GLCall(glUseProgram(shader));

    int location = glGetUniformLocation(shader, "u_Color");
    GLCall(glUniform4f(location, 1.0, 0.0, 0.0, 1.0));

    float r = 0.0f;
    float increment = 0.05f;

    // Unbind shader programs 
    GLCall(glUseProgram(0));
    /*
    * END OF Shader program setup
    */

        /*
    *   Set up for first shader program.
    */
    struct ShaderSource Source2 = parseShader(filepath);

    unsigned int shader2 = createShader(Source2.VertexShader, Source2.FragmentShader);
    GLCall(glUseProgram(shader2));

    int location2 = glGetUniformLocation(shader2, "u_Color");
    GLCall(glUniform4f(location2, 1.0, 0.5, 0.2, 1.0));

    // Unbind shader programs 
    GLCall(glUseProgram(0));
    /*
    * END OF Shader program setup
    */



    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* 
            Render here:
        */

        // Clear any colour data for all elements
        glClear(GL_COLOR_BUFFER_BIT);



        // Beginning of first draw call
        // Use first shader program
        GLCall(glUseProgram(shader));
        // Animation of colour
        GLCall(glUniform4f(location, r, 0.0, 0.0, 1.0));

        // Bind vertex array
        // Index buffer binding
        // Draw call or function..
        bindBufferArray(vao);
        indexBufferBind(ibo);
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, square.index));

        // Colour animation logic.
        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;
        //  End of first draw call

        // Beginning of second draw call
        // Use second shader program
        // Bind vertex array
        // Index buffer binding
        // Draw call or function..
        GLCall(glUseProgram(shader2));
        bindBufferArray(vao1);
        indexBufferBind(ibo1);
        GLCall(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, triangle.index));
        //  End of second draw call
        
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    
        /* Poll for and process events */
        glfwPollEvents();
        
    }

    GLCall(glDeleteProgram(shader));
    GLCall(glDeleteProgram(shader2));
    GLCall(glDeleteBuffers(1, &buffer));
    GLCall(glDeleteBuffers(1, &ibo));
    GLCall(glDeleteBuffers(1, &ibo1));
    GLCall(glDeleteVertexArrays(1, &vao));
    GLCall(glDeleteVertexArrays(1, &vao1));

    glfwTerminate();
    
    return 0;
}